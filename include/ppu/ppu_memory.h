/*
 * ppu_memory.h - Akses memori video PPU tanpa efek samping register.
 *
 * Fungsi di sini dipakai oleh rendering dan oleh register $2007. Berbeda
 * dengan ppu_read_register(), tidak ada buffering atau increment alamat.
 */
#ifndef TINYNES_PPU_PPU_MEMORY_H
#define TINYNES_PPU_PPU_MEMORY_H

#include "ppu/ppu.h"
#include "util/types.h"

/* Alamat 14-bit: pattern, nametable, attribute, palette. */
uint8_t ppu_memory_read(PPU *ppu, uint16_t address);
void ppu_memory_write(PPU *ppu, uint16_t address, uint8_t value);

/* OAM mentah. */
uint8_t ppu_oam_read(PPU *ppu, uint8_t index);
void ppu_oam_write(PPU *ppu, uint8_t index, uint8_t value);

/* Palette RAM dengan mirroring $3F10/$3F14/$3F18/$3F1C. */
uint8_t ppu_palette_read(PPU *ppu, uint8_t index);
void ppu_palette_write(PPU *ppu, uint8_t index, uint8_t value);

#endif /* TINYNES_PPU_PPU_MEMORY_H */
