# ppu_registers.h

**Peran:** Konstanta register dan bit field PPU.

## Isi utama

- Nomor register `PPU_REG_CTRL`..`PPU_REG_DATA`.
- Bit `PPUCTRL`, `PPUMASK`, `PPUSTATUS`.
- Mask posisi loopy register: coarse X/Y, nametable, fine Y.

## Catatan

Header ini tidak mendeklarasikan fungsi; hanya konstanta agar kode
rendering dan register memakai nama yang sama.
