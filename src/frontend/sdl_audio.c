/*
 * sdl_audio.c - SDL2 audio queue (SDL_QueueAudio).
 *
 * SDL_QueueAudio bersifat thread-safe terhadap thread audio internal SDL.
 * Main thread hanya menambahkan sampel; callback internal SDL yang
 * mengosongkan queue. Dengan demikian APU tidak dibaca dari dua thread.
 */
#include "frontend/sdl_audio.h"

#include "util/log.h"

#include <SDL2/SDL.h>
#include <stdlib.h>

struct SDLAudio {
    SDL_AudioDeviceID device;
    uint32_t sample_rate;
    bool ok;
};

SDLAudio *sdl_audio_create(uint32_t sample_rate, ErrorCode *error)
{
    SDLAudio *audio = NULL;
    SDL_AudioSpec want;
    SDL_AudioSpec have;

    if (error != NULL) {
        *error = ERR_OK;
    }

    audio = (SDLAudio *)calloc(1, sizeof(*audio));
    if (audio == NULL) {
        if (error != NULL) {
            *error = ERR_ALLOCATION;
        }
        return NULL;
    }

    SDL_zero(want);
    want.freq = (int)((sample_rate > 0U) ? sample_rate : 44100U);
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = NULL; /* Mode queue. */

    audio->device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio->device == 0) {
        log_error("SDL_OpenAudioDevice gagal: %s", SDL_GetError());
        if (error != NULL) {
            *error = ERR_SDL_AUDIO;
        }
        sdl_audio_destroy(audio);
        return NULL;
    }

    audio->sample_rate = (uint32_t)have.freq;
    audio->ok = true;
    SDL_PauseAudioDevice(audio->device, 0);
    log_info("Audio device: %u Hz, mono S16", audio->sample_rate);
    return audio;
}

void sdl_audio_destroy(SDLAudio *audio)
{
    if (audio == NULL) {
        return;
    }

    if (audio->device != 0) {
        SDL_CloseAudioDevice(audio->device);
    }
    free(audio);
}

void sdl_audio_queue(SDLAudio *audio, const int16_t *samples, size_t count)
{
    if (audio == NULL || !audio->ok || samples == NULL || count == 0U) {
        return;
    }

    /* Jangan biarkan latency menumpuk tanpa batas. */
    if (SDL_GetQueuedAudioSize(audio->device) > audio->sample_rate / 2U * 2U) {
        return;
    }

    if (SDL_QueueAudio(audio->device, samples, (Uint32)(count * sizeof(int16_t))) != 0) {
        log_warn("SDL_QueueAudio gagal: %s", SDL_GetError());
    }
}

uint32_t sdl_audio_queued_bytes(const SDLAudio *audio)
{
    if (audio == NULL || audio->device == 0) {
        return 0;
    }
    return SDL_GetQueuedAudioSize(audio->device);
}

uint32_t sdl_audio_sample_rate(const SDLAudio *audio)
{
    return (audio != NULL) ? audio->sample_rate : 0U;
}

bool sdl_audio_ok(const SDLAudio *audio)
{
    return audio != NULL && audio->ok;
}
