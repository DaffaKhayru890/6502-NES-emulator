# test_nes_integration.c

**Peran:** Menguji jalur lengkap `Nes`.

## Alur

1. Buat `Nes` dengan sample rate uji dan region.
2. Buat ROM iNES 1.0 sintetis 16 KiB PRG + CHR RAM.
3. Muat lewat `nes_load_rom_data()`.
4. Jalankan beberapa frame.
5. Periksa framebuffer dan state dasar.
6. Ulangi dengan ROM NES 2.0 bertiming PAL: region otomatis harus menjadi
   PAL, PPU/APU ikut PAL, dan frame rate sekitar 50 Hz.
7. Uji override eksplisit NTSC pada ROM PAL.

## Catatan

ROM diisi NOP sehingga CPU akan terus mengeksekusi `$EA`; ini cukup untuk
menguji scheduler, PPU clock, APU clock, dan akses framebuffer tanpa
ketergantungan pada program game nyata.
