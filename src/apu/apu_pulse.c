/*
 * apu_pulse.c - Dua kanal pulse (square wave).
 *
 * Setiap kanal pulse memiliki timer 11-bit, sequencer duty 8 langkah,
 * envelope, length counter, dan sweep unit. Sweep dapat mengubah period
 * secara otomatis (efek "slide").
 */
#include "apu_internal.h"

static const uint8_t g_duty_table[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 0, 0, 1, 1, 1, 1, 1}
};

void apu_pulse_reset(APUPulse *pulse)
{
    if (pulse == NULL) {
        return;
    }

    pulse->enabled = false;
    pulse->envelope.start = false;
    pulse->envelope.loop = false;
    pulse->envelope.constant = false;
    pulse->envelope.volume = 0;
    pulse->envelope.divider = 0;
    pulse->envelope.decay = 0;
    pulse->sweep.enabled = false;
    pulse->sweep.negate = false;
    pulse->sweep.reload = false;
    pulse->sweep.period = 0;
    pulse->sweep.shift = 0;
    pulse->sweep.divider = 0;
    pulse->timer_period = 0;
    pulse->timer_counter = 0;
    pulse->duty = 0;
    pulse->sequencer = 0;
    pulse->length_halt = false;
    pulse->length = 0;
}

void apu_pulse_write(APUPulse *pulse, uint8_t reg, uint8_t value, bool pulse_one)
{
    if (pulse == NULL) {
        return;
    }

    switch (reg) {
    case 0:
        pulse->duty = (uint8_t)((value >> 6) & 0x03U);
        pulse->length_halt = (value & 0x20U) != 0U;
        pulse->envelope.loop = pulse->length_halt;
        pulse->envelope.constant = (value & 0x10U) != 0U;
        pulse->envelope.volume = (uint8_t)(value & 0x0FU);
        break;

    case 1:
        pulse->sweep.enabled = (value & 0x80U) != 0U;
        pulse->sweep.period = (uint8_t)((value >> 4) & 0x07U);
        pulse->sweep.negate = (value & 0x08U) != 0U;
        pulse->sweep.shift = (uint8_t)(value & 0x07U);
        pulse->sweep.reload = true;
        break;

    case 2:
        pulse->timer_period = (uint16_t)((pulse->timer_period & 0x0700U) | value);
        break;

    case 3:
        pulse->timer_period =
            (uint16_t)((pulse->timer_period & 0x00FFU) | ((uint16_t)(value & 0x07U) << 8));
        pulse->length = g_apu_length_table[(value >> 3) & 0x1FU];
        pulse->sequencer = 0;
        pulse->envelope.start = true;
        (void)pulse_one;
        break;

    default:
        break;
    }
}

static int pulse_sweep_target(const APUPulse *pulse, bool pulse_one)
{
    int period = (int)pulse->timer_period;
    int change = period >> pulse->sweep.shift;

    if (pulse->sweep.negate) {
        /* Pulse 1 memakai komplemen satu, pulse 2 memakai komplemen dua. */
        return period - change - (pulse_one ? 1 : 0);
    }
    return period + change;
}

static bool pulse_sweep_muted(const APUPulse *pulse, bool pulse_one)
{
    int target = 0;

    if (pulse->timer_period < 8U) {
        return true;
    }

    target = pulse_sweep_target(pulse, pulse_one);
    return target > 0x7FF || target < 0;
}

void apu_pulse_timer_clock(APUPulse *pulse)
{
    if (pulse == NULL) {
        return;
    }

    if (pulse->timer_counter == 0U) {
        pulse->timer_counter = pulse->timer_period;
        pulse->sequencer = (uint8_t)((pulse->sequencer + 1U) & 0x07U);
    } else {
        --pulse->timer_counter;
    }
}

void apu_pulse_envelope_clock(APUPulse *pulse)
{
    if (pulse != NULL) {
        apu_envelope_clock(&pulse->envelope);
    }
}

void apu_pulse_sweep_clock(APUPulse *pulse, bool pulse_one)
{
    int target = 0;

    if (pulse == NULL) {
        return;
    }

    if (pulse->sweep.divider == 0U) {
        if (pulse->sweep.enabled && pulse->sweep.shift > 0U &&
            !pulse_sweep_muted(pulse, pulse_one)) {
            target = pulse_sweep_target(pulse, pulse_one);
            if (target >= 0 && target <= 0x7FF) {
                pulse->timer_period = (uint16_t)target;
            }
        }
    } else {
        --pulse->sweep.divider;
    }

    if (pulse->sweep.divider == 0U || pulse->sweep.reload) {
        pulse->sweep.divider = pulse->sweep.period;
        pulse->sweep.reload = false;
    }
}

void apu_pulse_length_clock(APUPulse *pulse)
{
    if (pulse == NULL) {
        return;
    }

    if (!pulse->length_halt && pulse->length > 0U) {
        --pulse->length;
    }
}

uint8_t apu_pulse_output(const APUPulse *pulse, bool pulse_one)
{
    uint8_t duty_bit = 0;

    if (pulse == NULL || !pulse->enabled || pulse->length == 0U) {
        return 0;
    }

    if (pulse_sweep_muted(pulse, pulse_one)) {
        return 0;
    }

    duty_bit = g_duty_table[pulse->duty & 0x03U][pulse->sequencer & 0x07U];
    if (duty_bit == 0U) {
        return 0;
    }

    return apu_envelope_volume(&pulse->envelope);
}
