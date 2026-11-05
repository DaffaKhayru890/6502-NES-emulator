# cpu_bus.c

**Peran:** Implementasi peta memori CPU NES.

## Isi utama

- `cpu_bus_create()` / `cpu_bus_destroy()`.
- Fungsi koneksi PPU, APU, cartridge, dan dua controller.
- `cpu_bus_read()` / `cpu_bus_write()`: decode alamat dan mirroring.
- `cpu_bus_reset()`: reset RAM dan state bus.
- OAM DMA `$4014` dengan stall 513/514 siklus.
- `cpu_bus_take_stall()`, `cpu_bus_tick()`, `cpu_bus_cycle_count()`.

## Peta memori

RAM 2 KiB + mirror, register PPU + mirror, register APU, controller,
PRG RAM, dan PRG ROM. Alamat tanpa perangkat mengembalikan open bus.

## Catatan

Bus tidak memiliki komponen yang dihubungkan; pointer boleh `NULL`.
