#include <stdarg.h>
#include "common.h"

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
