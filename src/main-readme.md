# main.c

**Peran:** Titik masuk aplikasi TinyNES.

## Alur

1. `config_parse()` membaca opsi dan path ROM.
2. `sdl_app_create()` membuka jendela, renderer, audio, dan input.
3. `nes_create()` dibuat dengan sample rate audio yang benar-benar dibuka
   dan region dari konfigurasi.
4. `nes_load_rom()` memuat cartridge dan menentukan region efektif.
5. Loop utama: event -> tombol -> `nes_run_frame()` -> present ->
   `nes_read_audio` -> `SDL_QueueAudio` -> pacing.

## Pacing

Target frame rate diambil dari `nes_frame_rate()`: sekitar 60,0988 fps untuk
NTSC dan 50,007 fps untuk PAL. Waktu dihitung dengan
`SDL_GetPerformanceCounter()`. Tidur dilakukan untuk sisa > 1 ms, lalu
busy-wait untuk sisa < 1 ms. Bila tertinggal lebih dari 4 frame, jadwal
di-reset.

## Catatan

Bila audio tidak aktif (`--mute` atau `--no-audio`), sampel tetap dikuras
agar ring buffer APU tidak menumpuk.
