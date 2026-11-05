/*
 * cartridge.h - Representasi cartridge NES.
 *
 * Cartridge memiliki data ROM/RAM dan satu Mapper. Modul lain tidak perlu
 * mengetahui format iNES; mereka hanya memanggil fungsi baca/tulis di sini.
 */
#ifndef TINYNES_CARTRIDGE_CARTRIDGE_H
#define TINYNES_CARTRIDGE_CARTRIDGE_H

#include "core/error.h"
#include "util/types.h"

typedef enum Mirroring {
    MIRRORING_HORIZONTAL = 0,
    MIRRORING_VERTICAL = 1,
    MIRRORING_FOUR_SCREEN = 2
} Mirroring;

typedef struct Cartridge Cartridge;

/* Memuat file .nes dari disk. Mengisi *error bila gagal. */
Cartridge *cartridge_load_file(const char *path, ErrorCode *error);

/* Memuat langsung dari buffer; berguna untuk unit test dan ROM sintetis. */
Cartridge *cartridge_load_memory(const uint8_t *data, size_t size, ErrorCode *error);

void cartridge_destroy(Cartridge *cartridge);

/* Akses dari CPU ($6000-$FFFF) dan PPU ($0000-$1FFF). */
uint8_t cartridge_cpu_read(Cartridge *cartridge, uint16_t address);
void cartridge_cpu_write(Cartridge *cartridge, uint16_t address, uint8_t value);
uint8_t cartridge_ppu_read(Cartridge *cartridge, uint16_t address);
void cartridge_ppu_write(Cartridge *cartridge, uint16_t address, uint8_t value);

/* Metadata untuk logging dan pengujian. */
int cartridge_mapper_number(const Cartridge *cartridge);
uint8_t cartridge_submapper(const Cartridge *cartridge);
bool cartridge_is_nes2(const Cartridge *cartridge);
Region cartridge_region(const Cartridge *cartridge);
Mirroring cartridge_mirroring(const Cartridge *cartridge);
size_t cartridge_prg_rom_size(const Cartridge *cartridge);
size_t cartridge_chr_size(const Cartridge *cartridge);
bool cartridge_chr_is_ram(const Cartridge *cartridge);
bool cartridge_has_battery(const Cartridge *cartridge);
bool cartridge_has_trainer(const Cartridge *cartridge);

#endif /* TINYNES_CARTRIDGE_CARTRIDGE_H */
