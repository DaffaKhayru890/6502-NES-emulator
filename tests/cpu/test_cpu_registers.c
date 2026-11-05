/*
 * test_cpu_registers.c - Register, flag, stack, dan interrupt CPU.
 */
#include "../test.h"
#include "../test_helpers.h"

#include "cpu/cpu_2a03.h"

#include <string.h>

static uint8_t execute(CPU *cpu, FakeBus *bus, uint16_t address, const uint8_t *code,
                       size_t size)
{
    test_cpu_load(cpu, bus, address, code, size);
    return cpu_step(cpu);
}

void test_cpu_registers_run(void)
{
    FakeBus bus;
    CPU *cpu = NULL;
    uint8_t code[8];

    memset(&bus, 0, sizeof(bus));
    test_bus_write16(&bus, 0xFFFC, 0x8000);
    cpu = test_cpu_create(&bus);
    CHECK_TRUE(cpu != NULL);
    if (cpu == NULL) {
        return;
    }

    cpu_reset(cpu);
    CHECK_EQ_U16(cpu->pc, 0x8000);
    CHECK_EQ_U8(cpu->sp, 0xFD);
    CHECK_EQ_U8(cpu->p & (CPU_FLAG_I | CPU_FLAG_U), CPU_FLAG_I | CPU_FLAG_U);
    CHECK_EQ_U8(cpu->p & (CPU_FLAG_C | CPU_FLAG_Z | CPU_FLAG_N | CPU_FLAG_V | CPU_FLAG_D), 0);

    cpu_flag_set(cpu, CPU_FLAG_C, true);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_C));
    cpu_flag_set(cpu, CPU_FLAG_C, false);
    CHECK_FALSE(cpu_flag_get(cpu, CPU_FLAG_C));
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_U)); /* U selalu aktif */

    cpu_set_zn(cpu, 0x80);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_N));
    CHECK_FALSE(cpu_flag_get(cpu, CPU_FLAG_Z));
    cpu_set_zn(cpu, 0x00);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_Z));
    CHECK_FALSE(cpu_flag_get(cpu, CPU_FLAG_N));

    cpu->sp = 0xFF;
    cpu_push(cpu, 0x12);
    CHECK_EQ_U8(bus.memory[0x01FF], 0x12);
    CHECK_EQ_U8(cpu->sp, 0xFE);
    CHECK_EQ_U8(cpu_pop(cpu), 0x12);
    CHECK_EQ_U8(cpu->sp, 0xFF);

    /* LDA #$80 harus menyalakan N; LDA #$00 menyalakan Z. */
    code[0] = 0xA9;
    code[1] = 0x80;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu->a, 0x80);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_N));
    code[1] = 0x00;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_Z));

    /* Transfers. */
    cpu->a = 0x42;
    code[0] = 0xAA; /* TAX */
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(cpu->x, 0x42);
    cpu->x = 0x37;
    code[0] = 0x8A; /* TXA */
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(cpu->a, 0x37);

    /* ADC mode biner: 0x09 + 0x01 harus 0x0A walau flag D aktif. */
    cpu->a = 0x09;
    cpu_flag_set(cpu, CPU_FLAG_D, true);
    cpu_flag_set(cpu, CPU_FLAG_C, false);
    code[0] = 0x69;
    code[1] = 0x01;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu->a, 0x0A);
    CHECK_FALSE(cpu_flag_get(cpu, CPU_FLAG_C));
    cpu_flag_set(cpu, CPU_FLAG_D, false);

    /* SBC: 0x50 - 0x10 = 0x40 dengan carry tetap 1. */
    cpu->a = 0x50;
    cpu_flag_set(cpu, CPU_FLAG_C, true);
    code[0] = 0xE9;
    code[1] = 0x10;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu->a, 0x40);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_C));

    /* JSR lalu RTS. */
    code[0] = 0x20; /* JSR $8100 */
    code[1] = 0x00;
    code[2] = 0x81;
    cpu->sp = 0xFD;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U16(cpu->pc, 0x8100);
    CHECK_EQ_U8(cpu->sp, 0xFB);
    code[0] = 0x60; /* RTS */
    execute(cpu, &bus, 0x8100, code, 1);
    CHECK_EQ_U16(cpu->pc, 0x8003);
    CHECK_EQ_U8(cpu->sp, 0xFD);

    /* BRK dan RTI. */
    test_bus_write16(&bus, 0xFFFE, 0x9000);
    cpu->sp = 0xFD;
    code[0] = 0x00; /* BRK */
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U16(cpu->pc, 0x9000);
    CHECK_EQ_U8(cpu->sp, 0xFA);
    code[0] = 0x40; /* RTI */
    execute(cpu, &bus, 0x9000, code, 1);
    CHECK_EQ_U16(cpu->pc, 0x8002);
    CHECK_EQ_U8(cpu->sp, 0xFD);

    /* NMI: vektor $FFFA, 7 siklus, flag I aktif. */
    test_bus_write16(&bus, 0xFFFA, 0xA000);
    cpu->pc = 0x8123;
    cpu->sp = 0xFD;
    cpu_flag_set(cpu, CPU_FLAG_I, false);
    CHECK_EQ_U8(cpu_nmi(cpu), 7);
    CHECK_EQ_U16(cpu->pc, 0xA000);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_I));

    /* IRQ diabaikan bila I = 1. */
    cpu->pc = 0x8123;
    CHECK_EQ_U8(cpu_irq(cpu), 0);
    cpu_flag_set(cpu, CPU_FLAG_I, false);
    CHECK_EQ_U8(cpu_irq(cpu), 7);
    CHECK_EQ_U16(cpu->pc, 0x9000);

    /* Penalti siklus: LDA absolute,X melewati page = 5 siklus. */
    code[0] = 0xBD; /* LDA $20FF,X */
    code[1] = 0xFF;
    code[2] = 0x20;
    cpu->x = 0x01;
    CHECK_EQ_U8(execute(cpu, &bus, 0x8000, code, 3), 5);
    cpu->x = 0x00;
    CHECK_EQ_U8(execute(cpu, &bus, 0x8000, code, 3), 4);

    cpu_destroy(cpu);
}
