# nes.h

**Peran:** API publik komposisi NES.

## Isi utama

- `Nes *nes_create(uint32_t audio_sample_rate, Region region)` dan
  `nes_destroy`.
- `nes_load_rom()` / `nes_load_rom_data()`.
- `nes_reset()`, `nes_run_frame()`.
- `nes_set_buttons()`, `nes_framebuffer()`, `nes_read_audio()`.
- `nes_region()` dan `nes_frame_rate()` untuk pacing.
- `nes_set_debug()`, `nes_debug_enabled()`.
- Akses komponen untuk debug/test: `nes_cpu`, `nes_ppu`, `nes_apu`,
  `nes_bus`, `nes_cartridge`.

## Catatan

`Nes` adalah tipe buram. Frontend hanya membutuhkan header ini untuk
menjalankan emulator.
