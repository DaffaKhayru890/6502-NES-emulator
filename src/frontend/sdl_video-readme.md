# sdl_video.c

**Peran:** Jendela, renderer, dan presentasi framebuffer.

## Isi utama

- `sdl_video_create()`: membuat window 256x240 yang diperbesar secara
  integer, renderer tanpa vsync, dan texture streaming `ARGB8888`.
- `sdl_video_present()`: mengubah setiap indeks palette menjadi RGB lewat
  `ppu_palette_rgb()`, upload texture, lalu render.
- `sdl_video_destroy()`: membersihkan texture, renderer, dan window.

## Catatan

`SDL_RenderSetLogicalSize` dan `SDL_RenderSetIntegerScale` menjaga rasio
dan letterbox. Framebuffer sumber selalu 256x240.
