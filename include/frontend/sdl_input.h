/*
 * sdl_input.h - Keyboard dan game controller SDL2.
 */
#ifndef TINYNES_FRONTEND_SDL_INPUT_H
#define TINYNES_FRONTEND_SDL_INPUT_H

#include "core/error.h"
#include "util/types.h"

union SDL_Event;

typedef struct SDLInput SDLInput;

SDLInput *sdl_input_create(ErrorCode *error);
void sdl_input_destroy(SDLInput *input);

/* Mengembalikan true bila event meminta aplikasi berhenti. */
bool sdl_input_process_event(SDLInput *input, const union SDL_Event *event);

/* Status tombol port 1 dan port 2 dalam format ControllerButton. */
void sdl_input_buttons(const SDLInput *input, uint8_t *port1, uint8_t *port2);

#endif /* TINYNES_FRONTEND_SDL_INPUT_H */
