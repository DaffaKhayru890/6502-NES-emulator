# Header APU

`apu.h` mendeklarasikan tipe buram `APU`, callback pembaca DMC, siklus hidup,
`apu_clock`, akses register, status IRQ, pengambilan sampel, dan sample rate.

Sampel yang dikembalikan `apu_read_samples()` adalah mono `int16_t`. Ring
buffer internal berukuran tetap dan tidak dialokasikan dinamis.
