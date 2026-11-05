/*
 * ppu.h - Antarmuka Picture Processing Unit RP2C02.
 *
 * PPU adalah unit grafis NES. Ia memiliki memori video sendiri (VRAM),
 * OAM untuk sprite, palette, dan berjalan dengan clock 3x lebih cepat
 * daripada CPU. CPU hanya melihat PPU melalui delapan register $2000-$2007.
 */
#ifndef TINYNES_PPU_PPU_H
#define TINYNES_PPU_PPU_H

#include "cartridge/cartridge.h"
#include "util/types.h"

typedef struct PPU PPU;

PPU *ppu_create(Cartridge *cartridge, Region region);
void ppu_destroy(PPU *ppu);
void ppu_reset(PPU *ppu);
void ppu_set_cartridge(PPU *ppu, Cartridge *cartridge);

/* Ganti region timing. PPU di-reset agar posisi scanline konsisten. */
void ppu_set_region(PPU *ppu, Region region);
Region ppu_region(const PPU *ppu);

/* Akses register dengan alamat 0..7 (sudah dimask oleh bus). */
uint8_t ppu_read_register(PPU *ppu, uint16_t address);
void ppu_write_register(PPU *ppu, uint16_t address, uint8_t value);

/* Maju satu dot PPU. Tiga pemanggilan setara satu siklus CPU. */
void ppu_clock(PPU *ppu);

/* Flag frame selesai di akhir/awal frame; scheduler mengosongkannya. */
bool ppu_frame_complete(const PPU *ppu);
void ppu_clear_frame_complete(PPU *ppu);

/* NMI adalah edge yang dikonsumsi sekali oleh CPU. */
bool ppu_take_nmi(PPU *ppu);

void ppu_oam_dma(PPU *ppu, const uint8_t *source);

const uint8_t *ppu_framebuffer(const PPU *ppu); /* 256x240 indeks palette */
uint32_t ppu_palette_rgb(uint8_t palette_index);

int ppu_scanline(const PPU *ppu);
int ppu_dot(const PPU *ppu);
uint64_t ppu_frame_count(const PPU *ppu);

#endif /* TINYNES_PPU_PPU_H */
