/*
 * test_cpu_addressing.c - Pengujian addressing mode dan bug JMP indirect.
 */
#include "../test.h"
#include "../test_helpers.h"

#include "cpu/cpu_2a03.h"

#include <string.h>

void test_cpu_addressing_run(void)
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

    /* Zero page wrap: LDA $FF,X dengan X=1 membaca $00. */
    bus.memory[0x0000] = 0x42;
    cpu->x = 0x01;
    code[0] = 0xB5;
    code[1] = 0xFF;
    test_cpu_load(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu_step(cpu), 4);
    CHECK_EQ_U8(cpu->a, 0x42);

    /* (Indirect,X): pointer $80 + X=$10 => alamat dari $90/$91. */
    bus.memory[0x0090] = 0x00;
    bus.memory[0x0091] = 0x20;
    bus.memory[0x2000] = 0x77;
    cpu->x = 0x10;
    code[0] = 0xA1;
    code[1] = 0x80;
    test_cpu_load(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu_step(cpu), 6);
    CHECK_EQ_U8(cpu->a, 0x77);

    /* (Indirect),Y dengan page crossing: 5 + 1 siklus. */
    bus.memory[0x0080] = 0xFF;
    bus.memory[0x0081] = 0x20;
    bus.memory[0x2100] = 0x55;
    cpu->y = 0x01;
    code[0] = 0xB1;
    code[1] = 0x80;
    test_cpu_load(cpu, &bus, 0x8000, code, 2);
    CHECK_EQ_U8(cpu_step(cpu), 6);
    CHECK_EQ_U8(cpu->a, 0x55);

    /* JMP indirect bug saat pointer berada di akhir page. */
    bus.memory[0x12FF] = 0x34;
    bus.memory[0x1200] = 0x12;
    bus.memory[0x1300] = 0x99;
    code[0] = 0x6C;
    code[1] = 0xFF;
    code[2] = 0x12;
    test_cpu_load(cpu, &bus, 0x8000, code, 3);
    CHECK_EQ_U8(cpu_step(cpu), 5);
    CHECK_EQ_U16(cpu->pc, 0x1234);

    /* Branch maju: diambil, tanpa page crossing = 3 siklus. */
    code[0] = 0xD0; /* BNE +2 */
    code[1] = 0x02;
    test_cpu_load(cpu, &bus, 0x8000, code, 2);
    cpu_flag_set(cpu, CPU_FLAG_Z, false);
    CHECK_EQ_U8(cpu_step(cpu), 3);
    CHECK_EQ_U16(cpu->pc, 0x8004);

    /* Branch diambil dan melewati page = 4 siklus. */
    code[0] = 0xD0;
    code[1] = 0x02;
    test_cpu_load(cpu, &bus, 0x80FD, code, 2);
    CHECK_EQ_U8(cpu_step(cpu), 4);
    CHECK_EQ_U16(cpu->pc, 0x8101);

    /* Branch tidak diambil = 2 siklus. */
    test_cpu_load(cpu, &bus, 0x8000, code, 2);
    cpu_flag_set(cpu, CPU_FLAG_Z, true);
    CHECK_EQ_U8(cpu_step(cpu), 2);
    CHECK_EQ_U16(cpu->pc, 0x8002);

    /* Relative mundur (offset negatif). */
    code[0] = 0xF0; /* BEQ -4 */
    code[1] = 0xFC;
    test_cpu_load(cpu, &bus, 0x8010, code, 2);
    cpu_flag_set(cpu, CPU_FLAG_Z, true);
    CHECK_EQ_U8(cpu_step(cpu), 3);
    CHECK_EQ_U16(cpu->pc, 0x800E);

    cpu_destroy(cpu);
}
