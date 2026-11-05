# ppu_registers.c

**Peran:** Perilaku delapan register PPU `$2000-$2007`.

## Efek samping

- `$2000` PPUCTRL: menyimpan kontrol dan menyegarkan bit nametable `t`.
- `$2002` PPUSTATUS: mengembalikan status, menghapus VBlank, mereset latch
  alamat.
- `$2004` OAMDATA: baca/tulis OAM dan menaikkan OAMADDR pada tulis.
- `$2005` PPUSCROLL: dua tulisan membentuk X lalu Y.
- `$2006` PPUADDR: dua tulisan membentuk alamat 14-bit.
- `$2007` PPUDATA: baca memakai buffer satu byte; tulis langsung dan
  menaikkan alamat sesuai bit increment.

## Catatan

Alamat yang diterima fungsi ini adalah 0..7 (sudah dimask bus).
