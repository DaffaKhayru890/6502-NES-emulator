# test_suites.h

**Peran:** Deklarasi seluruh fungsi suite.

## Isi utama

Sembilan fungsi `void test_*_run(void)`:

- `test_cpu_registers_run`
- `test_cpu_addressing_run`
- `test_cpu_instructions_run`
- `test_cpu_bus_run`
- `test_ppu_memory_run`
- `test_ppu_registers_run`
- `test_cartridge_nrom_run`
- `test_apu_run`
- `test_nes_integration_run`

## Catatan

Saat menambah suite, tambahkan deklarasi di sini, implementasi di file baru,
dan pemanggilan di `test_main.c`.
