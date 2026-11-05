# sdl_input.h

**Peran:** API publik input SDL2.

## Isi utama

- tipe buram `SDLInput`.
- `sdl_input_create`, `sdl_input_destroy`.
- `sdl_input_process_event`.
- `sdl_input_buttons` untuk port 1 dan 2.

## Catatan

`union SDL_Event` di-forward-declare agar file ini tidak memaksa pemanggil
menyertakan semua header SDL.
