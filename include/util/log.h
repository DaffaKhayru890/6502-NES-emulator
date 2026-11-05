/*
 * log.h - Logging sederhana untuk TinyNES.
 *
 * Log tidak boleh menjadi jalur panas (hot path) emulator. Pemanggil harus
 * memakai log_debug() hanya pada peristiwa penting, bukan setiap instruksi.
 * Level log disimpan sebagai satu variabel global kecil; ini bukan state
 * perangkat keras sehingga tidak melanggar aturan "no global hardware state".
 */
#ifndef TINYNES_UTIL_LOG_H
#define TINYNES_UTIL_LOG_H

#include "util/types.h"

typedef enum LogLevel {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3
} LogLevel;

void log_set_level(LogLevel level);
LogLevel log_get_level(void);

/* fmt memakai format printf. Tambahan atribut membantu compiler memeriksa. */
void log_message(LogLevel level, const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;

#define log_error(...) log_message(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_warn(...) log_message(LOG_LEVEL_WARN, __VA_ARGS__)
#define log_info(...) log_message(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_debug(...) log_message(LOG_LEVEL_DEBUG, __VA_ARGS__)

#endif /* TINYNES_UTIL_LOG_H */
