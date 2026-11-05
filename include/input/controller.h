/*
 * controller.h - Emulasi port controller NES.
 *
 * Controller NES adalah shift register 8 bit yang dibaca secara serial.
 * Menulis 1 ke $4016 "menahan" (strobe) register pada tombol saat ini;
 * menulis 0 memulai penggeseran. Setiap pembacaan $4016/$4017 mengembalikan
 * satu bit (A lebih dulu) lalu menggeser register.
 */
#ifndef TINYNES_INPUT_CONTROLLER_H
#define TINYNES_INPUT_CONTROLLER_H

#include "util/types.h"

/* Urutan bit internal. Bit 0 = A, sesuai urutan serial NES. */
typedef enum ControllerButton {
    CONTROLLER_BUTTON_A = 1U << 0,
    CONTROLLER_BUTTON_B = 1U << 1,
    CONTROLLER_BUTTON_SELECT = 1U << 2,
    CONTROLLER_BUTTON_START = 1U << 3,
    CONTROLLER_BUTTON_UP = 1U << 4,
    CONTROLLER_BUTTON_DOWN = 1U << 5,
    CONTROLLER_BUTTON_LEFT = 1U << 6,
    CONTROLLER_BUTTON_RIGHT = 1U << 7
} ControllerButton;

typedef struct Controller {
    uint8_t buttons; /* Status tombol terkini, bit sesuai ControllerButton. */
    uint8_t shift;   /* Register geser yang dibaca CPU. */
    bool strobe;     /* Status strobe dari $4016. */
} Controller;

void controller_init(Controller *controller);
void controller_set_buttons(Controller *controller, uint8_t buttons);
uint8_t controller_get_buttons(const Controller *controller);

/* Menulis bit 0 dari nilai ke port kontrol ($4016). */
void controller_write_strobe(Controller *controller, uint8_t value);

/* Membaca satu bit serial. Bit tinggi diisi 1 seperti open bus NES. */
uint8_t controller_read(Controller *controller);

#endif /* TINYNES_INPUT_CONTROLLER_H */
