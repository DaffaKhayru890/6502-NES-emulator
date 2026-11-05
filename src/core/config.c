/*
 * config.c - Parsing command line TinyNES.
 */
#include "core/config.h"

#include "util/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void config_init_defaults(Config *config)
{
    if (config == NULL) {
        return;
    }

    config->rom_path = NULL;
    config->scale = 3;
    config->mute = false;
    config->no_audio = false;
    config->debug = false;
    config->show_help = false;
    config->region = REGION_AUTO;
}

void config_print_usage(const char *program_name)
{
    const char *name = (program_name != NULL) ? program_name : "tinynes";

    printf("Penggunaan: %s [opsi] <rom.nes>\n\n", name);
    printf("Opsi:\n");
    printf("  --scale N        Faktor skala integer 1..10 (default: 3)\n");
    printf("  --mute           Matikan output suara (APU tetap berjalan)\n");
    printf("  --no-audio       Jangan buka audio device SDL2\n");
    printf("  --debug          Cetak statistik CPU/PPU ke stderr\n");
    printf("  --region auto    Pilih timing dari metadata ROM (default)\n");
    printf("  --region ntsc    Paksa timing NTSC (262 scanline, ~60 Hz)\n");
    printf("  --region pal     Paksa timing PAL (312 scanline, ~50 Hz)\n");
    printf("  --help           Tampilkan bantuan ini\n");
    printf("\nContoh:\n");
    printf("  %s --scale 4 games/example.nes\n", name);
    printf("  %s --mute --debug games/example.nes\n", name);
    printf("  %s --region pal games/example.nes\n", name);
}

static ErrorCode parse_scale(Config *config, const char *text)
{
    char *end = NULL;
    long value = 0;

    if (config == NULL || text == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || value < 1 || value > 10) {
        log_error("Nilai --scale harus bilangan bulat 1..10, bukan '%s'.", text);
        return ERR_CONFIG;
    }

    config->scale = (int)value;
    return ERR_OK;
}

ErrorCode config_parse(Config *config, int argc, char **argv)
{
    int i = 0;

    if (config == NULL || argc < 0 || argv == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    config_init_defaults(config);

    for (i = 1; i < argc; ++i) {
        const char *arg = argv[i];

        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            config->show_help = true;
            return ERR_OK;
        } else if (strcmp(arg, "--scale") == 0) {
            if (i + 1 >= argc) {
                log_error("Opsi --scale membutuhkan nilai.");
                return ERR_CONFIG;
            }
            ++i;
            if (parse_scale(config, argv[i]) != ERR_OK) {
                return ERR_CONFIG;
            }
        } else if (strcmp(arg, "--mute") == 0) {
            config->mute = true;
        } else if (strcmp(arg, "--no-audio") == 0) {
            config->no_audio = true;
        } else if (strcmp(arg, "--debug") == 0) {
            config->debug = true;
        } else if (strcmp(arg, "--region") == 0) {
            if (i + 1 >= argc) {
                log_error("Opsi --region membutuhkan nilai (auto/ntsc/pal).");
                return ERR_CONFIG;
            }
            ++i;
            if (strcmp(argv[i], "auto") == 0) {
                config->region = REGION_AUTO;
            } else if (strcmp(argv[i], "ntsc") == 0) {
                config->region = REGION_NTSC;
            } else if (strcmp(argv[i], "pal") == 0) {
                config->region = REGION_PAL;
            } else {
                log_error("Region '%s' tidak dikenal. Gunakan auto, ntsc, atau pal.", argv[i]);
                return ERR_CONFIG;
            }
        } else if (arg[0] == '-') {
            log_error("Opsi tidak dikenal: %s", arg);
            return ERR_CONFIG;
        } else {
            if (config->rom_path != NULL) {
                log_error("Hanya satu file ROM yang boleh diberikan.");
                return ERR_CONFIG;
            }
            config->rom_path = arg;
        }
    }

    return ERR_OK;
}
