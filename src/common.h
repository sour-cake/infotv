#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned int uint;

#define	itv_countof(x)	(sizeof(x)/sizeof((x)[0]))

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
