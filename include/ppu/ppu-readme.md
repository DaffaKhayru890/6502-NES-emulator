# ppu.h

**Peran:** API publik PPU RP2C02.

## Isi utama

- tipe buram `PPU`.
- `ppu_create(Cartridge *, Region)`, `ppu_destroy`, `ppu_reset`,
  `ppu_set_cartridge`.
- `ppu_set_region` dan `ppu_region` untuk timing NTSC/PAL.
- `ppu_read_register` / `ppu_write_register`.
- `ppu_clock`, `ppu_frame_complete`, `ppu_clear_frame_complete`.
- `ppu_take_nmi`, `ppu_oam_dma`.
- `ppu_framebuffer` (indeks palette), `ppu_palette_rgb`.
- getter scanline, dot, dan frame count.

## Catatan

NTSC memakai tiga `ppu_clock()` per siklus CPU; PAL memakai rasio 3,2 yang
dikelola scheduler `Nes`.
