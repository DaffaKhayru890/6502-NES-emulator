/*
 * cartridge_internal.h - Struktur Cartridge yang dipakai loader dan mapper.
 */
#ifndef TINYNES_CARTRIDGE_INTERNAL_H
#define TINYNES_CARTRIDGE_INTERNAL_H

#include "cartridge/cartridge.h"
#include "cartridge/mapper.h"

struct Cartridge {
    uint8_t *prg_rom;
    size_t prg_rom_size;

    uint8_t *chr_mem;
    size_t chr_size;
    bool chr_is_ram;

    uint8_t prg_ram[NES_PRG_RAM_SIZE];

    Mirroring mirroring;
    int mapper_number;
    uint8_t submapper;
    Region region;
    bool nes2;
    bool battery;
    bool trainer;

    Mapper *mapper;
};

/* Dipakai ines_loader.c dan pengujian. */
Cartridge *cartridge_allocate(void);

#endif /* TINYNES_CARTRIDGE_INTERNAL_H */
