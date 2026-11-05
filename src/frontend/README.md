# Modul Frontend

Lapisan SDL2. Modul ini tidak mengetahui CPU, PPU, atau cartridge; ia hanya
menerima framebuffer indeks palette dan sampel audio dari `main.c`.

## File

| File | Peran |
|---|---|
| `sdl_app.c` | Menggabungkan video, audio, input; event loop |
| `sdl_video.c` | Window, renderer, texture 256x240, penskalaan integer |
| `sdl_audio.c` | Device audio mono S16 dan `SDL_QueueAudio` |
| `sdl_input.c` | Keyboard dan game controller |

## Video

- Texture internal 256x240, format `ARGB8888`.
- Framebuffer berisi indeks palette; setiap present, indeks diubah ke RGB
  memakai `ppu_palette_rgb()`.
- `SDL_RenderSetLogicalSize` dan `SDL_RenderSetIntegerScale` menjaga
  penskalaan integer dan letterbox.

## Audio

- Device dibuka pada sample rate yang diminta (biasanya 44100 atau 48000).
- Format `AUDIO_S16SYS`, mono.
- Sampel dikirim dengan `SDL_QueueAudio`, sehingga tidak perlu callback yang
  membaca state APU dari thread lain.
- `--mute` dan `--no-audio` didukung; tanpa device, emulator tetap berjalan.

## Input

- Keyboard dipetakan tetap (lihat `docs/penggunaan.md`).
- Game controller dibuka otomatis saat terhubung dan ditutup saat dilepas.
- Status tombol dikembalikan dalam format bit `ControllerButton`.
