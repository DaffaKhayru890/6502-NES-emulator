/*
 * nes.h - Komposisi seluruh sistem NES.
 *
 * Nes adalah "root object": ia memiliki CPU, PPU, APU, CPU Bus, cartridge,
 * dan dua controller. Semua komponen dihubungkan secara eksplisit di sini;
 * tidak ada state hardware global.
 */
#ifndef TINYNES_CORE_NES_H
#define TINYNES_CORE_NES_H

#include "bus/cpu_bus.h"
#include "core/error.h"
#include "cpu/cpu_2a03.h"
#include "util/types.h"

typedef struct Nes Nes;

/* Sample rate audio ditentukan frontend sebelum emulator dibuat. */
Nes *nes_create(uint32_t audio_sample_rate, Region region);
void nes_destroy(Nes *nes);

/* Memuat ROM dari disk atau buffer (buffer berguna untuk pengujian). */
ErrorCode nes_load_rom(Nes *nes, const char *path);
ErrorCode nes_load_rom_data(Nes *nes, const uint8_t *data, size_t size);

void nes_reset(Nes *nes);

/* Jalankan emulasi sampai satu frame PPU selesai. */
void nes_run_frame(Nes *nes);

/* Status tombol controller, bit mengikuti ControllerButton. */
void nes_set_buttons(Nes *nes, int port, uint8_t buttons);

/* Region timing efektif setelah ROM dimuat dan frame rate yang dipakai pacing. */
Region nes_region(const Nes *nes);
double nes_frame_rate(const Nes *nes);

const uint8_t *nes_framebuffer(const Nes *nes);
size_t nes_read_audio(Nes *nes, int16_t *out, size_t max_samples);

void nes_set_debug(Nes *nes, bool enabled);
bool nes_debug_enabled(const Nes *nes);

/* Akses untuk debugging dan pengujian. */
CPU *nes_cpu(Nes *nes);
PPU *nes_ppu(Nes *nes);
APU *nes_apu(Nes *nes);
CPUBus *nes_bus(Nes *nes);
Cartridge *nes_cartridge(Nes *nes);

#endif /* TINYNES_CORE_NES_H */
