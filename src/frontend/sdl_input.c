/*
 * sdl_input.c - Pemetaan keyboard dan game controller ke tombol NES.
 *
 * Keyboard:
 *   Z / X           -> A / B
 *   Right Shift     -> Select
 *   Enter           -> Start
 *   Arrow keys      -> D-pad
 *   Escape          -> keluar
 *
 * Game controller pertama menempati port 1, controller kedua port 2.
 */
#include "frontend/sdl_input.h"

#include "input/controller.h"
#include "util/log.h"

#include <SDL2/SDL.h>
#include <stdlib.h>

#define MAX_CONTROLLERS 2

struct SDLInput {
    bool keys[SDL_NUM_SCANCODES];
    SDL_GameController *controllers[MAX_CONTROLLERS];
    SDL_JoystickID instance_ids[MAX_CONTROLLERS];
};

static void open_controller(SDLInput *input, int device_index)
{
    int slot = 0;

    for (slot = 0; slot < MAX_CONTROLLERS; ++slot) {
        if (input->controllers[slot] == NULL) {
            SDL_GameController *controller = SDL_GameControllerOpen(device_index);

            if (controller != NULL) {
                input->controllers[slot] = controller;
                input->instance_ids[slot] = SDL_JoystickInstanceID(
                    SDL_GameControllerGetJoystick(controller));
                log_info("Game controller terhubung ke port %d: %s", slot + 1,
                         SDL_GameControllerName(controller));
            }
            return;
        }
    }
}

static void close_controller(SDLInput *input, SDL_JoystickID instance_id)
{
    int slot = 0;

    for (slot = 0; slot < MAX_CONTROLLERS; ++slot) {
        if (input->controllers[slot] != NULL && input->instance_ids[slot] == instance_id) {
            SDL_GameControllerClose(input->controllers[slot]);
            input->controllers[slot] = NULL;
            log_info("Game controller port %d terputus.", slot + 1);
            return;
        }
    }
}

SDLInput *sdl_input_create(ErrorCode *error)
{
    SDLInput *input = (SDLInput *)calloc(1, sizeof(*input));

    if (error != NULL) {
        *error = ERR_OK;
    }

    if (input == NULL) {
        if (error != NULL) {
            *error = ERR_ALLOCATION;
        }
        return NULL;
    }

    /* Buka controller yang sudah terpasang saat aplikasi dimulai. */
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            open_controller(input, i);
        }
    }

    return input;
}

void sdl_input_destroy(SDLInput *input)
{
    if (input == NULL) {
        return;
    }

    for (int i = 0; i < MAX_CONTROLLERS; ++i) {
        if (input->controllers[i] != NULL) {
            SDL_GameControllerClose(input->controllers[i]);
        }
    }
    free(input);
}

bool sdl_input_process_event(SDLInput *input, const SDL_Event *event)
{
    if (input == NULL || event == NULL) {
        return false;
    }

    switch (event->type) {
    case SDL_QUIT:
        return true;

    case SDL_KEYDOWN:
        if ((int)event->key.keysym.scancode < SDL_NUM_SCANCODES) {
            input->keys[event->key.keysym.scancode] = true;
        }
        if (event->key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
            return true;
        }
        break;

    case SDL_KEYUP:
        if ((int)event->key.keysym.scancode < SDL_NUM_SCANCODES) {
            input->keys[event->key.keysym.scancode] = false;
        }
        break;

    case SDL_CONTROLLERDEVICEADDED:
        open_controller(input, event->cdevice.which);
        break;

    case SDL_CONTROLLERDEVICEREMOVED:
        close_controller(input, event->cdevice.which);
        break;

    default:
        break;
    }

    return false;
}

static uint8_t controller_buttons(SDL_GameController *controller)
{
    uint8_t buttons = 0;

    if (controller == NULL) {
        return 0;
    }

    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) != 0) {
        buttons |= CONTROLLER_BUTTON_A;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) != 0) {
        buttons |= CONTROLLER_BUTTON_B;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK) != 0) {
        buttons |= CONTROLLER_BUTTON_SELECT;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START) != 0) {
        buttons |= CONTROLLER_BUTTON_START;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) != 0) {
        buttons |= CONTROLLER_BUTTON_UP;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) != 0) {
        buttons |= CONTROLLER_BUTTON_DOWN;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) != 0) {
        buttons |= CONTROLLER_BUTTON_LEFT;
    }
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) != 0) {
        buttons |= CONTROLLER_BUTTON_RIGHT;
    }

    return buttons;
}

void sdl_input_buttons(const SDLInput *input, uint8_t *port1, uint8_t *port2)
{
    uint8_t keyboard = 0;

    if (port1 == NULL || port2 == NULL) {
        return;
    }

    *port1 = 0;
    *port2 = 0;

    if (input == NULL) {
        return;
    }

    if (input->keys[SDL_SCANCODE_Z]) {
        keyboard |= CONTROLLER_BUTTON_A;
    }
    if (input->keys[SDL_SCANCODE_X]) {
        keyboard |= CONTROLLER_BUTTON_B;
    }
    if (input->keys[SDL_SCANCODE_RSHIFT]) {
        keyboard |= CONTROLLER_BUTTON_SELECT;
    }
    if (input->keys[SDL_SCANCODE_RETURN]) {
        keyboard |= CONTROLLER_BUTTON_START;
    }
    if (input->keys[SDL_SCANCODE_UP]) {
        keyboard |= CONTROLLER_BUTTON_UP;
    }
    if (input->keys[SDL_SCANCODE_DOWN]) {
        keyboard |= CONTROLLER_BUTTON_DOWN;
    }
    if (input->keys[SDL_SCANCODE_LEFT]) {
        keyboard |= CONTROLLER_BUTTON_LEFT;
    }
    if (input->keys[SDL_SCANCODE_RIGHT]) {
        keyboard |= CONTROLLER_BUTTON_RIGHT;
    }

    *port1 = (uint8_t)(keyboard | controller_buttons(input->controllers[0]));
    *port2 = controller_buttons(input->controllers[1]);
}
