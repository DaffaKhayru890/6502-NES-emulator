/*
 * test_cpu_instructions.c - Semantik instruksi penting.
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

void test_cpu_instructions_run(void)
{
    FakeBus bus;
    CPU *cpu = NULL;
    uint8_t code[8];

    memset(&bus, 0, sizeof(bus));
    cpu = test_cpu_create(&bus);
    CHECK_TRUE(cpu != NULL);
    if (cpu == NULL) {
        return;
    }

    /* LDA/STA absolute. */
    code[0] = 0xA9;
    code[1] = 0x99;
    execute(cpu, &bus, 0x8000, code, 2);
    code[0] = 0x8D;
    code[1] = 0x00;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0300], 0x99);

    /* LDX/LDY/STX/STY. */
    code[0] = 0xA2;
    code[1] = 0x11;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu->x, 0x11);
    code[0] = 0xA0;
    code[1] = 0x22;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu->y, 0x22);
    code[0] = 0x8E;
    code[1] = 0x10;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0310], 0x11);
    code[0] = 0x8C;
    code[1] = 0x11;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0311], 0x22);

    /* Logika: (0xF0 & 0x3C) = 0x30; | 0x03 = 0x33; ^ 0xFF = 0xCC. */
    cpu->a = 0xF0;
    code[0] = 0x29;
    code[1] = 0x3C;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu->a, 0x30);
    code[0] = 0x09;
    code[1] = 0x03;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu->a, 0x33);
    code[0] = 0x49;
    code[1] = 0xFF;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu->a, 0xCC);

    /* ASL accumulator: 0x81 -> 0x02, C=1. */
    cpu->a = 0x81;
    cpu_flag_set(cpu, CPU_FLAG_C, false);
    code[0] = 0x0A;
    CHECK_EQ_U8(execute(cpu, &bus, 0x8000, code, 1), 2);
    CHECK_EQ_U8(cpu->a, 0x02);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_C));

    /* ROR dengan carry masuk: 0x01, C=1 -> 0x80, C=1. */
    cpu->a = 0x01;
    cpu_flag_set(cpu, CPU_FLAG_C, true);
    code[0] = 0x6A;
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(cpu->a, 0x80);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_C));

    /* ROL memory. */
    bus.memory[0x0320] = 0x40;
    cpu_flag_set(cpu, CPU_FLAG_C, false);
    code[0] = 0x2E;
    code[1] = 0x20;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0320], 0x80);

    /* LSR memory. */
    bus.memory[0x0321] = 0x03;
    code[0] = 0x4E;
    code[1] = 0x21;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0321], 0x01);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_C));

    /* CMP: 0x80 vs 0x40 -> C=1, Z=0, N=0. */
    cpu->a = 0x80;
    code[0] = 0xC9;
    code[1] = 0x40;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_C));
    CHECK_FALSE(cpu_flag_get(cpu, CPU_FLAG_Z));

    /* CPX dan CPY. */
    cpu->x = 0x10;
    code[0] = 0xE0;
    code[1] = 0x10;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_Z));
    cpu->y = 0x05;
    code[0] = 0xC0;
    code[1] = 0x06;
    execute(cpu, &bus, 0x8000, code, 2);
    CHECK_FALSE(cpu_flag_get(cpu, CPU_FLAG_C));

    /* INC/DEC memory dan register. */
    bus.memory[0x0330] = 0xFE;
    code[0] = 0xEE;
    code[1] = 0x30;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0330], 0xFF);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_N));
    code[0] = 0xCE;
    code[1] = 0x30;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0330], 0xFE);
    cpu->x = 0x00;
    code[0] = 0xCA; /* DEX -> 0xFF */
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(cpu->x, 0xFF);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_N));
    code[0] = 0xE8; /* INX -> 0x00 */
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(cpu->x, 0x00);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_Z));

    /* BIT. */
    bus.memory[0x0340] = 0xC0;
    cpu->a = 0x00;
    code[0] = 0x2C;
    code[1] = 0x40;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_Z));
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_N));
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_V));

    /* PHA/PLA dan PHP/PLP. */
    cpu->sp = 0xFD;
    cpu->a = 0x5A;
    code[0] = 0x48;
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(bus.memory[0x01FD], 0x5A);
    cpu->a = 0x00;
    code[0] = 0x68;
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(cpu->a, 0x5A);

    cpu->sp = 0xFD;
    cpu->p = 0x00;
    code[0] = 0x08;
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(bus.memory[0x01FD], CPU_FLAG_B | CPU_FLAG_U);

    cpu->sp = 0xFE;
    bus.memory[0x01FF] = 0xFF;
    code[0] = 0x28; /* PLP */
    execute(cpu, &bus, 0x8000, code, 1);
    CHECK_EQ_U8(cpu->p, 0xEF); /* B dibersihkan, U dipaksa 1 */

    /* NOP resmi. */
    code[0] = 0xEA;
    CHECK_EQ_U8(execute(cpu, &bus, 0x8000, code, 1), 2);

    /* LAX (tidak resmi): A dan X memuat nilai yang sama. */
    bus.memory[0x0350] = 0x6D;
    code[0] = 0xAF;
    code[1] = 0x50;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(cpu->a, 0x6D);
    CHECK_EQ_U8(cpu->x, 0x6D);

    /* SAX: simpan A & X. */
    cpu->a = 0xF0;
    cpu->x = 0x3C;
    code[0] = 0x8F;
    code[1] = 0x51;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0351], 0x30);

    /* DCP: decrement lalu compare. */
    bus.memory[0x0352] = 0x05;
    cpu->a = 0x04;
    code[0] = 0xCF;
    code[1] = 0x52;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(bus.memory[0x0352], 0x04);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_Z));

    /* SLO: ASL lalu ORA. */
    bus.memory[0x0353] = 0x40;
    cpu->a = 0x01;
    code[0] = 0x0F;
    code[1] = 0x53;
    code[2] = 0x03;
    execute(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(cpu->a, 0x81);
    CHECK_TRUE(cpu_flag_get(cpu, CPU_FLAG_N));

    /* Siklus STA absolute,X = 5 (tanpa penalti page). */
    code[0] = 0x9D;
    code[1] = 0xFF;
    code[2] = 0x30;
    cpu->x = 0x01;
    CHECK_EQ_U8(execute(cpu, &bus, 0x8000, code, 3), 5);

    cpu_destroy(cpu);
}
