# ppu_internal.h

**Peran:** State lengkap PPU dan deklarasi antar-file.

## Isi utama

- struct `PPUSpriteSlot` dan array sprite per scanline.
- struct `PPU`: register loopy `v`/`t`/`fine_x`, latch, shift register,
  VRAM, palette, OAM, framebuffer, region, status, dan state timing.
- deklarasi fungsi background, sprite, render, memory, dan palette.

## Catatan

Header tidak dipublikasikan; hanya dipakai file `ppu_*.c`.
