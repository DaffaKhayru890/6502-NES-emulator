# ppu_sprites.c

**Peran:** Evaluasi OAM dan compositing sprite.

## Isi utama

- `ppu_sprites_evaluate()`: pilih maksimal 8 sprite untuk scanline aktif,
  ambil pattern, dan simpan ke `sprite_line`.
- `ppu_sprite_pixel()`: hitung piksel sprite pada posisi x, termasuk flip,
  palette, prioritas, dan sprite 0 hit.

## Detail

- Ukuran 8x8 atau 8x16.
- Sprite 8x16: bit 0 tile memilih pattern table; tile dipaksa genap.
- Flip vertikal dihitung saat evaluasi, flip horizontal saat compositing.
- Sprite ke-9 yang cocok men-set flag overflow.

## Catatan

Evaluasi dilakukan di dot 0, bukan siklus 65-256 seperti hardware.
