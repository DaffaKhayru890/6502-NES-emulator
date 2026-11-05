/*
 * test_helpers.h - Utilitas bersama untuk unit test.
 */
#ifndef TINYNES_TESTS_TEST_HELPERS_H
#define TINYNES_TESTS_TEST_HELPERS_H

#include "cpu/cpu_2a03.h"
#include "util/types.h"

typedef struct FakeBus {
    uint8_t memory[65536];
} FakeBus;

uint8_t fake_bus_read(void *context, uint16_t address);
void fake_bus_write(void *context, uint16_t address, uint8_t value);

CPU *test_cpu_create(FakeBus *bus);
void test_cpu_load(CPU *cpu, FakeBus *bus, uint16_t address, const uint8_t *code, size_t size);
void test_bus_write16(FakeBus *bus, uint16_t address, uint16_t value);

/*
 * Membuat cartridge iNES sintetis.
 * prg_units dan chr_units dihitung dalam unit masing-masing (16 KiB / 8 KiB).
 * Isi PRG diisi 0xEA (NOP), CHR diisi 0.
 */
uint8_t *test_ines_create(uint8_t prg_units, uint8_t chr_units, uint8_t flags6, uint8_t flags7,
                          bool trainer, size_t *out_size);
void test_ines_free(uint8_t *rom);
size_t test_ines_prg_offset(bool trainer);

#endif /* TINYNES_TESTS_TEST_HELPERS_H */
