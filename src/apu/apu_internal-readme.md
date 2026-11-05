# apu_internal.h

**Peran:** State dan deklarasi internal APU.

## Isi utama

- `APUEnvelope`: start, loop, constant, volume, divider, decay.
- Struct kanal: `APUPulse`, `APUTriangle`, `APUNoise`, `APUDMC`; noise dan
  DMC menyimpan region untuk memilih tabel periode.
- Struct `APU`: kelima kanal, region, frame counter, akumulator sampling,
  ring buffer 8192 sampel, status enable, dan IRQ.
- Deklarasi fungsi internal kanal, mixer, frame counter, dan tabel length.

## Catatan

Header ini tidak dipublikasikan; hanya dipakai file `apu_*.c`.
