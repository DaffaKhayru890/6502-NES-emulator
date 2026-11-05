/*
 * sdl_audio.h - Output audio SDL2 berbasis queue.
 */
#ifndef TINYNES_FRONTEND_SDL_AUDIO_H
#define TINYNES_FRONTEND_SDL_AUDIO_H

#include "core/error.h"
#include "util/types.h"

typedef struct SDLAudio SDLAudio;

/* Membuka device mono S16 pada sample rate yang diminta. */
SDLAudio *sdl_audio_create(uint32_t sample_rate, ErrorCode *error);
void sdl_audio_destroy(SDLAudio *audio);

void sdl_audio_queue(SDLAudio *audio, const int16_t *samples, size_t count);
uint32_t sdl_audio_queued_bytes(const SDLAudio *audio);
uint32_t sdl_audio_sample_rate(const SDLAudio *audio);
bool sdl_audio_ok(const SDLAudio *audio);

#endif /* TINYNES_FRONTEND_SDL_AUDIO_H */
