# apu.c

**Peran:** Siklus hidup, register global, dan sampling APU.

## Isi utama

- Tabel length counter 32 entri.
- `apu_create(sample_rate, Region)` / `apu_destroy()` / `apu_reset()`.
- `apu_set_region()` / `apu_region()` untuk mengganti timing NTSC/PAL.
- `apu_clock()`: frame counter, timer semua kanal, lalu sampling.
- `apu_read_status()`: status kanal dan IRQ; membaca menghapus flag IRQ.
- `apu_write_register()`: decode `$4000-$4017`.
- `apu_read_samples()`: ambil sampel dari ring buffer.
- `apu_irq_pending()` dan `apu_frame_irq()`.

## Sampling

Akumulator pecahan `sample_rate / frekuensi_cpu_region` menghasilkan satu
sampel mono 16-bit setiap kali mencapai 1.0. Frekuensi CPU adalah 1.789.773
Hz untuk NTSC dan 1.662.607 Hz untuk PAL. Ring buffer berukuran 8192; bila
penuh, sampel terlama dibuang.
