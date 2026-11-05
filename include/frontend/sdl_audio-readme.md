# sdl_audio.h

**Peran:** API publik output audio SDL2.

## Isi utama

- tipe buram `SDLAudio`.
- `sdl_audio_create(sample_rate, error)`.
- `sdl_audio_queue`, `sdl_audio_queued_bytes`.
- `sdl_audio_sample_rate`, `sdl_audio_ok`, `sdl_audio_destroy`.

## Catatan

Device memakai format mono `AUDIO_S16SYS`.
