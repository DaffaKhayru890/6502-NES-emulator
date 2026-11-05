# Test PPU

- `test_ppu_memory.c`: pattern table, mirroring nametable, palette, dan OAM.
- `test_ppu_registers.c`: increment VRAM, buffer `$2007`, OAMADDR/OAMDATA,
  VBlank, NMI, pembacaan `$2002`, pembersihan flag, dan timing PAL
  (pre-render 311, 312 scanline).

Test memakai cartridge sintetis dengan CHR RAM bila diperlukan.
