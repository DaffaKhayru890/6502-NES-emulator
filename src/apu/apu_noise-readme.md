# apu_noise.c

**Peran:** Kanal noise berbasis LFSR.

## Isi utama

- Tabel 16 periode NTSC dan 16 periode PAL.
- LFSR 15 bit dengan feedback bit 0 dan bit 1 (mode normal) atau bit 6
  (mode "metalik").
- Envelope dan length counter.
- `apu_noise_output()` mengembalikan 0-15.

## Register

`$400C` volume/envelope, `$400E` mode/periode, `$400F` length counter.
