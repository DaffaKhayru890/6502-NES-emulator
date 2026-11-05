/*
 * sdl_video.h - Window, renderer, dan presentasi framebuffer NES.
 */
#ifndef TINYNES_FRONTEND_SDL_VIDEO_H
#define TINYNES_FRONTEND_SDL_VIDEO_H

#include "core/error.h"
#include "util/types.h"

typedef struct SDLVideo SDLVideo;

SDLVideo *sdl_video_create(const char *title, int scale, ErrorCode *error);
void sdl_video_destroy(SDLVideo *video);

/* framebuffer berisi indeks palette NES 256x240. */
void sdl_video_present(SDLVideo *video, const uint8_t *framebuffer);

#endif /* TINYNES_FRONTEND_SDL_VIDEO_H */
