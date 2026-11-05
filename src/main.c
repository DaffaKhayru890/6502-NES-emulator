/*
 * main.c - Titik masuk aplikasi TinyNES.
 *
 * Urutan:
 * 1. Parse command line.
 * 2. Inisialisasi SDL2 (menentukan sample rate audio).
 * 3. Buat NES dengan sample rate tersebut.
 * 4. Muat ROM.
 * 5. Loop: event -> tombol -> satu frame emulasi -> presentasi -> audio -> pacing.
 */
#include "core/config.h"
#include "core/error.h"
#include "core/nes.h"
#include "frontend/sdl_app.h"
#include "util/log.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define AUDIO_CHUNK 4096

int main(int argc, char **argv)
{
    Config config;
    ErrorCode error = ERR_OK;
    SDLApp *app = NULL;
    Nes *nes = NULL;
    int16_t audio_buffer[AUDIO_CHUNK];
    uint64_t perf_frequency = 0;
    double frame_ticks = 0.0;
    uint64_t next_frame = 0;
    int exit_code = EXIT_FAILURE;

    error = config_parse(&config, argc, argv);
    if (error != ERR_OK) {
        config_print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (config.show_help) {
        config_print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (config.rom_path == NULL) {
        fprintf(stderr, "Penggunaan: %s [options] <rom.nes>\n", argv[0]);
        config_print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    log_set_level(config.debug ? LOG_LEVEL_DEBUG : LOG_LEVEL_INFO);

    app = sdl_app_create(&config, &error);
    if (app == NULL) {
        error_report(error, "Gagal menginisialisasi SDL2");
        return EXIT_FAILURE;
    }

    nes = nes_create(sdl_app_sample_rate(app), config.region);
    if (nes == NULL) {
        log_error("Gagal membuat emulator NES.");
        sdl_app_destroy(app);
        return EXIT_FAILURE;
    }

    nes_set_debug(nes, config.debug);

    error = nes_load_rom(nes, config.rom_path);
    if (error != ERR_OK) {
        error_report(error, "Gagal memuat ROM");
        goto cleanup;
    }

    perf_frequency = SDL_GetPerformanceFrequency();
    frame_ticks = (double)perf_frequency / nes_frame_rate(nes);
    next_frame = SDL_GetPerformanceCounter();

    while (sdl_app_process_events(app)) {
        uint8_t port1 = 0;
        uint8_t port2 = 0;
        size_t sample_count = 0;

        sdl_app_buttons(app, &port1, &port2);
        nes_set_buttons(nes, 0, port1);
        nes_set_buttons(nes, 1, port2);

        nes_run_frame(nes);
        sdl_app_present(app, nes_framebuffer(nes));

        if (sdl_app_audio_enabled(app)) {
            sample_count = nes_read_audio(nes, audio_buffer, AUDIO_CHUNK);
            sdl_app_queue_audio(app, audio_buffer, sample_count);
        } else {
            /* Tetap kosongkan ring buffer agar tidak menumpuk. */
            while (nes_read_audio(nes, audio_buffer, AUDIO_CHUNK) > 0U) {
            }
        }

        /* Pacing host terpisah dari waktu emulasi (lihat docs/timing.md). */
        {
            uint64_t now = SDL_GetPerformanceCounter();

            next_frame += (uint64_t)frame_ticks;
            if (now < next_frame) {
                double remaining_ms =
                    (double)(next_frame - now) * 1000.0 / (double)perf_frequency;
                if (remaining_ms > 1.0) {
                    SDL_Delay((Uint32)(remaining_ms - 1.0));
                }
                while (SDL_GetPerformanceCounter() < next_frame) {
                    /* Tunggu sisa <1 ms tanpa tidur agar presisi. */
                }
            } else if (now - next_frame > (uint64_t)(frame_ticks * 4.0)) {
                /* Terlalu jauh tertinggal (mis. window dipindahkan); reset. */
                next_frame = now;
            }
        }
    }

    exit_code = EXIT_SUCCESS;

cleanup:
    nes_destroy(nes);
    sdl_app_destroy(app);
    return exit_code;
}
