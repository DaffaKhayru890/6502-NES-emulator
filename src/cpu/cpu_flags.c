/*
 * cpu_flags.c - Helper processor status dan stack.
 */
#include "cpu_internal.h"

bool cpu_flag_get(const CPU *cpu, uint8_t flag)
{
    return cpu != NULL && (cpu->p & flag) != 0U;
}

void cpu_flag_set(CPU *cpu, uint8_t flag, bool value)
{
    if (cpu == NULL) {
        return;
    }

    if (value) {
        cpu->p = (uint8_t)(cpu->p | flag);
    } else {
        cpu->p = (uint8_t)(cpu->p & (uint8_t)~flag);
    }

    /* Bit U tidak pernah benar-benar menjadi 0 pada 6502. */
    cpu->p = (uint8_t)(cpu->p | CPU_FLAG_U);
}

void cpu_set_zn(CPU *cpu, uint8_t value)
{
    if (cpu == NULL) {
        return;
    }

    cpu_flag_set(cpu, CPU_FLAG_Z, value == 0U);
    cpu_flag_set(cpu, CPU_FLAG_N, (value & 0x80U) != 0U);
}

void cpu_push(CPU *cpu, uint8_t value)
{
    if (cpu == NULL) {
        return;
    }

    cpu_bus_write8(cpu, (uint16_t)(0x0100U | cpu->sp), value);
    cpu->sp = (uint8_t)(cpu->sp - 1U);
}

uint8_t cpu_pop(CPU *cpu)
{
    if (cpu == NULL) {
        return 0;
    }

    cpu->sp = (uint8_t)(cpu->sp + 1U);
    return cpu_bus_read8(cpu, (uint16_t)(0x0100U | cpu->sp));
}
