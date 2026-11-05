/*
 * log.c - Implementasi logging TinyNES.
 */
#include "util/log.h"

#include <stdarg.h>
#include <stdio.h>

static LogLevel g_log_level = LOG_LEVEL_INFO;

static const char *level_prefix(LogLevel level)
{
    switch (level) {
    case LOG_LEVEL_ERROR:
        return "[ERROR]";
    case LOG_LEVEL_WARN:
        return "[WARN ]";
    case LOG_LEVEL_INFO:
        return "[INFO ]";
    case LOG_LEVEL_DEBUG:
        return "[DEBUG]";
    default:
        return "[?????]";
    }
}

void log_set_level(LogLevel level)
{
    g_log_level = level;
}

LogLevel log_get_level(void)
{
    return g_log_level;
}

void log_message(LogLevel level, const char *fmt, ...)
{
    va_list args;

    if (level > g_log_level || fmt == NULL) {
        return;
    }

    /* Log selalu ke stderr agar stdout tetap bersih untuk keperluan lain. */
    fprintf(stderr, "%s ", level_prefix(level));

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fputc('\n', stderr);
}
