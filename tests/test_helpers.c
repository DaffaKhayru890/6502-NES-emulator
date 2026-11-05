/*
 * test_helpers.c - Implementasi helper unit test.
 */
#include "test_helpers.h"

#include <stdlib.h>
#include <string.h>

uint8_t fake_bus_read(void *context, uint16_t address)
{
    FakeBus *bus = (FakeBus *)context;

    return (bus != NULL) ? bus->memory[address] : 0;
}

void fake_bus_write(void *context, uint16_t address, uint8_t value)
{
    FakeBus *bus = (FakeBus *)context;

    if (bus != NULL) {
        bus->memory[address] = value;
    }
}

CPU *test_cpu_create(FakeBus *bus)
{
    CPU *cpu = cpu_create(fake_bus_read, fake_bus_write, bus);

    if (cpu != NULL) {
        cpu_reset(cpu);
    }
    return cpu;
}

void test_bus_write16(FakeBus *bus, uint16_t address, uint16_t value)
{
    if (bus == NULL) {
        return;
    }

    bus->memory[address] = (uint8_t)(value & 0x00FFU);
    bus->memory[(uint16_t)(address + 1U)] = (uint8_t)(value >> 8);
}

void test_cpu_load(CPU *cpu, FakeBus *bus, uint16_t address, const uint8_t *code, size_t size)
{
    if (cpu == NULL || bus == NULL || code == NULL) {
        return;
    }

    memcpy(&bus->memory[address], code, size);
    cpu->pc = address;
}

uint8_t *test_ines_create(uint8_t prg_units, uint8_t chr_units, uint8_t flags6, uint8_t flags7,
                          bool trainer, size_t *out_size)
{
    size_t prg_size = (size_t)prg_units * 16384U;
    size_t chr_size = (size_t)chr_units * 8192U;
    size_t header_and_trainer = 16U + (trainer ? 512U : 0U);
    size_t total = header_and_trainer + prg_size + chr_size;
    uint8_t *rom = (uint8_t *)calloc(1, total);

    if (rom == NULL) {
        return NULL;
    }

    rom[0] = 0x4E;
    rom[1] = 0x45;
    rom[2] = 0x53;
    rom[3] = 0x1A;
    rom[4] = prg_units;
    rom[5] = chr_units;
    rom[6] = flags6;
    rom[7] = flags7;
    memset(rom + header_and_trainer, 0xEA, prg_size);

    if (out_size != NULL) {
        *out_size = total;
    }
    return rom;
}

void test_ines_free(uint8_t *rom)
{
    free(rom);
}

size_t test_ines_prg_offset(bool trainer)
{
    return 16U + (trainer ? 512U : 0U);
}
