# types.h

**Peran:** Tipe dasar dan konstanta global.

## Isi utama

- include `stdbool.h`, `stddef.h`, `stdint.h`.
- versi dan judul aplikasi.
- ukuran layar 256x240.
- enum `Region`: NTSC, PAL, dan AUTO.
- frekuensi CPU NTSC/PAL, rasio PPU:CPU (3,0 dan 3,2), jumlah scanline
  per region, dan dot per scanline.
- helper inline untuk scanline pre-render, frekuensi CPU, numerator dot,
  dan frame rate per region.
- ukuran RAM, OAM, palette, CHR RAM, PRG RAM, dan batas sprite per baris.

## Catatan

Header ini sengaja tidak menarik dependensi SDL atau modul lain.
