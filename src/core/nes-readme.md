# nes.c

**Peran:** Komposisi seluruh sistem NES dan scheduler frame.

## Isi utama

- `nes_create()`: mengalokasikan `CPUBus`, `CPU`, `PPU`, `APU`, dua
  `Controller`, lalu menghubungkan callback. Region awal diberikan sebagai
  parameter `Region`.
- `nes_load_rom()` / `nes_load_rom_data()`: memuat cartridge dan memasangnya
  ke bus dan PPU. Bila region `REGION_AUTO`, region efektif diambil dari
  metadata NES 2.0 setelah ROM dimuat.
- `nes_reset()`: mereset bus, CPU, PPU, dan APU serta akumulator dot PPU.
- `nes_run_frame()`: menjalankan instruksi sampai `ppu_frame_complete()`.
- `nes_region()` dan `nes_frame_rate()`: region efektif dan frame rate untuk
  pacing frontend.
- Akses framebuffer, audio, tombol, dan komponen untuk debug/test.

## Scheduler

Setiap instruksi:

1. periksa NMI dan IRQ;
2. `cpu_step()`;
3. ambil stall OAM DMA;
4. `cpu_bus_tick()`;
5. majukan PPU 15/5 dot (NTSC) atau 16/5 dot (PAL) dan APU 1 tick per
   siklus.

Rasio PAL yang pecahan disimpan dalam akumulator integer berbasis 1/5 dot,
sehingga rasio 3,2 tetap presisi tanpa floating point di loop panas.

## Kepemilikan

`Nes` memiliki semua komponen dan membebaskannya di `nes_destroy()`.
