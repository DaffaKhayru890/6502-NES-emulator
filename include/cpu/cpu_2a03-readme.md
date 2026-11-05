# cpu_2a03.h

**Peran:** API publik CPU RP2A03.

## Isi utama

- callback `CPUReadFn` dan `CPUWriteFn`.
- enum `CPUFlag`.
- struct `CPU`: register, callback, `context`, dan statistik eksekusi.
- `cpu_create`, `cpu_destroy`, `cpu_reset`, `cpu_step`, `cpu_nmi`,
  `cpu_irq`.
- helper `cpu_flag_get/set`, `cpu_set_zn`, `cpu_push/pop`.

## Catatan

Struct `CPU` sengaja terbuka agar test dan debugger dapat membaca register
tanpa getter tambahan.
