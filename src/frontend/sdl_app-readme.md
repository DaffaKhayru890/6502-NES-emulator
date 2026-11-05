# sdl_app.c

**Peran:** Inisialisasi dan koordinasi subsistem SDL2.

## Isi utama

- `sdl_app_create()`: `SDL_Init`, lalu membuat video, audio, dan input.
  Audio bersifat opsional; kegagalan audio tidak menghentikan emulator.
- `sdl_app_destroy()`: membebaskan subsistem dalam urutan aman.
- `sdl_app_process_events()`: memproses event; `false` berarti keluar.
- `sdl_app_buttons()`: meneruskan status tombol port 1/2.
- `sdl_app_present()` dan `sdl_app_queue_audio()`.
- Getter sample rate dan status audio.

## Catatan

`SDLApp` tidak mengetahui CPU/PPU; semua data berasal dari `main.c`.
