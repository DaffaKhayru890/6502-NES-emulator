# cpu_addressing.c

**Peran:** Fetch operand dan resolusi 13 addressing mode 6502.

## Isi utama

- `cpu_bus_read8()` / `cpu_bus_write8()`: pembungkus callback bus.
- `fetch_byte()` / `fetch_word()`: mengambil operand dan menaikkan `PC`.
- `cpu_resolve_address()`: menghitung alamat efektif untuk immediate,
  zero page, zero page,X/Y, absolute, absolute,X/Y, indirect, indexed
  indirect, dan indirect indexed.

## Detail penting

- Zero page selalu wrap dalam 256 byte.
- Page crossing dilaporkan lewat `page_crossed` agar instruksi baca dapat
  menambah satu siklus.
- `JMP ($xxFF)` meniru bug asli: byte high diambil dari `$xx00`.
