# cpu_flags.c

**Peran:** Helper processor status dan stack.

## Isi utama

- `cpu_flag_get()` / `cpu_flag_set()`: baca/tulis satu bit `P`.
- `cpu_set_zn()`: set flag Z dan N berdasarkan nilai 8-bit.
- `cpu_push()` / `cpu_pop()`: operasi stack dengan `SP` yang wrap.

## Catatan

Bit B hanya bermakna pada salinan `P` di stack. `cpu_pop()` tidak
memaksakan bit U; instruksi `PLP` yang menanganinya.
