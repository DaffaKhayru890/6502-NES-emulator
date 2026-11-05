# cartridge.h

**Peran:** API publik representasi cartridge.

## Isi utama

- enum `Mirroring`.
- tipe buram `Cartridge`.
- `cartridge_load_file`, `cartridge_load_memory`, `cartridge_destroy`.
- akses CPU/PPU.
- getter mapper, submapper, status NES 2.0, region, mirroring, ukuran,
  CHR RAM, battery, trainer.

## Catatan

Detail format iNES 1.0/NES 2.0 hanya diketahui loader; modul lain memakai
API ini.
