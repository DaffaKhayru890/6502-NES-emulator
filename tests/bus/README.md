# Test Bus

`test_cpu_bus.c` menguji peta memori CPU secara langsung:

- mirror RAM `$0000-$1FFF`;
- mirror register PPU `$2008-$3FFF`;
- penerusan ke PPU, APU, cartridge, dan controller;
- open bus;
- OAM DMA dan stall;
- pembacaan tombol serial.
