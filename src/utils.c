#include "utils.h"
#include "config.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* ---- LOGGING ---- */

static FILE *log_file = NULL;

static const char *log_level_str[] = 
{
    [LOG_DEBUG]   = "DEBUG",
    [LOG_INFO]    = "INFO",
    [LOG_WARNING] = "WARNING",
    [LOG_ERROR]   = "ERROR",
    [LOG_FATAL]   = "FATAL"
};

void log_close(void)
{
    if (log_file)
    {
        fclose(log_file);
    }
}

void log_init(void)
{
    const char *mode = CLEAR_LOG_AT_STARTUP ? "w" : "a";
    log_file = fopen(LOG_FILE_NAME, mode);
    if (log_file == NULL)
    {
        fprintf(stderr, "Error opening log file\n");
        return;
    }

    atexit(log_close);
    LOG(LOG_INFO, "Log file opened");
}

void log_msg(enum log_level lv, const char *fmt, ...)
{
    if (log_file == NULL || lv < LOG_LEVEL)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);
    fprintf(log_file, "[%s] ", log_level_str[lv]);
    vfprintf(log_file, fmt, args);
    fprintf(log_file, "\n");
    fflush(log_file);
    va_end(args);
}
