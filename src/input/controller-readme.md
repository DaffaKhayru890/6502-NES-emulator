# controller.c

**Peran:** Shift register controller NES.

## Isi utama

- `controller_init()`: reset status dan strobe.
- `controller_set_buttons()` / `controller_get_buttons()`.
- `controller_write_strobe()`: bit 0 = 1 menahan register, 0 melepas.
- `controller_read()`: kembalikan bit 0, geser, dan isi bit tinggi dengan 1
  setelah 8 pembacaan.

## Urutan bit

A, B, Select, Start, Up, Down, Left, Right.
