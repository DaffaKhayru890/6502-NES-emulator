# ppu_palette.c

**Peran:** Palette RAM dan tabel warna NTSC.

## Isi utama

- Tabel 64 warna RGB khas 2C02 NTSC.
- `ppu_palette_rgb()`: indeks 0-63 -> `0x00RRGGBB`.
- `ppu_palette_read()` / `ppu_palette_write()`: akses palette RAM dengan
  mirroring alamat `$3F10/$3F14/$3F18/$3F1C`.

## Catatan

PPU menyimpan indeks palette di framebuffer; konversi RGB hanya dilakukan
frontend saat present.
