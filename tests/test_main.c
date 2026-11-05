/*
 * test_main.c - Titik masuk seluruh unit test TinyNES.
 */
#include "test.h"
#include "test_suites.h"

#include "util/log.h"

int g_test_checks = 0;
int g_test_failures = 0;
int g_test_suites = 0;

int main(void)
{
    log_set_level(LOG_LEVEL_ERROR);

    RUN_SUITE(test_cpu_registers_run);
    RUN_SUITE(test_cpu_addressing_run);
    RUN_SUITE(test_cpu_instructions_run);
    RUN_SUITE(test_cpu_bus_run);
    RUN_SUITE(test_ppu_memory_run);
    RUN_SUITE(test_ppu_registers_run);
    RUN_SUITE(test_cartridge_nrom_run);
    RUN_SUITE(test_apu_run);
    RUN_SUITE(test_nes_integration_run);

    printf("\nRingkasan: %d suite, %d pemeriksaan, %d kegagalan.\n", g_test_suites,
           g_test_checks, g_test_failures);

    return (g_test_failures == 0) ? 0 : 1;
}
