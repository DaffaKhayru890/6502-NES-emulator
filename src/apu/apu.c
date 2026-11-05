/*
 * apu.c - Siklus hidup, register, dan sampling APU.
 */
#include "apu/apu.h"

#include "apu_internal.h"
#include "util/log.h"

#include <stdlib.h>
#include <string.h>

const uint8_t g_apu_length_table[APU_LENGTH_TABLE_SIZE] = {
    10,  254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
    12,  16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

APU *apu_create(uint32_t sample_rate, Region region)
{
    APU *apu = NULL;

    if (sample_rate == 0U) {
        sample_rate = 44100U;
    }

    apu = (APU *)calloc(1, sizeof(*apu));
    if (apu == NULL) {
        log_error("Alokasi memori APU gagal.");
        return NULL;
    }

    apu->sample_rate = sample_rate;
    apu->region = (region == REGION_AUTO) ? REGION_NTSC : region;
    apu_reset(apu);
    return apu;
}

void apu_destroy(APU *apu)
{
    free(apu);
}

void apu_set_region(APU *apu, Region region)
{
    if (apu == NULL || region == REGION_AUTO) {
        return;
    }

    apu->region = region;
    apu_reset(apu);
}

Region apu_region(const APU *apu)
{
    return (apu != NULL) ? apu->region : REGION_NTSC;
}

void apu_reset(APU *apu)
{
    if (apu == NULL) {
        return;
    }

    apu_pulse_reset(&apu->pulse1);
    apu_pulse_reset(&apu->pulse2);
    apu_triangle_reset(&apu->triangle);
    apu_noise_reset(&apu->noise, apu->region);
    apu_dmc_reset(&apu->dmc, apu->region);
    apu_frame_counter_reset(apu);

    apu->sample_accumulator = 0.0;
    apu->sample_head = 0;
    apu->sample_tail = 0;
    apu->sample_count = 0;
    apu->total_cycles = 0;
    apu->frame_irq = false;
}

void apu_set_dmc_reader(APU *apu, APUReadFn read, void *context)
{
    if (apu == NULL) {
        return;
    }

    apu->dmc_reader = read;
    apu->dmc_context = context;
}

void apu_envelope_clock(APUEnvelope *envelope)
{
    if (envelope == NULL) {
        return;
    }

    if (envelope->start) {
        envelope->start = false;
        envelope->decay = 15;
        envelope->divider = envelope->volume;
    } else if (envelope->divider == 0U) {
        envelope->divider = envelope->volume;
        if (envelope->decay > 0U) {
            --envelope->decay;
        } else if (envelope->loop) {
            envelope->decay = 15;
        }
    } else {
        --envelope->divider;
    }
}

uint8_t apu_envelope_volume(const APUEnvelope *envelope)
{
    if (envelope == NULL) {
        return 0;
    }
    return envelope->constant ? envelope->volume : envelope->decay;
}

static void push_sample(APU *apu, int16_t sample)
{
    if (apu == NULL) {
        return;
    }

    if (apu->sample_count >= APU_SAMPLE_BUFFER_SIZE) {
        /* Buang sampel terlama agar produsen tidak pernah memblokir. */
        apu->sample_tail = (apu->sample_tail + 1U) % APU_SAMPLE_BUFFER_SIZE;
        --apu->sample_count;
    }

    apu->samples[apu->sample_head] = sample;
    apu->sample_head = (apu->sample_head + 1U) % APU_SAMPLE_BUFFER_SIZE;
    ++apu->sample_count;
}

void apu_clock(APU *apu)
{
    double sample_step = 0.0;

    if (apu == NULL) {
        return;
    }

    ++apu->total_cycles;

    apu_frame_counter_clock(apu);
    apu_pulse_timer_clock(&apu->pulse1);
    apu_pulse_timer_clock(&apu->pulse2);
    apu_triangle_timer_clock(&apu->triangle);
    apu_noise_timer_clock(&apu->noise);
    apu_dmc_timer_clock(apu);

    /* Rasio sampel terhadap clock CPU region, memakai akumulator pecahan. */
    sample_step = (double)apu->sample_rate / nes_region_cpu_frequency(apu->region);
    apu->sample_accumulator += sample_step;
    if (apu->sample_accumulator >= 1.0) {
        apu->sample_accumulator -= 1.0;
        push_sample(apu, apu_mix(apu));
    }
}

uint8_t apu_read_status(APU *apu)
{
    uint8_t status = 0;

    if (apu == NULL) {
        return 0;
    }

    if (apu->pulse1.length > 0U) {
        status = (uint8_t)(status | 0x01U);
    }
    if (apu->pulse2.length > 0U) {
        status = (uint8_t)(status | 0x02U);
    }
    if (apu->triangle.length > 0U) {
        status = (uint8_t)(status | 0x04U);
    }
    if (apu->noise.length > 0U) {
        status = (uint8_t)(status | 0x08U);
    }
    if (apu->dmc.bytes_remaining > 0U) {
        status = (uint8_t)(status | 0x10U);
    }
    if (apu->frame_irq) {
        status = (uint8_t)(status | 0x40U);
    }
    if (apu->dmc.irq_flag) {
        status = (uint8_t)(status | 0x80U);
    }

    /* Membaca $4015 menghapus frame IRQ dan DMC IRQ. */
    apu->frame_irq = false;
    apu->dmc.irq_flag = false;
    return status;
}

void apu_write_register(APU *apu, uint16_t address, uint8_t value)
{
    if (apu == NULL) {
        return;
    }

    if (address >= 0x4000U && address <= 0x4003U) {
        apu_pulse_write(&apu->pulse1, (uint8_t)(address - 0x4000U), value, true);
    } else if (address >= 0x4004U && address <= 0x4007U) {
        apu_pulse_write(&apu->pulse2, (uint8_t)(address - 0x4004U), value, false);
    } else if (address >= 0x4008U && address <= 0x400BU) {
        apu_triangle_write(&apu->triangle, (uint8_t)(address - 0x4008U), value);
    } else if (address >= 0x400CU && address <= 0x400FU) {
        apu_noise_write(&apu->noise, (uint8_t)(address - 0x400CU), value);
    } else if (address >= 0x4010U && address <= 0x4013U) {
        apu_dmc_write(apu, (uint8_t)(address - 0x4010U), value);
    } else if (address == 0x4015U) {
        apu->pulse1.enabled = (value & 0x01U) != 0U;
        apu->pulse2.enabled = (value & 0x02U) != 0U;
        apu->triangle.enabled = (value & 0x04U) != 0U;
        apu->noise.enabled = (value & 0x08U) != 0U;

        if (!apu->pulse1.enabled) {
            apu->pulse1.length = 0;
        }
        if (!apu->pulse2.enabled) {
            apu->pulse2.length = 0;
        }
        if (!apu->triangle.enabled) {
            apu->triangle.length = 0;
        }
        if (!apu->noise.enabled) {
            apu->noise.length = 0;
        }

        apu_dmc_set_enabled(apu, (value & 0x10U) != 0U);
    } else if (address == 0x4017U) {
        apu_frame_counter_write(apu, value);
    }
}

bool apu_irq_pending(const APU *apu)
{
    return apu != NULL && (apu->frame_irq || apu->dmc.irq_flag);
}

bool apu_frame_irq(const APU *apu)
{
    return apu != NULL && apu->frame_irq;
}

size_t apu_read_samples(APU *apu, int16_t *out, size_t max_samples)
{
    size_t count = 0;

    if (apu == NULL || out == NULL) {
        return 0;
    }

    while (count < max_samples && apu->sample_count > 0U) {
        out[count] = apu->samples[apu->sample_tail];
        apu->sample_tail = (apu->sample_tail + 1U) % APU_SAMPLE_BUFFER_SIZE;
        --apu->sample_count;
        ++count;
    }

    return count;
}

uint32_t apu_sample_rate(const APU *apu)
{
    return (apu != NULL) ? apu->sample_rate : 0U;
}
