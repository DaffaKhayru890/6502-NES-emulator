# ppu_render.c

**Peran:** Menggabungkan background dan sprite menjadi satu piksel.

## Prioritas

1. Sprite opaque dan background transparan -> sprite.
2. Keduanya opaque -> sprite di depan, kecuali attribute "behind".
3. Background opaque -> background.
4. Tidak ada yang opaque -> warna universal `$3F00`.

## Detail

- Clipping 8 piksel kiri mengikuti `$2001`.
- Grayscale memaksa indeks menjadi kelipatan 16.
- Hasil yang disimpan adalah indeks palette 0-63, bukan RGB.

## Catatan

Sprite 0 hit di-set di `ppu_sprites.c` saat compositing, bukan saat fetch.
