# Header PPU

- `ppu.h`: tipe buram `PPU`, register, clock, NMI, OAM DMA, framebuffer,
  palette RGB, dan posisi scanline/dot.
- `ppu_memory.h`: akses pattern/nametable/palette/OAM tanpa efek samping
  register.
- `ppu_registers.h`: nomor register dan bit field `$2000`, `$2001`, `$2002`,
  serta posisi bit loopy register.

Framebuffer yang dikembalikan `ppu_framebuffer()` berisi 256x240 indeks
palette 0-63.
