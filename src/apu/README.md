# Modul APU

Implementasi Audio Processing Unit RP2A03 dengan lima kanal.

## File

| File | Peran |
|---|---|
| `apu.c` | Siklus hidup, register global, sampling, ring buffer |
| `apu_pulse.c` | Dua kanal pulse: duty, envelope, sweep, length |
| `apu_triangle.c` | Kanal triangle 32 langkah dan linear counter |
| `apu_noise.c` | LFSR 15 bit, mode 6 bit, envelope, length |
| `apu_dmc.c` | Sampel delta 1-bit, timer, loop, IRQ |
| `apu_frame_counter.c` | Sequencer 4-step/5-step dan frame IRQ |
| `apu_mixer.c` | Pencampuran nonlinear pulse + TND |
| `apu_internal.h` | State kanal dan deklarasi internal |

## Waktu

`apu_clock()` dipanggil sekali per siklus CPU. Di dalamnya:

1. frame counter dimajukan;
2. timer setiap kanal dimajukan;
3. akumulator sampling ditambah `sample_rate / frekuensi_cpu_region`;
4. bila akumulator >= 1, satu sampel mono 16-bit dihasilkan.

Frekuensi CPU adalah 1.789.773 Hz (NTSC) atau 1.662.607 Hz (PAL). Frame
counter, tabel noise, dan tabel DMC juga memilih nilai sesuai region.

## Register

- `$4000-$400F`: pulse, triangle, noise.
- `$4010-$4013`: DMC.
- `$4015`: enable/status kanal.
- `$4017`: mode frame counter dan IRQ.

Membaca `$4015` mengembalikan status length/DMC/IRQ lalu menghapus flag IRQ
frame dan DMC.

## Mixer

```
pulse = 95.88 / (8128 / (p1 + p2) + 100)
tnd   = 159.79 / (1 / (t/8227 + n/12241 + d/22638) + 100)
```

Hasil dikonversi ke rentang 16-bit. Ini pendekatan umum, bukan replika
analog.

## Batasan

- DMC belum menambah stall DMA CPU.
- Output mono, tanpa filter analog.
- Detail timer pada write beruntun dapat berbeda dari hardware.
