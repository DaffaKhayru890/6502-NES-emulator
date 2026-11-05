/*
 * mapper.h - Antarmuka polimorfik untuk cartridge mapper.
 *
 * Mapper adalah sirkuit di cartridge yang memetakan alamat CPU/PPU ke isi
 * ROM/RAM. Setiap mapper memiliki aturan berbeda, tetapi semuanya berbicara
 * melalui MapperOps yang sama. Inilah bentuk polimorfisme di C: struct berisi
 * pointer fungsi, dan context menunjuk state milik implementasi.
 */
#ifndef TINYNES_CARTRIDGE_MAPPER_H
#define TINYNES_CARTRIDGE_MAPPER_H

#include "util/types.h"

typedef struct MapperOps {
    uint8_t (*cpu_read)(void *context, uint16_t address);
    void (*cpu_write)(void *context, uint16_t address, uint8_t value);
    uint8_t (*ppu_read)(void *context, uint16_t address);
    void (*ppu_write)(void *context, uint16_t address, uint8_t value);
    void (*destroy)(void *context); /* Boleh NULL bila context tidak dialokasikan. */
} MapperOps;

typedef struct Mapper {
    const MapperOps *ops;
    void *context;
} Mapper;

/* Membungkus ops dan context. Mengembalikan NULL bila alokasi gagal. */
Mapper *mapper_create(const MapperOps *ops, void *context);
void mapper_destroy(Mapper *mapper);

uint8_t mapper_cpu_read(Mapper *mapper, uint16_t address);
void mapper_cpu_write(Mapper *mapper, uint16_t address, uint8_t value);
uint8_t mapper_ppu_read(Mapper *mapper, uint16_t address);
void mapper_ppu_write(Mapper *mapper, uint16_t address, uint8_t value);

#endif /* TINYNES_CARTRIDGE_MAPPER_H */
