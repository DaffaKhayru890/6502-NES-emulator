/*
 * apu_triangle.c - Kanal triangle.
 *
 * Triangle tidak memakai envelope. Amplitudonya selalu 15 langkah dan
 * diredam oleh length counter serta linear counter. Sequencer 32 langkah
 * menghasilkan gelombang segitiga naik-turun.
 */
#include "apu_internal.h"

static const uint8_t g_triangle_sequence[32] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

void apu_triangle_reset(APUTriangle *triangle)
{
    if (triangle == NULL) {
        return;
    }

    triangle->enabled = false;
    triangle->control = false;
    triangle->linear_reload = 0;
    triangle->linear_counter = 0;
    triangle->linear_reload_flag = false;
    triangle->timer_period = 0;
    triangle->timer_counter = 0;
    triangle->sequencer = 0;
    triangle->length_halt = false;
    triangle->length = 0;
}

void apu_triangle_write(APUTriangle *triangle, uint8_t reg, uint8_t value)
{
    if (triangle == NULL) {
        return;
    }

    switch (reg) {
    case 0:
        triangle->control = (value & 0x80U) != 0U;
        triangle->length_halt = triangle->control;
        triangle->linear_reload = (uint8_t)(value & 0x7FU);
        break;

    case 2:
        triangle->timer_period = (uint16_t)((triangle->timer_period & 0x0700U) | value);
        break;

    case 3:
        triangle->timer_period =
            (uint16_t)((triangle->timer_period & 0x00FFU) | ((uint16_t)(value & 0x07U) << 8));
        triangle->length = g_apu_length_table[(value >> 3) & 0x1FU];
        triangle->linear_reload_flag = true;
        break;

    default:
        break;
    }
}

void apu_triangle_timer_clock(APUTriangle *triangle)
{
    if (triangle == NULL) {
        return;
    }

    if (triangle->timer_counter == 0U) {
        triangle->timer_counter = triangle->timer_period;
        if (triangle->length > 0U && triangle->linear_counter > 0U) {
            triangle->sequencer = (uint8_t)((triangle->sequencer + 1U) & 0x1FU);
        }
    } else {
        --triangle->timer_counter;
    }
}

void apu_triangle_linear_clock(APUTriangle *triangle)
{
    if (triangle == NULL) {
        return;
    }

    if (triangle->linear_reload_flag) {
        triangle->linear_counter = triangle->linear_reload;
    } else if (triangle->linear_counter > 0U) {
        --triangle->linear_counter;
    }

    if (!triangle->control) {
        triangle->linear_reload_flag = false;
    }
}

void apu_triangle_length_clock(APUTriangle *triangle)
{
    if (triangle == NULL) {
        return;
    }

    if (!triangle->length_halt && triangle->length > 0U) {
        --triangle->length;
    }
}

uint8_t apu_triangle_output(const APUTriangle *triangle)
{
    if (triangle == NULL || !triangle->enabled || triangle->length == 0U ||
        triangle->linear_counter == 0U) {
        return 0;
    }

    return g_triangle_sequence[triangle->sequencer & 0x1FU];
}
