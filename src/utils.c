#include "utils.h"
#include "config.h"
#include <stdarg.h>
#include <stdlib.h>

/* ---- MISC ---- */ 

void **alloc2d(size_t n_rows, size_t n_cols, size_t type_size)
{ 
    void **arr = malloc(n_rows * sizeof(void *));
    if (arr != NULL)
    {
        // Contiguous block
        arr[0] = malloc(n_rows * n_cols * type_size);
        if (arr[0] == NULL)
        {
            free(arr);
            return NULL;
        }

        // Link
        for (size_t i = 1; i < n_rows; i++)
        {
            arr[i] = (char *)arr[0] + i * n_cols * type_size;
        }
    }
    return arr;
}

void **calloc2d(size_t n_rows, size_t n_cols, size_t type_size)
{
    void **arr = malloc(n_rows * sizeof(void *));
    if (arr != NULL)
    {
        // Contiguous block
        arr[0] = calloc(n_rows * n_cols, type_size);
        if (arr[0] == NULL)
        {
            free(arr);
            return NULL;
        }

        // Link
        for (size_t i = 1; i < n_rows; i++)
        {
            arr[i] = (char *)arr[0] + i * n_cols * type_size;
        }
    }
    return arr;
}

void free2d(void **arr, size_t n_rows)
{
    if (arr != NULL)
    {
        free(arr[0]);
    }
    free(arr); arr = NULL;
}

void free_ptr_array(void **arr, size_t n)
{
    if (arr != NULL)
    {
        for (size_t i = 0; i < n; i++)
        {
            free(arr[i]);
        }
    }
    free(arr); arr = NULL;
}

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
