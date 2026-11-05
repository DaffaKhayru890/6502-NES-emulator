/*
 * apu_mixer.c - Pencampuran lima kanal APU.
 *
 * Rumus nonlinear yang dipakai adalah pendekatan yang lazim di emulator NES:
 *   pulse = 95.88 / (8128 / (p1 + p2) + 100)
 *   tnd   = 159.79 / (1 / (t/8227 + n/12241 + d/22638) + 100)
 * Hasilnya bukan replika analog sempurna, tetapi jauh lebih dekat ke
 * karakteristik hardware daripada penjumlahan linear sederhana.
 */
#include "apu_internal.h"

int16_t apu_mix(const APU *apu)
{
    float pulse1 = 0.0f;
    float pulse2 = 0.0f;
    float triangle = 0.0f;
    float noise = 0.0f;
    float dmc = 0.0f;
    float pulse_sum = 0.0f;
    float pulse_out = 0.0f;
    float tnd_in = 0.0f;
    float tnd_out = 0.0f;
    float signal = 0.0f;
    float sample = 0.0f;

    if (apu == NULL) {
        return 0;
    }

    pulse1 = (float)apu_pulse_output(&apu->pulse1, true);
    pulse2 = (float)apu_pulse_output(&apu->pulse2, false);
    triangle = (float)apu_triangle_output(&apu->triangle);
    noise = (float)apu_noise_output(&apu->noise);
    dmc = (float)apu->dmc.output_level;

    pulse_sum = pulse1 + pulse2;
    if (pulse_sum > 0.0f) {
        pulse_out = 95.88f / ((8128.0f / pulse_sum) + 100.0f);
    }

    tnd_in = (triangle / 8227.0f) + (noise / 12241.0f) + (dmc / 22638.0f);
    if (tnd_in > 0.0f) {
        tnd_out = 159.79f / ((1.0f / tnd_in) + 100.0f);
    }

    signal = pulse_out + tnd_out;

    /* Geser ke domain signed dan beri headroom agar tidak clipping. */
    sample = (signal - 0.15f) * 40000.0f;
    if (sample > 32767.0f) {
        sample = 32767.0f;
    } else if (sample < -32768.0f) {
        sample = -32768.0f;
    }

    return (int16_t)sample;
}
