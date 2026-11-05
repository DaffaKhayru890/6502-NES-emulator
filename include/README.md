# Header Publik

Direktori `include/` berisi API publik setiap modul. Header di sini tidak
memakai SDL2 kecuali `frontend/`, sehingga inti emulator tetap portabel.

| Direktori | Header | Modul |
|---|---|---|
| `core/` | `nes.h`, `config.h`, `error.h` | Komposisi dan konfigurasi |
| `cpu/` | `cpu_2a03.h` | CPU |
| `bus/` | `cpu_bus.h` | Bus CPU |
| `ppu/` | `ppu.h`, `ppu_memory.h`, `ppu_registers.h` | PPU |
| `apu/` | `apu.h` | APU |
| `cartridge/` | `cartridge.h`, `mapper.h`, `mapper_nrom.h` | Cartridge |
| `input/` | `controller.h` | Controller |
| `frontend/` | `sdl_app.h`, `sdl_video.h`, `sdl_audio.h`, `sdl_input.h` | Frontend SDL2 |
| `util/` | `types.h`, `log.h`, `file.h` | Utilitas |

## Aturan

- Header publik hanya mendeklarasikan API yang boleh dipakai modul lain.
- State internal tidak diekspos; tipe buram (`typedef struct X X;`) dipakai
  bila memungkinkan.
- `cpu_2a03.h` mengekspos struct `CPU` karena pengujian dan debugger perlu
  membaca register.
- Komentar dokumentasi memakai Bahasa Indonesia.
