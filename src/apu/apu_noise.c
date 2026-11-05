/*
 * apu_noise.c - Kanal noise (LFSR).
 *
 * Noise memakai Linear Feedback Shift Register 15 bit. Bit keluaran
 * membentuk derau pseudorandom yang dipakai untuk efek drum, ledakan, dan
 * angin. Mode bit 7 memilih feedback 6 bit (bunyi lebih "metalik").
 */
#include "apu_internal.h"

/* Periode timer NTSC dan PAL untuk indeks $400E bit 0-3. */
static const uint16_t g_noise_periods_ntsc[16] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

static const uint16_t g_noise_periods_pal[16] = {
    4, 8, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708, 944, 1890, 3778
};

static uint16_t noise_period(Region region, uint8_t index)
{
    return nes_region_is_pal(region) ? g_noise_periods_pal[index & 0x0FU]
                                     : g_noise_periods_ntsc[index & 0x0FU];
}

void apu_noise_reset(APUNoise *noise, Region region)
{
    if (noise == NULL) {
        return;
    }

    noise->enabled = false;
    noise->envelope.start = false;
    noise->envelope.loop = false;
    noise->envelope.constant = false;
    noise->envelope.volume = 0;
    noise->envelope.divider = 0;
    noise->envelope.decay = 0;
    noise->mode = false;
    noise->region = (region == REGION_AUTO) ? REGION_NTSC : region;
    noise->timer_period = noise_period(noise->region, 0);
    noise->timer_counter = 0;
    noise->shift = 1;
    noise->length_halt = false;
    noise->length = 0;
}

void apu_noise_write(APUNoise *noise, uint8_t reg, uint8_t value)
{
    if (noise == NULL) {
        return;
    }

    switch (reg) {
    case 0:
        noise->length_halt = (value & 0x20U) != 0U;
        noise->envelope.loop = noise->length_halt;
        noise->envelope.constant = (value & 0x10U) != 0U;
        noise->envelope.volume = (uint8_t)(value & 0x0FU);
        break;

    case 2:
        noise->mode = (value & 0x80U) != 0U;
        noise->timer_period = noise_period(noise->region, value & 0x0FU);
        break;

    case 3:
        noise->length = g_apu_length_table[(value >> 3) & 0x1FU];
        noise->envelope.start = true;
        break;

    default:
        break;
    }
}

void apu_noise_timer_clock(APUNoise *noise)
{
    uint16_t feedback = 0;

    if (noise == NULL) {
        return;
    }

    if (noise->timer_counter == 0U) {
        noise->timer_counter = noise->timer_period;

        if (noise->mode) {
            feedback = (uint16_t)((noise->shift & 0x01U) ^ ((noise->shift >> 6) & 0x01U));
        } else {
            feedback = (uint16_t)((noise->shift & 0x01U) ^ ((noise->shift >> 1) & 0x01U));
        }

        noise->shift = (uint16_t)(noise->shift >> 1);
        if (feedback != 0U) {
            noise->shift = (uint16_t)(noise->shift | 0x4000U);
        }
    } else {
        --noise->timer_counter;
    }
}

void apu_noise_envelope_clock(APUNoise *noise)
{
    if (noise != NULL) {
        apu_envelope_clock(&noise->envelope);
    }
}

void apu_noise_length_clock(APUNoise *noise)
{
    if (noise == NULL) {
        return;
    }

    if (!noise->length_halt && noise->length > 0U) {
        --noise->length;
    }
}

uint8_t apu_noise_output(const APUNoise *noise)
{
    if (noise == NULL || !noise->enabled || noise->length == 0U) {
        return 0;
    }

    /* Bit 0 = 1 berarti output diredam. */
    if ((noise->shift & 0x01U) != 0U) {
        return 0;
    }

    return apu_envelope_volume(&noise->envelope);
}
