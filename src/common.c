#include "common.h"
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <json-c/json.h>


char *
itv_strdup(char const *old)
{
	size_t	len	= strlen(old);
	char *	new	= malloc(len + 1);
	if (new) memcpy(new, old, len + 1);
	return new;
}

static uint		log_callback_count = 0;
static itv_log_func	log_callbacks[2];
static void *		log_contexts[2];
static char		log_buffer[4096];

void
itv_log_print(char const *func, char const *file, uint line, uint level, char const *text)
{
	struct itv_log_param const param = {
		.text	= text,
		.func	= func,
		.file	= file,
		.line	= line,
		.level	= level,
	};

	for (uint i = 0; i < log_callback_count; ++i)
		log_callbacks[i](&param, log_contexts[i]);
}

void
itv_log_printf(char const *func, char const *file, uint line, uint level, char const *format, ...)
{
	va_list	args;

	va_start(args, format);
	int length = vsnprintf(log_buffer, sizeof log_buffer - 1, format, args);
	va_end(args);

	if (length >= 0)
		itv_log_print(func, file, line, level, log_buffer);
	else
		itv_log_print(func, file, line, level, "(vsnprintf error)");
}

bool
itv_log_register(itv_log_func func, void *context)
{
	if (log_callback_count < itv_countof(log_callbacks)) {
		uint i = log_callback_count++;
		log_callbacks[i] = func;
		log_contexts[i] = context;
		return true;
	} else
		return false;
}

void
itv_log_file(struct itv_log_param const *param, void *context)
{
	fprintf(context, "%-16s: %s\n", param->func, param->text);
}

//#define	itv_assert(x)	(void)((x) ? fprintf(stderr, "Assertion (%s) failed.", #x) : 0)
//#define itv_fail(...)	do { fprintf(stderr, __VA_ARGS__); fputc('\n', stdout); goto fail; } while (0)


char itv_time_unit_chars[] = {
	[itv_time_unit_year]	= 'Y',
	[itv_time_unit_month]	= 'M',
	//[itv_time_unit_week]	= 'V',
	[itv_time_unit_mday]	= 'D',
	[itv_time_unit_wday]	= 'P',
	[itv_time_unit_hour]	= 'h',
	[itv_time_unit_minute]	= 'm',
	[itv_time_unit_second]	= 's',
};

void
itv_time_range_init(struct itv_time_range *range)
{
	for (uint i = 0; i < itv_time_unit_count; ++i) {
		range->min[i] = INT_MIN;
		range->max[i] = INT_MAX;
	}
}

bool
itv_time_range_parse(struct itv_time_range *range, char const *start, char **end)
{
	char const *	c	= start;

	itv_time_range_init(range);

	while (*c && isspace(*c)) ++c;
	if (!isalpha(*c)) return false;

	while (isalpha(*c)) {
		unsigned long		min	= 0;
		unsigned long		max	= 0;
		enum itv_time_unit	unit	= 0;

		while (unit < itv_time_unit_count && itv_time_unit_chars[unit] != *c)
			++unit;
		if (unit >= itv_time_unit_count)
			break;
		++c;

		if (!isdigit(*c))
			break;
		min = max = strtoul(c, (char **)&c, 10);
		if (*c == '-')
			max = strtoul(c + 1, (char **)&c, 10);

		range->min[unit] = min;
		range->max[unit] = max;
	}

	if (end) *end = (char *)c;
	return true;
}

bool
itv_time_point_in_range(struct itv_time_range const *range, struct tm *point)
{
	int wday = (point->tm_wday + 6) % 7;

	return point->tm_mon + 1 >= range->min[itv_time_unit_month]
		&& point->tm_mon + 1 <= range->max[itv_time_unit_month]
		//&& point->tm_week >= range->min[itv_time_unit_week]
		//&& point->tm_week <= range->max[itv_time_unit_week]
		&& point->tm_mday >= range->min[itv_time_unit_mday]
		&& point->tm_mday <= range->max[itv_time_unit_mday]
		&& wday >= range->min[itv_time_unit_wday]
		&& wday <= range->max[itv_time_unit_wday]
		&& point->tm_hour >= range->min[itv_time_unit_hour]
		&& point->tm_hour <= range->max[itv_time_unit_hour]
		&& point->tm_min >= range->min[itv_time_unit_minute]
		&& point->tm_min <= range->max[itv_time_unit_minute]
		&& point->tm_sec >= range->min[itv_time_unit_second]
		&& point->tm_sec <= range->max[itv_time_unit_second];
}


void
itv_schedule_free(struct itv_schedule *schedule)
{
	for (uint i = 0; i < schedule->slots_len; ++i)
		free(schedule->slots[i].filename);
	free(schedule->slots);
	memset(schedule, 0, sizeof *schedule);
}

bool
itv_schedule_load(struct itv_schedule *schedule, char const *filename)
{
	bool			ok	= false;
	struct json_object *	obj	= 0;
	struct json_object *	ptr	= 0;

	if (!(obj = json_object_from_file(filename)))
		itv_fail("json_object_from_file(%s): %s", filename, json_util_get_last_err());

	if (!(ptr = json_object_object_get(obj, "slot_time")))
		itv_fail("\"%s\": missing key 'slot_time'.", filename);
	schedule->slot_seconds = json_object_get_int(ptr);

	if (!(ptr = json_object_object_get(obj, "slots")))
		itv_fail("\"%s\": missing key 'slots'.", filename);
	struct array_list *arr = json_object_get_array(ptr);
	size_t arr_len = array_list_length(arr);

	schedule->slots_len = arr_len;
	if (!(schedule->slots = malloc(sizeof(schedule->slots[0]) * arr_len)))
		itv_fail("malloc(): %s", strerror(errno));

	for (size_t i = 0; i < arr_len; ++i) {
		struct json_object *	it	= array_list_get_idx(arr, i);
		struct itv_time_range	time	= {0};
		char *			name	= NULL;

		itv_time_range_init(&time);
		if ((ptr = json_object_object_get(it, "time"))) {
			char const *t = json_object_get_string(ptr);
			if (!itv_time_range_parse(&time, t, NULL))
				itv_fail("\"%s\": malfromed time range 'slots[%zu].time'.", filename, i);
		}

		if (!(ptr = json_object_object_get(it, "name")))
			itv_fail("\"%s\": missing key 'slots[i].name'.", filename);
		if (!(name = itv_strdup(json_object_get_string(ptr))))
			itv_fail("malloc(): %s", strerror(errno));

		schedule->slots[i] = (struct itv_schedule_slot){
			.filename	= name,
			.time		= time,
		};
	}

fail:
	if (!obj) json_object_put(obj);
	return ok;
}

char const *
itv_schedule_next(struct itv_schedule *schedule, struct tm *time)
{
	struct itv_schedule_slot *	slot	= 0;

	do {
		slot = &schedule->slots[schedule->next_slot];
		schedule->next_slot = (schedule->next_slot + 1) % schedule->slots_len;
	} while (!itv_time_point_in_range(&slot->time, time));

	return slot->filename;
}
