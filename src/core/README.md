# Modul Core

Modul `core` menyatukan seluruh perangkat keras dan menyediakan titik masuk
aplikasi.

## File

| File | Peran |
|---|---|
| `nes.c` | Membuat/menghubungkan CPU, bus, PPU, APU, cartridge, controller; scheduler frame |
| `config.c` | Parsing command line dan bantuan penggunaan |
| `error.c` | Pemetaan `ErrorCode` ke pesan Bahasa Indonesia |

## Alur

`nes_create()` mengalokasikan semua komponen dan menghubungkannya:

```c
CPU  -> CPUBus (callback baca/tulis)
CPUBus -> PPU, APU, Cartridge, Controller
PPU  -> Cartridge (pattern table)
APU  -> CPUBus (pembaca DMC)
```

`nes_load_rom()` memuat cartridge lalu memasangnya ke bus dan PPU. Bila
region `auto`, region efektif diambil dari metadata NES 2.0. `nes_run_frame()`
menjalankan instruksi CPU sampai PPU menandai frame selesai, memajukan PPU
3,0 dot (NTSC) atau 3,2 dot (PAL) dan APU 1 tick per siklus CPU.

## Kepemilikan

`Nes` memiliki semua komponen. Tidak ada komponen yang boleh dibebaskan
sendiri oleh pemanggil selama `Nes` masih hidup.
