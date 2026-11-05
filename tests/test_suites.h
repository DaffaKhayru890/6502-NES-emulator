/*
 * test_suites.h - Deklarasi seluruh suite pengujian.
 */
#ifndef TINYNES_TESTS_TEST_SUITES_H
#define TINYNES_TESTS_TEST_SUITES_H

void test_cpu_registers_run(void);
void test_cpu_addressing_run(void);
void test_cpu_instructions_run(void);
void test_cpu_bus_run(void);
void test_ppu_memory_run(void);
void test_ppu_registers_run(void);
void test_cartridge_nrom_run(void);
void test_apu_run(void);
void test_nes_integration_run(void);

#endif /* TINYNES_TESTS_TEST_SUITES_H */
