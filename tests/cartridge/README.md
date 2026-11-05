# Test Cartridge

`test_nrom.c` menguji loader iNES dan mapper 0:

- validasi magic dan ukuran;
- PRG 16 KiB dicerminkan;
- PRG 32 KiB;
- CHR ROM dan CHR RAM;
- trainer;
- penerimaan NES 2.0: mapper 12-bit, submapper, CHR RAM, dan encoding
  ukuran eksponen;
- region dari timing NES 2.0: PAL diterima, multi-region menjadi NTSC;
- penolakan mapper lain, console type NES 2.0 selain NES/Famicom, timing
  Dendy, dan file terpotong.
