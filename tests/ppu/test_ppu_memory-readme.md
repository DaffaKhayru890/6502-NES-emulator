# test_ppu_memory.c

**Peran:** Menguji memori video PPU.

## Cakupan

- Baca/tulis pattern table lewat cartridge.
- Mirroring nametable horizontal dan vertical.
- Mirror `$3000-$3EFF`.
- Palette RAM dan mirror `$3F10` dst.
- OAM read/write.
