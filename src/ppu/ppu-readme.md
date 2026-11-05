# ppu.c

**Peran:** Siklus hidup, timing, dan NMI PPU.

## Isi utama

- `ppu_create(Cartridge *, Region)` / `ppu_destroy()` / `ppu_reset()`.
- `ppu_set_region()` / `ppu_region()` untuk mengganti timing NTSC/PAL.
- `ppu_clock()`: memproses satu dot, lalu maju ke dot/scanline berikutnya.
- VBlank di-set pada scanline 241 dot 1; NMI dicatat bila diaktifkan.
- Pembersihan flag VBlank/sprite 0/overflow pada dot 1 scanline pre-render
  (261 NTSC, 311 PAL).
- `ppu_take_nmi()`: konsumsi edge NMI sekali.
- `ppu_oam_dma()`, akses framebuffer, dan getter scanline/dot/frame count.
- `ppu_increment_x/y()` dan `ppu_copy_x/y()` untuk loopy register.

## Model

NTSC memakai 262 scanline, PAL 312 scanline; keduanya 341 dot. Area visible
tetap 0-239 dan VBlank dimulai di 241. Frame ganjil melewati satu dot
pre-render. Frame dianggap selesai pada scanline 239 dot 340.

## Catatan

PPU memajukan pipeline background dan sprite berdasarkan `$2001`. Detail
akurasi dijelaskan di `docs/batasan.md`.
