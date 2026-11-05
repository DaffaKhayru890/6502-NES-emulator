# Header Core

- `nes.h`: tipe buram `Nes`, pembuatan, pemuatan ROM, eksekusi frame,
  framebuffer, audio, tombol, dan akses komponen untuk debug/test.
- `config.h`: struct `Config` dan enum `Region`, plus parsing CLI.
- `error.h`: `ErrorCode` dan helper pelaporan error.

`nes.h` adalah satu-satunya header yang perlu dipakai frontend untuk
menjalankan emulator.
