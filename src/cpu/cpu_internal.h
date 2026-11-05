/*
 * cpu_internal.h - Deklarasi internal modul CPU.
 *
 * Header ini tidak berada di include/ karena bukan API publik. Ia dipakai
 * bersama oleh cpu_2a03.c, cpu_addressing.c, cpu_instructions.c, dan
 * cpu_flags.c agar tabel opcode dapat memanggil fungsi addressing.
 */
#ifndef TINYNES_CPU_INTERNAL_H
#define TINYNES_CPU_INTERNAL_H

#include "cpu/cpu_2a03.h"

typedef enum AddressingMode {
    MODE_IMP, /* Implied */
    MODE_ACC, /* Accumulator */
    MODE_IMM,
    MODE_ZP,
    MODE_ZPX,
    MODE_ZPY,
    MODE_REL,
    MODE_ABS,
    MODE_ABX,
    MODE_ABY,
    MODE_IND,
    MODE_IZX, /* (Indirect,X) */
    MODE_IZY  /* (Indirect),Y */
} AddressingMode;

typedef void (*OpcodeFn)(CPU *cpu, uint16_t address);

typedef struct OpcodeInfo {
    const char *name;
    AddressingMode mode;
    OpcodeFn op;
    uint8_t cycles;
    bool page_penalty; /* Tambah 1 siklus bila alamat melewati page. */
} OpcodeInfo;

/* Mengambil deskripsi opcode; tidak pernah NULL (entri kosong valid). */
const OpcodeInfo *cpu_opcode_info(uint8_t opcode);

/*
 * Menyelesaikan operand dan mengembalikan alamat efektif.
 * *page_crossed diisi true bila perhitungan indeks melewati page.
 */
uint16_t cpu_resolve_address(CPU *cpu, AddressingMode mode, bool *page_crossed);

uint8_t cpu_bus_read8(CPU *cpu, uint16_t address);
void cpu_bus_write8(CPU *cpu, uint16_t address, uint8_t value);
uint16_t cpu_read16(CPU *cpu, uint16_t address);

#endif /* TINYNES_CPU_INTERNAL_H */
