/*
 * config.h - Konfigurasi aplikasi hasil parsing command line.
 *
 * Config hanyalah data konfigurasi milik frontend/aplikasi, bukan state
 * perangkat keras. Dengan demikian menyimpannya di satu objek yang diberikan
 * secara eksplisit tetap aman dan mudah diuji.
 */
#ifndef TINYNES_CORE_CONFIG_H
#define TINYNES_CORE_CONFIG_H

#include "core/error.h"
#include "util/types.h"

typedef struct Config {
    const char *rom_path;
    int scale;          /* Faktor pembesaran integer, 1..10. */
    bool mute;          /* Suara dimatikan, emulasi APU tetap berjalan. */
    bool no_audio;      /* Tidak membuka audio device sama sekali. */
    bool debug;         /* Statistik debug dicetak ke stderr. */
    bool show_help;     /* Pengguna meminta --help. */
    Region region;      /* REGION_AUTO, REGION_NTSC, atau REGION_PAL. */
} Config;

/* Nilai default: skala 3x, audio aktif, region otomatis. */
void config_init_defaults(Config *config);

/*
 * Parse argv. Argumen pertama yang bukan opsi dianggap path ROM.
 * Mengembalikan ERR_OK, ERR_CONFIG, atau ERR_INVALID_ARGUMENT.
 */
ErrorCode config_parse(Config *config, int argc, char **argv);

/* Cetak petunjuk pemakaian ke stdout. */
void config_print_usage(const char *program_name);

#endif /* TINYNES_CORE_CONFIG_H */
