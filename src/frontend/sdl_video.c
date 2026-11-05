/*
 * sdl_video.c - Implementasi jendela dan texture streaming SDL2.
 *
 * Texture internal tetap 256x240. SDL_RenderSetLogicalSize dan
 * SDL_RenderSetIntegerScale membuat penskalaan integer dengan letterbox,
 * sehingga rasio 4:3 NES terjaga tanpa stretching.
 */
#include "frontend/sdl_video.h"

#include "ppu/ppu.h"
#include "util/log.h"

#include <SDL2/SDL.h>
#include <stdlib.h>

struct SDLVideo {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t pixels[NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT];
};

SDLVideo *sdl_video_create(const char *title, int scale, ErrorCode *error)
{
    SDLVideo *video = NULL;
    int width = 0;
    int height = 0;

    if (error != NULL) {
        *error = ERR_OK;
    }

    if (scale < 1) {
        scale = 3;
    }

    width = NES_SCREEN_WIDTH * scale;
    height = NES_SCREEN_HEIGHT * scale;

    video = (SDLVideo *)calloc(1, sizeof(*video));
    if (video == NULL) {
        if (error != NULL) {
            *error = ERR_ALLOCATION;
        }
        return NULL;
    }

    video->window = SDL_CreateWindow((title != NULL) ? title : TINYNES_TITLE,
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
                                     height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (video->window == NULL) {
        log_error("SDL_CreateWindow gagal: %s", SDL_GetError());
        if (error != NULL) {
            *error = ERR_SDL_WINDOW;
        }
        sdl_video_destroy(video);
        return NULL;
    }

    /*
     * Tidak memakai PRESENTVSYNC: pacing dilakukan berdasarkan clock emulasi
     * region (sekitar 60 Hz NTSC atau 50 Hz PAL), bukan refresh rate monitor.
     */
    video->renderer = SDL_CreateRenderer(video->window, -1, SDL_RENDERER_ACCELERATED);
    if (video->renderer == NULL) {
        log_error("SDL_CreateRenderer gagal: %s", SDL_GetError());
        if (error != NULL) {
            *error = ERR_SDL_RENDERER;
        }
        sdl_video_destroy(video);
        return NULL;
    }

    /* Penskalaan terdekat (nearest neighbor) agar piksel tetap tajam. */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_RenderSetLogicalSize(video->renderer, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);
    SDL_RenderSetIntegerScale(video->renderer, SDL_TRUE);

    video->texture = SDL_CreateTexture(video->renderer, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_STREAMING, NES_SCREEN_WIDTH,
                                       NES_SCREEN_HEIGHT);
    if (video->texture == NULL) {
        log_error("SDL_CreateTexture gagal: %s", SDL_GetError());
        if (error != NULL) {
            *error = ERR_SDL_TEXTURE;
        }
        sdl_video_destroy(video);
        return NULL;
    }

    return video;
}

void sdl_video_destroy(SDLVideo *video)
{
    if (video == NULL) {
        return;
    }

    if (video->texture != NULL) {
        SDL_DestroyTexture(video->texture);
    }
    if (video->renderer != NULL) {
        SDL_DestroyRenderer(video->renderer);
    }
    if (video->window != NULL) {
        SDL_DestroyWindow(video->window);
    }
    free(video);
}

void sdl_video_present(SDLVideo *video, const uint8_t *framebuffer)
{
    size_t i = 0;

    if (video == NULL || framebuffer == NULL) {
        return;
    }

    for (i = 0; i < (size_t)NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT; ++i) {
        uint32_t rgb = ppu_palette_rgb(framebuffer[i]);
        video->pixels[i] = 0xFF000000U | rgb;
    }

    SDL_UpdateTexture(video->texture, NULL, video->pixels, NES_SCREEN_WIDTH * (int)sizeof(uint32_t));
    SDL_RenderClear(video->renderer);
    SDL_RenderCopy(video->renderer, video->texture, NULL, NULL);
    SDL_RenderPresent(video->renderer);
}
