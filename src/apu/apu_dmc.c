/*
 * apu_dmc.c - Kanal DMC (Delta Modulation Channel).
 *
 * DMC memutar sampel PCM 1-bit dari PRG ROM. Emulator mengambil byte
 * melalui bus CPU, menggesernya bit per bit, lalu menaik-turunkan level
 * output 7 bit. Implementasi ini meniru timer, loop, dan IRQ, tetapi belum
 * menambahkan stall DMA CPU secara presisi.
 */
#include "apu_internal.h"

/* Periode timer DMC NTSC dan PAL berdasarkan $4010 bit 0-3. */
static const uint16_t g_dmc_periods_ntsc[16] = {
    428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
};

static const uint16_t g_dmc_periods_pal[16] = {
    398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118, 98, 78, 66, 50
};

static uint16_t dmc_period(Region region, uint8_t index)
{
    return nes_region_is_pal(region) ? g_dmc_periods_pal[index & 0x0FU]
                                     : g_dmc_periods_ntsc[index & 0x0FU];
}

void apu_dmc_reset(APUDMC *dmc, Region region)
{
    if (dmc == NULL) {
        return;
    }

    dmc->enabled = false;
    dmc->irq_enable = false;
    dmc->loop = false;
    dmc->region = (region == REGION_AUTO) ? REGION_NTSC : region;
    dmc->timer_period = dmc_period(dmc->region, 0);
    dmc->timer_counter = 0;
    dmc->output_level = 0;
    dmc->sample_address = 0xC000U;
    dmc->current_address = 0xC000U;
    dmc->sample_length = 0;
    dmc->bytes_remaining = 0;
    dmc->sample_buffer = 0;
    dmc->buffer_full = false;
    dmc->irq_flag = false;
    dmc->shift = 0;
    dmc->bits_remaining = 0;
}

static void dmc_fill_buffer(APU *apu)
{
    APUDMC *dmc = &apu->dmc;
    uint8_t byte = 0;

    if (dmc->buffer_full || dmc->bytes_remaining == 0U) {
        return;
    }

    if (apu->dmc_reader != NULL) {
        byte = apu->dmc_reader(apu->dmc_context, dmc->current_address);
    }

    dmc->sample_buffer = byte;
    dmc->buffer_full = true;

    ++dmc->current_address;
    if (dmc->current_address == 0x0000U) {
        dmc->current_address = 0x8000U; /* Alamat DMC berada di $8000-$FFFF. */
    }

    --dmc->bytes_remaining;

    if (dmc->bytes_remaining == 0U) {
        if (dmc->loop) {
            dmc->current_address = dmc->sample_address;
            dmc->bytes_remaining = dmc->sample_length;
        } else if (dmc->irq_enable) {
            dmc->irq_flag = true;
        }
    }
}

static void dmc_clock_output(APU *apu)
{
    APUDMC *dmc = &apu->dmc;

    if (dmc->bits_remaining == 0U) {
        if (!dmc->buffer_full) {
            dmc_fill_buffer(apu);
        }
        if (dmc->buffer_full) {
            dmc->shift = dmc->sample_buffer;
            dmc->buffer_full = false;
            dmc->bits_remaining = 8;
        }
    }

    if (dmc->bits_remaining == 0U) {
        return;
    }

    if ((dmc->shift & 0x01U) != 0U) {
        if (dmc->output_level <= 125U) {
            dmc->output_level = (uint8_t)(dmc->output_level + 2U);
        }
    } else if (dmc->output_level >= 2U) {
        dmc->output_level = (uint8_t)(dmc->output_level - 2U);
    }

    dmc->shift = (uint8_t)(dmc->shift >> 1);
    --dmc->bits_remaining;
}

void apu_dmc_timer_clock(APU *apu)
{
    APUDMC *dmc = NULL;

    if (apu == NULL) {
        return;
    }

    dmc = &apu->dmc;
    if (dmc->timer_counter == 0U) {
        dmc->timer_counter = dmc->timer_period;
        if (dmc->enabled) {
            dmc_clock_output(apu);
        }
    } else {
        --dmc->timer_counter;
    }
}

void apu_dmc_write(APU *apu, uint8_t reg, uint8_t value)
{
    APUDMC *dmc = NULL;

    if (apu == NULL) {
        return;
    }

    dmc = &apu->dmc;

    switch (reg) {
    case 0:
        dmc->irq_enable = (value & 0x80U) != 0U;
        if (!dmc->irq_enable) {
            dmc->irq_flag = false;
        }
        dmc->loop = (value & 0x40U) != 0U;
        dmc->timer_period = dmc_period(dmc->region, value & 0x0FU);
        break;

    case 1:
        dmc->output_level = (uint8_t)(value & 0x7FU);
        break;

    case 2:
        dmc->sample_address = (uint16_t)(0xC000U + ((uint16_t)value << 6));
        break;

    case 3:
        dmc->sample_length = (uint16_t)(((uint16_t)value << 4) + 1U);
        break;

    default:
        break;
    }
}

void apu_dmc_set_enabled(APU *apu, bool enabled)
{
    APUDMC *dmc = NULL;

    if (apu == NULL) {
        return;
    }

    dmc = &apu->dmc;

    if (enabled && !dmc->enabled) {
        if (dmc->bytes_remaining == 0U) {
            dmc->current_address = dmc->sample_address;
            dmc->bytes_remaining = dmc->sample_length;
        }
    } else if (!enabled) {
        dmc->bytes_remaining = 0;
    }

    dmc->enabled = enabled;
}
