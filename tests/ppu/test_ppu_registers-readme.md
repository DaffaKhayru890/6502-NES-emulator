# test_ppu_registers.c

**Peran:** Menguji register dan timing PPU.

## Cakupan

- Alamat VRAM lewat `$2006`, increment 1 dan 32.
- Buffer baca `$2007`.
- OAMADDR/OAMDATA.
- VBlank setelah 82524 dot dari reset NTSC (scanline 241 dot 2).
- NMI enable dan konsumsi edge.
- Pembacaan `$2002` menghapus VBlank dan mereset latch.
- Tidak ada NMI bila bit enable mati.
- PAL: pre-render di scanline 311, VBlank tetap di 241, dan frame selesai
  setelah area visible 240 baris.

## Catatan

Angka 82524 berasal dari 341 dot scanline pre-render (261 NTSC atau 311
PAL) + 241 scanline x 341 dot + 2 dot scanline 241. Untuk PAL, jumlah ini
sama karena area visible juga 240 scanline.
