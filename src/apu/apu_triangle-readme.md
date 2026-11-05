# apu_triangle.c

**Peran:** Kanal triangle.

## Isi utama

- Sequencer 32 langkah: 15 turun ke 0, lalu naik kembali ke 15.
- Timer 11-bit.
- Linear counter dengan reload flag dan control flag.
- Length counter.
- `apu_triangle_output()` mengembalikan 0-15.

## Catatan

Triangle tidak memakai envelope. Kanal akan diam bila length counter atau
linear counter bernilai 0.
