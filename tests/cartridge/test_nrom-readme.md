# test_nrom.c

**Peran:** Menguji loader iNES dan mapper NROM.

## Cakupan

- Magic valid dan tidak valid.
- Ukuran file terpotong.
- PRG 16 KiB dicerminkan di `$8000` dan `$C000`.
- PRG 32 KiB.
- CHR ROM dan CHR RAM.
- Trainer dilewati.
- Mirroring horizontal/vertical.
- Penerimaan NES 2.0: submapper, CHR RAM, dan encoding ukuran eksponen.
- Region metadata: iNES 1.0 = NTSC, timing PAL diterima, multi-region
  menjadi NTSC.
- Penolakan mapper selain 0, termasuk mapper 12-bit dari byte 8.
- Penolakan console type NES 2.0 selain NES/Famicom.
- Penolakan timing Dendy dan encoding ukuran yang overflow.
- PRG RAM.
