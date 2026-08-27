#ifndef LOG_H
#define LOG_H

#include <stdbool.h>
#include <stdarg.h>

extern bool g_verbose;

typedef enum{
    OK,
    INFO,
    WARN,
    ERROR,
} Logtype;

#if defined(__GNUC__) || defined(__clang__)
#define PRINTF_LIKE(format_index, argument_index) \
    __attribute__((format(printf, format_index, argument_index)))
#else
#define PRINTF_LIKE(format_index, argument_index)
#endif

void log_printf(Logtype type, const char *func, const char *fmt, ...)
    PRINTF_LIKE(3, 4);

#define log_info(...) log_printf(INFO, __func__, __VA_ARGS__)
#define log_error(...) log_printf(ERROR,__func__, __VA_ARGS__)
#define log_ok(...) log_printf(OK,__func__, __VA_ARGS__)
#define log_warn(...) log_printf(WARN,__func__, __VA_ARGS__)

#endif //LOG_H
