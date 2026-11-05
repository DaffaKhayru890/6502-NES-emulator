# apu.h

**Peran:** API publik APU RP2A03.

## Isi utama

- tipe buram `APU`.
- callback `APUReadFn` untuk DMC.
- `apu_create(sample_rate, Region)`, `apu_destroy`, `apu_reset`.
- `apu_set_region`, `apu_region` untuk timing NTSC/PAL.
- `apu_set_dmc_reader`, `apu_clock`.
- `apu_read_status`, `apu_write_register`.
- `apu_irq_pending`, `apu_frame_irq`.
- `apu_read_samples`, `apu_sample_rate`.

## Catatan

Sampel adalah mono `int16_t`. Ring buffer internal berukuran tetap.
