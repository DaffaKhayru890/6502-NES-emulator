/*
 * mapper_nrom.h - Mapper 0 (NROM).
 *
 * NROM adalah mapper paling sederhana: PRG ROM 16 KiB (dicerminkan dua kali)
 * atau 32 KiB, serta CHR ROM/RAM 8 KiB tanpa bank switching.
 */
#ifndef TINYNES_CARTRIDGE_MAPPER_NROM_H
#define TINYNES_CARTRIDGE_MAPPER_NROM_H

#include "cartridge/cartridge.h"
#include "cartridge/mapper.h"

/* Context mapper adalah Cartridge itu sendiri; tidak ada alokasi tambahan. */
Mapper *mapper_nrom_create(Cartridge *cartridge);

#endif /* TINYNES_CARTRIDGE_MAPPER_NROM_H */
