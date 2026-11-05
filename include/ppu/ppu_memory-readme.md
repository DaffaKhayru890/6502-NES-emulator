# ppu_memory.h

**Peran:** Akses memori video tanpa efek samping register.

## Isi utama

- `ppu_memory_read/write` untuk alamat 14-bit.
- `ppu_oam_read/write`.
- `ppu_palette_read/write` dengan mirroring.

## Catatan

Berbeda dari `ppu_read_register()`, fungsi di sini tidak memakai buffer
baca atau menaikkan alamat.
