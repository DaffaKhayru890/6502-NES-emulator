/*
 * controller.c - Implementasi shift register controller NES.
 */
#include "input/controller.h"

void controller_init(Controller *controller)
{
    if (controller == NULL) {
        return;
    }

    controller->buttons = 0;
    controller->shift = 0;
    controller->strobe = false;
}

void controller_set_buttons(Controller *controller, uint8_t buttons)
{
    if (controller == NULL) {
        return;
    }

    controller->buttons = buttons;
    /* Saat strobe tinggi, register harus selalu mengikuti tombol terkini. */
    if (controller->strobe) {
        controller->shift = buttons;
    }
}

uint8_t controller_get_buttons(const Controller *controller)
{
    return (controller != NULL) ? controller->buttons : 0;
}

void controller_write_strobe(Controller *controller, uint8_t value)
{
    bool new_strobe = false;

    if (controller == NULL) {
        return;
    }

    new_strobe = (value & 0x01U) != 0U;

    /*
     * Transisi 0 -> 1 memuat ulang register geser. Menulis 1 berkali-kali
     * juga tetap memuat ulang; perilaku ini aman untuk game yang memakai
     * strobe panjang.
     */
    if (new_strobe) {
        controller->shift = controller->buttons;
    }

    controller->strobe = new_strobe;
}

uint8_t controller_read(Controller *controller)
{
    uint8_t value = 0;

    if (controller == NULL) {
        return 0x40;
    }

    if (controller->strobe) {
        value = (uint8_t)(controller->buttons & 0x01U);
    } else {
        value = (uint8_t)(controller->shift & 0x01U);
        /* Setelah bit terakhir, isi dengan 1 (perilaku open bus NES). */
        controller->shift = (uint8_t)((controller->shift >> 1) | 0x80U);
    }

    return (uint8_t)(value | 0x40U);
}
