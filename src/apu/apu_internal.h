/*
 * apu_internal.h - State dan fungsi internal APU.
 */
#ifndef TINYNES_APU_INTERNAL_H
#define TINYNES_APU_INTERNAL_H

#include "apu/apu.h"

#define APU_SAMPLE_BUFFER_SIZE 8192
#define APU_LENGTH_TABLE_SIZE 32

typedef struct APUEnvelope {
    bool start;
    bool loop;
    bool constant;
    uint8_t volume;
    uint8_t divider;
    uint8_t decay;
} APUEnvelope;

typedef struct APUSweep {
    bool enabled;
    bool negate;
    bool reload;
    uint8_t period;
    uint8_t shift;
    uint8_t divider;
} APUSweep;

typedef struct APUPulse {
    bool enabled;
    APUEnvelope envelope;
    APUSweep sweep;
    uint16_t timer_period;
    uint16_t timer_counter;
    uint8_t duty;
    uint8_t sequencer;
    bool length_halt;
    uint8_t length;
} APUPulse;

typedef struct APUTriangle {
    bool enabled;
    bool control;
    uint8_t linear_reload;
    uint8_t linear_counter;
    bool linear_reload_flag;
    uint16_t timer_period;
    uint16_t timer_counter;
    uint8_t sequencer;
    bool length_halt;
    uint8_t length;
} APUTriangle;

typedef struct APUNoise {
    bool enabled;
    APUEnvelope envelope;
    bool mode;
    Region region;
    uint16_t timer_period;
    uint16_t timer_counter;
    uint16_t shift;
    bool length_halt;
    uint8_t length;
} APUNoise;

typedef struct APUDMC {
    bool enabled;
    bool irq_enable;
    bool loop;
    Region region;
    uint16_t timer_period;
    uint16_t timer_counter;
    uint8_t output_level;
    uint16_t sample_address;
    uint16_t current_address;
    uint16_t sample_length;
    uint16_t bytes_remaining;
    uint8_t sample_buffer;
    bool buffer_full;
    bool irq_flag;
    uint8_t shift;
    uint8_t bits_remaining;
} APUDMC;

struct APU {
    uint32_t sample_rate;
    Region region;
    double sample_accumulator;
    int16_t samples[APU_SAMPLE_BUFFER_SIZE];
    size_t sample_head;
    size_t sample_tail;
    size_t sample_count;

    APUPulse pulse1;
    APUPulse pulse2;
    APUTriangle triangle;
    APUNoise noise;
    APUDMC dmc;

    uint8_t frame_mode;
    uint32_t frame_cycle;
    bool frame_irq;

    APUReadFn dmc_reader;
    void *dmc_context;
    uint64_t total_cycles;
};

/* Tabel panjang (length counter), satuan half-frame. */
extern const uint8_t g_apu_length_table[APU_LENGTH_TABLE_SIZE];

/* Helper envelope dipakai pulse dan noise. */
void apu_envelope_clock(APUEnvelope *envelope);
uint8_t apu_envelope_volume(const APUEnvelope *envelope);

/* apu_pulse.c */
void apu_pulse_reset(APUPulse *pulse);
void apu_pulse_write(APUPulse *pulse, uint8_t reg, uint8_t value, bool pulse_one);
void apu_pulse_timer_clock(APUPulse *pulse);
void apu_pulse_envelope_clock(APUPulse *pulse);
void apu_pulse_sweep_clock(APUPulse *pulse, bool pulse_one);
void apu_pulse_length_clock(APUPulse *pulse);
uint8_t apu_pulse_output(const APUPulse *pulse, bool pulse_one);

/* apu_triangle.c */
void apu_triangle_reset(APUTriangle *triangle);
void apu_triangle_write(APUTriangle *triangle, uint8_t reg, uint8_t value);
void apu_triangle_timer_clock(APUTriangle *triangle);
void apu_triangle_linear_clock(APUTriangle *triangle);
void apu_triangle_length_clock(APUTriangle *triangle);
uint8_t apu_triangle_output(const APUTriangle *triangle);

/* apu_noise.c */
void apu_noise_reset(APUNoise *noise, Region region);
void apu_noise_write(APUNoise *noise, uint8_t reg, uint8_t value);
void apu_noise_timer_clock(APUNoise *noise);
void apu_noise_envelope_clock(APUNoise *noise);
void apu_noise_length_clock(APUNoise *noise);
uint8_t apu_noise_output(const APUNoise *noise);

/* apu_dmc.c */
void apu_dmc_reset(APUDMC *dmc, Region region);
void apu_dmc_write(APU *apu, uint8_t reg, uint8_t value);
void apu_dmc_timer_clock(APU *apu);
void apu_dmc_set_enabled(APU *apu, bool enabled);

/* apu_frame_counter.c */
void apu_frame_counter_reset(APU *apu);
void apu_frame_counter_clock(APU *apu);
void apu_frame_counter_write(APU *apu, uint8_t value);

/* apu_mixer.c */
int16_t apu_mix(const APU *apu);

#endif /* TINYNES_APU_INTERNAL_H */
