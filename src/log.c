#include <stdio.h>

#include "colors.h"
#include "log.h"

void log_printf(Logtype type, const char *func, const char *fmt, ...)
{
    va_list args;

    switch(type){
        case INFO:
            if(g_verbose) fprintf(stderr, COLOR_CYAN "[%-30s] INFO: " COLOR_RESET, func);
            else fprintf(stderr, COLOR_CYAN "INFO: " COLOR_RESET);
            break;
        case ERROR:
            if(g_verbose) fprintf(stderr, COLOR_RED "[%-30s] ERROR: " COLOR_RESET, func);
            else fprintf(stderr, COLOR_RED "ERROR: " COLOR_RESET);
            break;
        case OK:
            if(!g_verbose) return;
            fprintf(stderr, COLOR_GREEN "[%-30s] OK: " COLOR_RESET, func);
            break;
        case WARN:
            if(!g_verbose) return;
            fprintf(stderr, COLOR_YELLOW "[%-30s] WARN: " COLOR_RESET, func);
            break;
        default:
            return;
    }

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

