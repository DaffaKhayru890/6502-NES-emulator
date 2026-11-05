# test_cpu_bus.c

**Peran:** Menguji peta memori CPU.

## Cakupan

- Tulis/baca RAM dan mirror `$0800-$1FFF`.
- Mirror register PPU.
- Penerusan register PPU/APU.
- PRG RAM dan PRG ROM lewat cartridge.
- Open bus.
- OAM DMA dan nilai stall.
- Strobe dan pembacaan controller.
