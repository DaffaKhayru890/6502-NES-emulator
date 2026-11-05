# Arsitektur TinyNES

## Prinsip

1. **Inti emulator tidak bergantung pada SDL2.** `tinynes_core` hanya berisi
   CPU, bus, PPU, APU, cartridge, controller, dan utilitas. Frontend SDL2
   berada di library terpisah, `tinynes_frontend`.
2. **Tidak ada state perangkat keras global.** Semua objek dibuat secara
   eksplisit dan saling dihubungkan lewat pointer. Yang global hanya level log.
3. **Ketergantungan satu arah.** CPU tidak tahu PPU; CPU hanya tahu dua
   callback baca/tulis. Bus yang mengetahui PPU, APU, cartridge, dan
   controller. `Nes` mengetahui semuanya dan menjadi pemilik objek.
4. **Kegagalan eksplisit.** Operasi yang bisa gagal mengembalikan
   `ErrorCode`, bukan diam-diam mengembalikan nilai nol.

## Diagram Modul

```
                    +----------------------+
                    |        main.c        |
                    |  CLI + loop aplikasi |
                    +----------+-----------+
                               |
                  +------------+-------------+
                  |                          |
          +-------v--------+        +--------v--------+
          | tinynes_frontend|        |    tinynes_core |
          | SDL video/audio |        |                 |
          | input           |        |  +-----------+  |
          +-----------------+        |  |    Nes    |  |
                                     |  | scheduler |  |
                                     |  +-----+-----+  |
                                     |        |        |
                                     |  +-----v-----+  |
                                     |  |  CPUBus   |  |
                                     |  +--+--+--+--+  |
                                     |     |  |  |     |
                                     |  CPU PPU APU    |
                                     |     |  |  |     |
                                     |  Cartridge      |
                                     |  Controller x2  |
                                     +-----------------+
```

`Nes` membuat dan memiliki:

- `CPU` dengan callback `nes_bus_read_callback` dan
  `nes_bus_write_callback`;
- `CPUBus` yang memiliki RAM internal;
- `PPU` dengan referensi ke `Cartridge`;
- `APU` dengan callback pembaca DMC ke bus;
- `Cartridge` hasil pemuatan ROM;
- dua `Controller`.

Urutan pembuatan penting: frontend SDL2 dibuat lebih dulu agar sample rate
audio yang benar-benar dibuka dapat diteruskan ke `nes_create()`. APU
menghitung rasio sampling dari sample rate tersebut.

## Siklus Hidup

1. `config_parse()` membaca argumen.
2. `sdl_app_create()` membuka jendela, renderer, audio, dan input.
3. `nes_create(sample_rate, region)` mengalokasikan seluruh komponen.
4. `nes_load_rom()` memuat file, memvalidasi header iNES 1.0/NES 2.0,
   membuat mapper, lalu menghubungkan cartridge ke bus dan PPU. Bila region
   `auto`, timing efektif ditentukan dari metadata ROM.
5. `nes_run_frame()` menjalankan emulasi sampai satu frame PPU selesai.
6. `nes_destroy()` membebaskan objek dalam urutan terbalik.

## Scheduler

`nes_run_frame()` adalah inti waktu emulasi:

```c
while (!ppu_frame_complete(ppu)) {
    if (ppu_take_nmi(ppu)) cycles += cpu_nmi(cpu);
    if (apu_irq_pending(apu)) cycles += cpu_irq(cpu);
    cycles += cpu_step(cpu);
    cycles += cpu_bus_take_stall(bus);
    cpu_bus_tick(bus, cycles);
    for (i = 0; i < cycles; ++i) {
        /* 15/5 dot NTSC atau 16/5 dot PAL, dengan akumulator pecahan. */
        dot_accumulator += dot_numerator;
        while (dot_accumulator >= 5) {
            ppu_clock(ppu);
            dot_accumulator -= 5;
        }
        apu_clock(apu);
    }
}
```

Konsekuensinya:

- CPU berjalan per instruksi, bukan per siklus. Ini cukup untuk game NROM
  biasa, tetapi bukan emulasi cycle-accurate penuh.
- Setiap siklus CPU memajukan PPU 3,0 dot (NTSC) atau 3,2 dot (PAL) dan
  APU satu langkah.
- NMI dan IRQ diperiksa di antara instruksi, sehingga latensi interrupt
  paling besar satu instruksi.
- OAM DMA menambah stall 513/514 siklus yang ikut memajukan PPU/APU.

## Kepemilikan Memori

| Objek | Pemilik | Dibebaskan oleh |
|---|---|---|
| `Config` | `main` (stack) | otomatis |
| `SDLApp` | `main` | `sdl_app_destroy` |
| `Nes` | `main` | `nes_destroy` |
| `CPU`, `PPU`, `APU`, `CPUBus` | `Nes` | `nes_destroy` |
| `Cartridge` | `Nes` | `nes_destroy` |
| `Mapper` | `Cartridge` | `cartridge_destroy` |
| Buffer `FileData` | pemanggil | `file_data_free` |

Bus hanya menyimpan pointer ke komponen; ia tidak memilikinya. Ini mencegah
double free dan membuat pengujian dapat menghubungkan objek tiruan.

## Gaya Kode

- C17 murni, tanpa ekstensi compiler.
- Peringatan `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` aktif.
- Nama fungsi memakai awalan modul (`cpu_`, `ppu_`, `apu_`, `nes_`, ...).
- Header internal berada di `src/<modul>/*_internal.h`, bukan di `include/`.
- Komentar dan pesan pengguna memakai Bahasa Indonesia; identifier kode
  memakai Bahasa Inggris agar konsisten dengan istilah teknis.

## Alur Data per Frame

```
event SDL
   -> sdl_app_buttons()
   -> nes_set_buttons()
   -> nes_run_frame()
        -> CPU instruksi -> bus -> PPU/APU/cartridge
        -> PPU menghasilkan framebuffer indeks palette
        -> APU menghasilkan sampel mono
   -> sdl_app_present(framebuffer)
   -> sdl_app_queue_audio(samples)
   -> pacing berdasarkan SDL_GetPerformanceCounter()
```

Framebuffer berisi indeks palette 0-63, bukan RGB. Konversi ke RGB dilakukan
saat upload texture di frontend. Dengan demikian inti emulator tidak perlu
tahu format piksel SDL.
