# Header Frontend

- `sdl_app.h`: tipe buram `SDLApp`, pembuatan, event, tombol, present,
  audio, sample rate.
- `sdl_video.h`: pembuatan window/renderer dan present framebuffer.
- `sdl_audio.h`: device audio queue mono S16.
- `sdl_input.h`: keyboard dan game controller.

Header ini memakai `union SDL_Event` sebagai forward declaration agar file
yang tidak membutuhkan SDL tetap dapat menyertakan `sdl_input.h`.
