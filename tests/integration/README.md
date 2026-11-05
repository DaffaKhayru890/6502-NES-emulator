# Test Integrasi

`test_nes_integration.c` membuat objek `Nes` lengkap, memuat ROM iNES/NES 2.0
sintetis 16 KiB PRG + CHR RAM, lalu menjalankan beberapa frame.

Yang diperiksa:

- pembuatan dan pemuatan berhasil;
- eksekusi frame tidak crash;
- framebuffer dapat diakses;
- jalur NMI/PPU berjalan;
- deteksi region PAL dari NES 2.0 dan override region eksplisit.
