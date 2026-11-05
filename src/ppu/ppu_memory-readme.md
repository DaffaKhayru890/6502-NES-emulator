# ppu_memory.c

**Peran:** Peta memori video PPU.

## Isi utama

- `ppu_memory_read()` / `ppu_memory_write()` untuk pattern, nametable,
  attribute, dan palette.
- Mirroring nametable berdasarkan `Mirroring` cartridge.
- Mirror `$3000-$3EFF` ke `$2000-$2EFF`.
- `ppu_oam_read()` / `ppu_oam_write()`.
- `ppu_palette_read()` / `ppu_palette_write()` dengan mirror `$3F10` dst.

## Catatan

Fungsi ini tidak melakukan buffering atau increment alamat; efek samping
register ditangani `ppu_registers.c`.
