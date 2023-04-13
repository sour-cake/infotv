#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef unsigned int uint;

#define	itv_countof(x)	(sizeof(x)/sizeof((x)[0]))

char *	itv_strdup(char const *old);

enum itv_log_level {
	itv_log_level_debug,
	itv_log_level_info,
	itv_log_level_warning,
	itv_log_level_error,
	itv_log_level_count,
};

struct itv_log_param {
	char const *	text;
	char const *	func;
	char const *	file;
	uint		line;
	uint		level;
};

typedef void (*itv_log_func)(struct itv_log_param const *param, void *context);

void	itv_log_print(char const *func, char const *file, uint line, uint level, char const *text);
void	itv_log_printf(char const *func, char const *file, uint line, uint level, char const *format, ...);
bool	itv_log_register(itv_log_func func, void *context);
void	itv_log_file(struct itv_log_param const *param, void *context);

#define	itv_debug(...)	itv_log_printf(__func__, __FILE__, __LINE__, itv_log_level_debug, __VA_ARGS__)
#define	itv_info(...)	itv_log_printf(__func__, __FILE__, __LINE__, itv_log_level_info, __VA_ARGS__)
#define	itv_warn(...)	itv_log_printf(__func__, __FILE__, __LINE__, itv_log_level_warning, __VA_ARGS__)
#define	itv_error(...)	itv_log_printf(__func__, __FILE__, __LINE__, itv_log_level_error, __VA_ARGS__)

#define	itv_assert(x)	(void)(!(x) ? itv_error("Assertion (%s) failed.", #x) : 0)
#define itv_fail(...)	do { itv_error(__VA_ARGS__); goto fail; } while (0)


enum itv_time_unit {
	itv_time_unit_year,
	itv_time_unit_month,
	//itv_time_unit_week,
	itv_time_unit_mday,
	itv_time_unit_wday,
	itv_time_unit_hour,
	itv_time_unit_minute,
	itv_time_unit_second,
	itv_time_unit_count,
};

extern char itv_time_unit_chars[];

struct itv_time_range {
	int		min[itv_time_unit_count];
	int		max[itv_time_unit_count];
};

void	itv_time_range_init(struct itv_time_range *range);
bool	itv_time_range_parse(struct itv_time_range *range, char const *start, char **end);
bool	itv_time_point_in_range(struct itv_time_range const *range, struct tm *point);


struct itv_schedule_slot {
	char *				filename;
	struct itv_time_range		time;
};

struct itv_schedule {
	struct itv_schedule_slot *	slots;
	uint				slots_len;
	uint				next_slot;
	uint				slot_seconds;
};

void		itv_schedule_free(struct itv_schedule *schedule);
bool		itv_schedule_parse(struct itv_schedule *schedule, char const *start, char **end);
bool		itv_schedule_load(struct itv_schedule *schedule, char const *filename);
char const *	itv_schedule_next(struct itv_schedule *schedule, struct tm *time);
