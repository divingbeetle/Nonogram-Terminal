#ifndef UTILS_H
#define UTILS_H

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

void log_init(void);
void log_msg(enum log_level lv, const char *fmt, ...);

#endif // UTILS_H
