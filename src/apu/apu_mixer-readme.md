# apu_mixer.c

**Peran:** Mencampur lima kanal menjadi satu sampel.

## Rumus

```
pulse = 95.88 / (8128 / (p1 + p2) + 100)
tnd   = 159.79 / (1 / (t/8227 + n/12241 + d/22638) + 100)
```

`p1` dan `p2` adalah output pulse 0-15; `t`, `n`, `d` adalah output triangle,
noise, dan DMC. Hasil dikonversi ke rentang `int16_t`.

## Catatan

Ini pendekatan nonlinear yang umum, bukan replika analog. Tidak ada filter
low-pass atau penguatan per kanal.
