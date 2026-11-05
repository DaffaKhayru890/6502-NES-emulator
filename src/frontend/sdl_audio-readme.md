# sdl_audio.c

**Peran:** Output audio SDL2 berbasis queue.

## Isi utama

- `sdl_audio_create()`: membuka device mono S16 pada sample rate yang
  diminta.
- `sdl_audio_queue()`: menambahkan sampel dengan `SDL_QueueAudio`.
- `sdl_audio_queued_bytes()`: memantau ukuran queue.
- `sdl_audio_sample_rate()`, `sdl_audio_ok()`, `sdl_audio_destroy()`.

## Catatan

`SDL_QueueAudio` aman terhadap thread audio internal SDL; APU tidak dibaca
dari dua thread sekaligus.
