/*
 * apu_frame_counter.c - Frame sequencer APU.
 *
 * Frame counter membangkitkan clock kuarter dan setengah frame yang dipakai
 * envelope, sweep, length counter, dan linear counter. Mode 4-step (default)
 * juga dapat memicu IRQ; mode 5-step tidak.
 */
#include "apu_internal.h"

static void clock_quarter_frame(APU *apu)
{
    apu_pulse_envelope_clock(&apu->pulse1);
    apu_pulse_envelope_clock(&apu->pulse2);
    apu_noise_envelope_clock(&apu->noise);
    apu_pulse_sweep_clock(&apu->pulse1, true);
    apu_pulse_sweep_clock(&apu->pulse2, false);
    apu_triangle_linear_clock(&apu->triangle);
}

static void clock_half_frame(APU *apu)
{
    apu_pulse_length_clock(&apu->pulse1);
    apu_pulse_length_clock(&apu->pulse2);
    apu_triangle_length_clock(&apu->triangle);
    apu_noise_length_clock(&apu->noise);
}

void apu_frame_counter_reset(APU *apu)
{
    if (apu == NULL) {
        return;
    }

    apu->frame_mode = 0;
    apu->frame_cycle = 0;
    apu->frame_irq = false;
}

void apu_frame_counter_write(APU *apu, uint8_t value)
{
    if (apu == NULL) {
        return;
    }

    apu->frame_mode = value;
    apu->frame_cycle = 0;
    apu->frame_irq = false;

    /* Mode 5-step langsung menjalankan satu kuarter dan setengah frame. */
    if ((value & 0x80U) != 0U) {
        clock_quarter_frame(apu);
        clock_half_frame(apu);
    }
}

void apu_frame_counter_clock(APU *apu)
{
    bool pal = false;
    uint32_t quarter_1 = 0;
    uint32_t half_1 = 0;
    uint32_t quarter_2 = 0;
    uint32_t half_2_irq = 0;
    uint32_t irq_cycle = 0;
    uint32_t reset_cycle = 0;

    if (apu == NULL) {
        return;
    }

    pal = nes_region_is_pal(apu->region);
    ++apu->frame_cycle;

    if ((apu->frame_mode & 0x80U) != 0U) {
        /* Mode 5-step. PAL memakai periode lebih panjang karena CPU 1,66 MHz. */
        quarter_1 = pal ? 8313U : 7457U;
        half_1 = pal ? 16639U : 14913U;
        quarter_2 = pal ? 24939U : 22371U;
        half_2_irq = pal ? 41565U : 37281U;
        reset_cycle = pal ? 41566U : 37282U;

        if (apu->frame_cycle == quarter_1) {
            clock_quarter_frame(apu);
        } else if (apu->frame_cycle == half_1) {
            clock_quarter_frame(apu);
            clock_half_frame(apu);
        } else if (apu->frame_cycle == quarter_2) {
            clock_quarter_frame(apu);
        } else if (apu->frame_cycle == half_2_irq) {
            clock_quarter_frame(apu);
            clock_half_frame(apu);
        } else if (apu->frame_cycle >= reset_cycle) {
            apu->frame_cycle = 0;
        }
    } else {
        /* Mode 4-step; PAL juga lebih panjang. */
        quarter_1 = pal ? 8313U : 7457U;
        half_1 = pal ? 16639U : 14913U;
        quarter_2 = pal ? 24939U : 22371U;
        half_2_irq = pal ? 33252U : 29828U;
        irq_cycle = pal ? 33253U : 29829U;
        reset_cycle = pal ? 33254U : 29830U;

        if (apu->frame_cycle == quarter_1) {
            clock_quarter_frame(apu);
        } else if (apu->frame_cycle == half_1) {
            clock_quarter_frame(apu);
            clock_half_frame(apu);
        } else if (apu->frame_cycle == quarter_2) {
            clock_quarter_frame(apu);
        } else if (apu->frame_cycle == half_2_irq) {
            clock_quarter_frame(apu);
            clock_half_frame(apu);
        } else if (apu->frame_cycle == irq_cycle) {
            if ((apu->frame_mode & 0x40U) == 0U) {
                apu->frame_irq = true;
            }
        } else if (apu->frame_cycle >= reset_cycle) {
            apu->frame_cycle = 0;
        }
    }
}
