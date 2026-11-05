# cpu_bus.h

**Peran:** API publik bus memori CPU.

## Isi utama

- tipe buram `CPUBus`.
- fungsi koneksi PPU, APU, cartridge, dan controller.
- `cpu_bus_read/write`, `cpu_bus_reset`.
- `cpu_bus_take_stall`, `cpu_bus_tick`, `cpu_bus_cycle_count`.

## Catatan

Bus tidak memiliki komponen yang dihubungkan. Semua pointer boleh `NULL`
untuk mensimulasikan perangkat tidak ada.
