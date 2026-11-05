# cartridge.c

**Peran:** Siklus hidup cartridge dan penerusan akses ke mapper.

## Isi utama

- `cartridge_allocate()` / `cartridge_destroy()`.
- `cartridge_cpu_read/write()` dan `cartridge_ppu_read/write()`.
- Getter metadata: mapper, submapper, status NES 2.0, region, mirroring,
  ukuran PRG/CHR, CHR RAM, battery, trainer.

## Catatan

Semua akses memori diteruskan ke `cart->mapper`; cartridge tidak mengetahui
detail bank switching. `cartridge_destroy()` membebaskan PRG, CHR, dan
mapper.
