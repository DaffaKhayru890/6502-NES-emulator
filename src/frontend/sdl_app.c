/*
 * sdl_app.c - Inisialisasi dan koordinasi subsistem SDL2.
 */
#include "frontend/sdl_app.h"

#include "frontend/sdl_audio.h"
#include "frontend/sdl_input.h"
#include "frontend/sdl_video.h"
#include "util/log.h"

#include <SDL2/SDL.h>
#include <stdlib.h>

struct SDLApp {
    SDLVideo *video;
    SDLAudio *audio;
    SDLInput *input;
    bool mute;
    bool audio_requested;
};

SDLApp *sdl_app_create(const Config *config, ErrorCode *error)
{
    SDLApp *app = NULL;

    if (error != NULL) {
        *error = ERR_OK;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) !=
        0) {
        log_error("SDL_Init gagal: %s", SDL_GetError());
        if (error != NULL) {
            *error = ERR_SDL_INIT;
        }
        return NULL;
    }

    app = (SDLApp *)calloc(1, sizeof(*app));
    if (app == NULL) {
        if (error != NULL) {
            *error = ERR_ALLOCATION;
        }
        SDL_Quit();
        return NULL;
    }

    app->mute = (config != NULL) ? config->mute : false;
    app->audio_requested = (config == NULL) || !config->no_audio;

    app->video = sdl_video_create(TINYNES_TITLE, (config != NULL) ? config->scale : 3, error);
    if (app->video == NULL) {
        sdl_app_destroy(app);
        SDL_Quit();
        return NULL;
    }

    app->input = sdl_input_create(error);
    if (app->input == NULL) {
        sdl_app_destroy(app);
        SDL_Quit();
        return NULL;
    }

    if (app->audio_requested) {
        ErrorCode audio_error = ERR_OK;

        app->audio = sdl_audio_create(44100U, &audio_error);
        if (app->audio == NULL) {
            /* Emulator tetap dapat berjalan tanpa audio. */
            log_warn("Audio tidak tersedia (%s); melanjutkan tanpa suara.",
                     error_string(audio_error));
        }
    }

    return app;
}

void sdl_app_destroy(SDLApp *app)
{
    if (app == NULL) {
        return;
    }

    sdl_audio_destroy(app->audio);
    sdl_input_destroy(app->input);
    sdl_video_destroy(app->video);
    free(app);
    SDL_Quit();
}

bool sdl_app_process_events(SDLApp *app)
{
    SDL_Event event;

    if (app == NULL) {
        return false;
    }

    while (SDL_PollEvent(&event)) {
        if (sdl_input_process_event(app->input, &event)) {
            return false;
        }
    }

    return true;
}

void sdl_app_buttons(const SDLApp *app, uint8_t *port1, uint8_t *port2)
{
    if (app == NULL) {
        if (port1 != NULL) {
            *port1 = 0;
        }
        if (port2 != NULL) {
            *port2 = 0;
        }
        return;
    }

    sdl_input_buttons(app->input, port1, port2);
}

void sdl_app_present(SDLApp *app, const uint8_t *framebuffer)
{
    if (app != NULL) {
        sdl_video_present(app->video, framebuffer);
    }
}

void sdl_app_queue_audio(SDLApp *app, const int16_t *samples, size_t count)
{
    if (app == NULL || app->mute || app->audio == NULL) {
        return;
    }

    sdl_audio_queue(app->audio, samples, count);
}

uint32_t sdl_app_sample_rate(const SDLApp *app)
{
    uint32_t rate = (app != NULL && app->audio != NULL) ? sdl_audio_sample_rate(app->audio) : 0U;

    return (rate > 0U) ? rate : 44100U;
}

bool sdl_app_audio_enabled(const SDLApp *app)
{
    return app != NULL && app->audio != NULL && !app->mute;
}
