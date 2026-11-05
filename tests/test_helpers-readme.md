# test_helpers.c / test_helpers.h

**Peran:** Utilitas bersama unit test.

## Isi utama

- struct `FakeBus` berisi `memory[65536]`.
- `fake_bus_read()` / `fake_bus_write()`: bus datar untuk CPU.
- `test_cpu_create()`: membuat CPU dengan `FakeBus`.
- `test_cpu_load()`: menyalin kode mesin ke alamat tertentu.
- `test_bus_write16()`: menulis little-endian 16-bit.
- `test_ines_create()`: membuat ROM iNES 1.0 sintetis di memori.
- `test_ines_free()` dan `test_ines_prg_offset()`.

Kasus NES 2.0 dibuat dengan memodifikasi byte header hasil
`test_ines_create()`, misalnya byte 7 menjadi `0x08`, byte 8 untuk mapper
tinggi/submapper, dan byte 9 untuk ukuran tinggi.

## Catatan

Header mendeklarasikan API; file `.c` mengimplementasikannya. ROM sintetis
mengisi PRG dengan `0xEA` (NOP) dan CHR dengan 0. Tidak ada file ROM yang
dibaca dari disk.
