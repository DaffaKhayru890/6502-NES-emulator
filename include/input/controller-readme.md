# controller.h

**Peran:** API publik controller NES.

## Isi utama

- enum `ControllerButton` dengan urutan bit A..Right.
- struct `Controller`: `buttons`, `shift`, `strobe`.
- `controller_init`, `controller_set_buttons`, `controller_get_buttons`.
- `controller_write_strobe`, `controller_read`.

## Catatan

Pembacaan mengembalikan satu bit serial; setelah 8 bit, nilai 1 dikembalikan
seperti open bus.
