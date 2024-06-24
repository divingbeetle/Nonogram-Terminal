#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* ---- MISC ---- */ 

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * Malloc for 2d arrays with contigous memory.
 *  - Allows array notation for access
 * @param type_size Your `sizeof(type)`
 * @retval NULL if allocation failed
 */
void **alloc2d(size_t n_rows, size_t n_cols, size_t type_size);
void free2d(void **arr, size_t n_rows);

void free_ptr_array(void **arr, size_t n);

/* ---- LOGGING ---- */

enum log_level
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
};

#define LOGF(level, fmt, ...)                                                  \
    log_msg(level, "%s:%d in %s " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define LOG(level, msg) LOGF(level, "%s", msg)

#define LOG_AND_EXIT(fmt, ...)                                                 \
    do                                                                         \
    {                                                                          \
        LOGF(LOG_FATAL, fmt, __VA_ARGS__);                                     \
        fprintf(stderr, fmt, __VA_ARGS__);                                     \
        exit(1);                                                               \
    } while (0)

#define ALLOC_CHECK_RETURN(ptr, ret)                                           \
    do                                                                         \
    {                                                                          \
        if ((ptr) == NULL)                                                     \
        {                                                                      \
            LOG(LOG_ERROR, "Memory allocation failed");                        \
            return (ret);                                                      \
        }                                                                      \
    } while (0)

#define ALLOC_CHECK_EXIT(ptr)                                                  \
    do                                                                         \
    {                                                                          \
        if ((ptr) == NULL)                                                     \
        {                                                                      \
            LOG_AND_EXIT("%s", "Memory allocation failed");                    \
        }                                                                      \
    } while (0)

void log_init(void);
void log_msg(enum log_level lv, const char *fmt, ...);

#endif // UTILS_H
