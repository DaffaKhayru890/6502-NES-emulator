/*
 * cpu_2a03.c - Siklus hidup dan eksekusi CPU RP2A03.
 *
 * File ini sengaja tidak berisi semantik instruksi. Ia hanya:
 * 1. menyimpan register CPU,
 * 2. mengambil opcode,
 * 3. meminta cpu_addressing.c menghitung alamat efektif,
 * 4. memanggil fungsi instruksi dari tabel di cpu_instructions.c,
 * 5. menghitung total siklus.
 */
#include "cpu/cpu_2a03.h"

#include "cpu_internal.h"
#include "util/log.h"

#include <stdlib.h>
#include <string.h>

#define CPU_RESET_VECTOR 0xFFFCU
#define CPU_NMI_VECTOR 0xFFFAU
#define CPU_IRQ_VECTOR 0xFFFEU
#define CPU_STATUS_RESET 0x24U /* U | I */
#define CPU_STACK_RESET 0xFDU

CPU *cpu_create(CPUReadFn read, CPUWriteFn write, void *context)
{
    CPU *cpu = NULL;

    if (read == NULL || write == NULL) {
        log_error("CPU membutuhkan callback read dan write yang valid.");
        return NULL;
    }

    cpu = (CPU *)calloc(1, sizeof(*cpu));
    if (cpu == NULL) {
        log_error("Alokasi memori untuk CPU gagal.");
        return NULL;
    }

    cpu->read = read;
    cpu->write = write;
    cpu->context = context;
    cpu->sp = CPU_STACK_RESET;
    cpu->p = CPU_STATUS_RESET;
    return cpu;
}

void cpu_destroy(CPU *cpu)
{
    free(cpu);
}

void cpu_reset(CPU *cpu)
{
    if (cpu == NULL) {
        return;
    }

    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->sp = CPU_STACK_RESET;
    cpu->p = CPU_STATUS_RESET;
    cpu->pc = cpu_read16(cpu, CPU_RESET_VECTOR);
    cpu->total_cycles = 0;
    cpu->current_opcode = 0;
    cpu->current_cycles = 0;
    cpu->extra_cycles = 0;
    cpu->jammed = false;
    memset(cpu->warned_opcode, 0, sizeof(cpu->warned_opcode));

    log_debug("CPU reset: PC=$%04X SP=$%02X P=$%02X", cpu->pc, cpu->sp, cpu->p);
}

uint8_t cpu_step(CPU *cpu)
{
    const OpcodeInfo *info = NULL;
    uint16_t address = 0;
    bool page_crossed = false;
    int cycles = 0;
    uint8_t opcode = 0;

    if (cpu == NULL || cpu->read == NULL) {
        return 0;
    }

    if (cpu->jammed) {
        /* Opcode KIL menghentikan CPU sampai reset. Kembalikan 1 agar
         * scheduler tidak terjebak loop tanpa kemajuan waktu. */
        return 1;
    }

    opcode = cpu_bus_read8(cpu, cpu->pc);
    cpu->pc = (uint16_t)(cpu->pc + 1U);
    cpu->current_opcode = opcode;
    cpu->extra_cycles = 0;
    info = cpu_opcode_info(opcode);

    if (info->op == NULL) {
        if (!cpu->warned_opcode[opcode]) {
            cpu->warned_opcode[opcode] = true;
            log_warn("Opcode tidak diimplementasikan: $%02X di PC=$%04X (diperlakukan NOP)",
                     opcode, (uint16_t)(cpu->pc - 1U));
        }
        cycles = 2;
    } else {
        address = cpu_resolve_address(cpu, info->mode, &page_crossed);
        info->op(cpu, address);
        cycles = (int)info->cycles + cpu->extra_cycles;
        if (page_crossed && info->page_penalty) {
            ++cycles;
        }
    }

    if (cycles <= 0) {
        cycles = 2;
    }

    cpu->current_cycles = (uint8_t)cycles;
    cpu->total_cycles += (uint64_t)cycles;
    return (uint8_t)cycles;
}

uint8_t cpu_nmi(CPU *cpu)
{
    if (cpu == NULL) {
        return 0;
    }

    /* Push PC (high lalu low), lalu status dengan B=0 dan U=1. */
    cpu_push(cpu, (uint8_t)(cpu->pc >> 8));
    cpu_push(cpu, (uint8_t)(cpu->pc & 0x00FFU));
    cpu_push(cpu, (uint8_t)((cpu->p & (uint8_t)~CPU_FLAG_B) | CPU_FLAG_U));
    cpu_flag_set(cpu, CPU_FLAG_I, true);
    cpu->pc = cpu_read16(cpu, CPU_NMI_VECTOR);
    cpu->total_cycles += 7;
    log_debug("NMI diterima, PC baru=$%04X", cpu->pc);
    return 7;
}

uint8_t cpu_irq(CPU *cpu)
{
    if (cpu == NULL) {
        return 0;
    }

    if (cpu_flag_get(cpu, CPU_FLAG_I)) {
        return 0;
    }

    cpu_push(cpu, (uint8_t)(cpu->pc >> 8));
    cpu_push(cpu, (uint8_t)(cpu->pc & 0x00FFU));
    cpu_push(cpu, (uint8_t)((cpu->p & (uint8_t)~CPU_FLAG_B) | CPU_FLAG_U));
    cpu_flag_set(cpu, CPU_FLAG_I, true);
    cpu->pc = cpu_read16(cpu, CPU_IRQ_VECTOR);
    cpu->total_cycles += 7;
    return 7;
}
