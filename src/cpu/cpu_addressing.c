/*
 * cpu_addressing.c - Resolusi addressing mode 6502.
 *
 * Konsep penting:
 * - Zero page adalah 256 byte pertama ($0000-$00FF). Alamat zero page
 *   selalu dibungkus (wrap) sehingga $FF + 1 menjadi $00.
 * - Page crossing terjadi bila alamat efektif berada di page berbeda dari
 *   alamat basis. Instruksi baca tertentu mendapat penalti 1 siklus.
 * - JMP ($xxFF) memiliki bug asli 6502: byte high diambil dari $xx00,
 *   bukan $xx+1:00.
 */
#include "cpu_internal.h"

uint8_t cpu_bus_read8(CPU *cpu, uint16_t address)
{
    if (cpu == NULL || cpu->read == NULL) {
        return 0;
    }
    return cpu->read(cpu->context, address);
}

void cpu_bus_write8(CPU *cpu, uint16_t address, uint8_t value)
{
    if (cpu == NULL || cpu->write == NULL) {
        return;
    }
    cpu->write(cpu->context, address, value);
}

uint16_t cpu_read16(CPU *cpu, uint16_t address)
{
    uint8_t low = cpu_bus_read8(cpu, address);
    uint8_t high = cpu_bus_read8(cpu, (uint16_t)(address + 1U));
    return (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
}

static uint8_t fetch_byte(CPU *cpu)
{
    uint8_t value = cpu_bus_read8(cpu, cpu->pc);
    cpu->pc = (uint16_t)(cpu->pc + 1U);
    return value;
}

static uint16_t fetch_word(CPU *cpu)
{
    uint8_t low = fetch_byte(cpu);
    uint8_t high = fetch_byte(cpu);
    return (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
}

uint16_t cpu_resolve_address(CPU *cpu, AddressingMode mode, bool *page_crossed)
{
    uint16_t base = 0;
    uint16_t address = 0;
    uint8_t pointer = 0;

    if (page_crossed != NULL) {
        *page_crossed = false;
    }

    if (cpu == NULL) {
        return 0;
    }

    switch (mode) {
    case MODE_IMP:
    case MODE_ACC:
        return 0;

    case MODE_IMM:
        address = cpu->pc;
        cpu->pc = (uint16_t)(cpu->pc + 1U);
        return address;

    case MODE_ZP:
        return fetch_byte(cpu);

    case MODE_ZPX:
        return (uint16_t)((fetch_byte(cpu) + cpu->x) & 0xFFU);

    case MODE_ZPY:
        return (uint16_t)((fetch_byte(cpu) + cpu->y) & 0xFFU);

    case MODE_REL: {
        int8_t offset = (int8_t)fetch_byte(cpu);
        address = (uint16_t)(cpu->pc + (int16_t)offset);
        return address;
    }

    case MODE_ABS:
        return fetch_word(cpu);

    case MODE_ABX:
        base = fetch_word(cpu);
        address = (uint16_t)(base + cpu->x);
        if (page_crossed != NULL && (base & 0xFF00U) != (address & 0xFF00U)) {
            *page_crossed = true;
        }
        return address;

    case MODE_ABY:
        base = fetch_word(cpu);
        address = (uint16_t)(base + cpu->y);
        if (page_crossed != NULL && (base & 0xFF00U) != (address & 0xFF00U)) {
            *page_crossed = true;
        }
        return address;

    case MODE_IND:
        base = fetch_word(cpu);
        if ((base & 0x00FFU) == 0x00FFU) {
            /* Bug hardware 6502: high byte diambil dari page yang sama. */
            uint8_t low = cpu_bus_read8(cpu, base);
            uint8_t high = cpu_bus_read8(cpu, (uint16_t)(base & 0xFF00U));
            return (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
        }
        return cpu_read16(cpu, base);

    case MODE_IZX:
        pointer = fetch_byte(cpu);
        address = cpu_bus_read8(cpu, (uint16_t)((pointer + cpu->x) & 0xFFU));
        base = cpu_bus_read8(cpu, (uint16_t)((pointer + cpu->x + 1U) & 0xFFU));
        return (uint16_t)((uint16_t)address | ((uint16_t)base << 8));

    case MODE_IZY:
        pointer = fetch_byte(cpu);
        base = (uint16_t)((uint16_t)cpu_bus_read8(cpu, pointer) |
                          ((uint16_t)cpu_bus_read8(cpu, (uint16_t)((pointer + 1U) & 0xFFU)) << 8));
        address = (uint16_t)(base + cpu->y);
        if (page_crossed != NULL && (base & 0xFF00U) != (address & 0xFF00U)) {
            *page_crossed = true;
        }
        return address;

    default:
        return 0;
    }
}
