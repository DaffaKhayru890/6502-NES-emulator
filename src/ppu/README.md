# Modul PPU

Implementasi Picture Processing Unit RP2C02.

## File

| File | Peran |
|---|---|
| `ppu.c` | Siklus hidup, reset, timing scanline/dot, NMI, framebuffer |
| `ppu_registers.c` | Delapan register `$2000-$2007` dan efek sampingnya |
| `ppu_memory.c` | Pattern, nametable, attribute, palette, OAM |
| `ppu_background.c` | Pipeline fetch background per 8 dot |
| `ppu_sprites.c` | Evaluasi OAM dan compositing sprite |
| `ppu_render.c` | Penggabungan background + sprite + grayscale |
| `ppu_palette.c` | Palette RAM dan tabel RGB NTSC |
| `ppu_internal.h` | State PPU dan deklarasi antar-file |

## Model Waktu

- NTSC: 262 scanline x 341 dot; PAL: 312 scanline x 341 dot.
- Scanline 0-239 visible, 240 post-render, VBlank mulai 241, pre-render di
  261 (NTSC) atau 311 (PAL).
- `ppu_clock()` memproses satu dot lalu maju.
- Scheduler memajukan 3,0 dot per siklus CPU (NTSC) atau 3,2 (PAL).
- Frame ganjil melewati satu dot pre-render.

## Rendering

Background:

- Fetch nametable, attribute, dan dua plane pattern setiap 8 dot.
- Shift register 16 bit digeser setiap dot.
- Fine X memilih bit keluaran.
- Clipping 8 piksel kiri mengikuti `$2001` bit 1.

Sprite:

- Maksimal 8 sprite per scanline.
- Ukuran 8x8 atau 8x16.
- Flip horizontal/vertikal dan prioritas di belakang background.
- Sprite 0 hit dan flag overflow.

Warna akhir adalah indeks palette 0-63. Framebuffer menyimpan indeks, bukan
RGB; konversi dilakukan frontend.

## Akurasi

Evaluasi sprite dilakukan di dot 0, bukan bertahap. Bug overflow hardware
tidak ditiru. Tidak ada konflik bus VRAM. Lihat `docs/batasan.md`.
