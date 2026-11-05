/*
 * apu.h - Antarmuka Audio Processing Unit RP2A03.
 *
 * APU menghasilkan lima kanal audio: dua pulse, satu triangle, satu noise,
 * dan satu DMC. APU berjalan satu langkah setiap siklus CPU. Sampel yang
 * dihasilkan disimpan di ring buffer internal dan diambil frontend secara
 * periodik.
 */
#ifndef TINYNES_APU_APU_H
#define TINYNES_APU_APU_H

#include "util/types.h"

typedef struct APU APU;
typedef uint8_t (*APUReadFn)(void *context, uint16_t address);

APU *apu_create(uint32_t sample_rate, Region region);
void apu_destroy(APU *apu);
void apu_reset(APU *apu);

/* Ganti region timing (frekuensi CPU dan tabel periode). APU di-reset. */
void apu_set_region(APU *apu, Region region);
Region apu_region(const APU *apu);

/* DMC membaca sampel melalui bus CPU. */
void apu_set_dmc_reader(APU *apu, APUReadFn read, void *context);

/* Maju satu siklus CPU. */
void apu_clock(APU *apu);

uint8_t apu_read_status(APU *apu);
void apu_write_register(APU *apu, uint16_t address, uint8_t value);

/* Frame IRQ dan DMC IRQ digabung menjadi satu sinyal. */
bool apu_irq_pending(const APU *apu);
bool apu_frame_irq(const APU *apu);

/*
 * Ambil sampel mono 16-bit. Mengembalikan jumlah sampel yang disalin.
 * Ring buffer tidak dialokasikan dinamis; bila penuh, sampel terlama dibuang.
 */
size_t apu_read_samples(APU *apu, int16_t *out, size_t max_samples);
uint32_t apu_sample_rate(const APU *apu);

#endif /* TINYNES_APU_APU_H */
