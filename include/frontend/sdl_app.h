/*
 * sdl_app.h - Abstraksi aplikasi SDL2.
 *
 * SDLApp menyatukan video, audio, dan input. Ia tidak mengetahui apa pun
 * tentang CPU, PPU, atau cartridge; semua data grafis/audio diberikan oleh
 * pemanggil (main.c) dari inti emulator.
 */
#ifndef TINYNES_FRONTEND_SDL_APP_H
#define TINYNES_FRONTEND_SDL_APP_H

#include "core/config.h"
#include "core/error.h"
#include "util/types.h"

typedef struct SDLApp SDLApp;

SDLApp *sdl_app_create(const Config *config, ErrorCode *error);
void sdl_app_destroy(SDLApp *app);

/* Proses seluruh event SDL. False berarti pengguna meminta keluar. */
bool sdl_app_process_events(SDLApp *app);

void sdl_app_buttons(const SDLApp *app, uint8_t *port1, uint8_t *port2);
void sdl_app_present(SDLApp *app, const uint8_t *framebuffer);
void sdl_app_queue_audio(SDLApp *app, const int16_t *samples, size_t count);

uint32_t sdl_app_sample_rate(const SDLApp *app);
bool sdl_app_audio_enabled(const SDLApp *app);

#endif /* TINYNES_FRONTEND_SDL_APP_H */
