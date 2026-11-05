# apu_pulse.c

**Peran:** Dua kanal pulse (square wave).

## Isi utama

- Tabel duty 4 mode x 8 langkah.
- Timer 11-bit dan sequencer duty.
- Envelope (volume konstan atau decay).
- Sweep unit dengan target period dan mute condition.
- Length counter.
- `apu_pulse_output()` menghitung amplitudo 0-15.

## Register

`$4000-$4003` untuk pulse 1 dan `$4004-$4007` untuk pulse 2. Penulisan
`$4003/$4007` me-reload timer dan length counter.

## Catatan

Perilaku detail sweep saat period di bawah 8 atau target > 0x7FF dapat
berbeda dari hardware.
