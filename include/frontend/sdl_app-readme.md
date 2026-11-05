# sdl_app.h

**Peran:** API publik aplikasi SDL2.

## Isi utama

- tipe buram `SDLApp`.
- `sdl_app_create`, `sdl_app_destroy`.
- `sdl_app_process_events`, `sdl_app_buttons`.
- `sdl_app_present`, `sdl_app_queue_audio`.
- `sdl_app_sample_rate`, `sdl_app_audio_enabled`.

## Catatan

Header ini tidak menyertakan SDL; hanya `union SDL_Event` yang
di-forward-declare di header input.
