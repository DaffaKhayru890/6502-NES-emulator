# cartridge_internal.h

**Peran:** Definisi struct `Cartridge` untuk loader dan mapper.

## Isi utama

- Buffer PRG ROM dan ukurannya.
- Buffer CHR (ROM atau RAM) dan ukurannya.
- PRG RAM 8 KiB.
- Mirroring, nomor mapper, submapper, region, flag NES 2.0, flag
  battery/trainer, dan pointer `Mapper`.

## Catatan

Header ini tidak dipublikasikan. Kode luar hanya memakai tipe buram dari
`cartridge.h`.
