# apu_dmc.c

**Peran:** Kanal DMC (Delta Modulation Channel).

## Isi utama

- Tabel 16 periode NTSC dan 16 periode PAL.
- Buffer sampel 1-bit yang diisi lewat callback bus CPU.
- `dmc_fill_buffer()` mengambil byte berikutnya dari alamat sampel.
- `apu_dmc_timer_clock()` menggeser bit dan menaik-turunkan level 7 bit.
- Loop, IRQ, dan enable/disable lewat `$4015` bit 4.
- `apu_dmc_write()` menangani `$4010-$4013`.

## Catatan

Stall DMA CPU belum ditambahkan. Panjang sampel dan loop mengikuti register,
tetapi detail timing pengambilan byte dapat berbeda dari hardware.
