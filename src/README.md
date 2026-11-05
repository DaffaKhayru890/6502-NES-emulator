# Sumber Inti TinyNES

Direktori `src/` berisi implementasi emulator. Setiap subdirektori adalah
satu modul perangkat keras atau lapisan aplikasi.

| Direktori | Tanggung jawab |
|---|---|
| `core/` | Objek `Nes`, scheduler, konfigurasi CLI, kode error |
| `cpu/` | CPU RP2A03: fetch, addressing, opcode, flag, interrupt |
| `bus/` | Peta memori CPU dan OAM DMA |
| `ppu/` | PPU RP2C02: register, VRAM, rendering, NMI |
| `apu/` | APU RP2A03: lima kanal, frame sequencer, mixer |
| `cartridge/` | Loader iNES 1.0/NES 2.0, abstraksi mapper, mapper NROM |
| `input/` | Shift register controller |
| `frontend/` | Jendela, texture, audio queue, input SDL2 |
| `util/` | Tipe dasar, logging, pembacaan file |

`main.c` adalah satu-satunya file di akar `src/`; ia menghubungkan
konfigurasi, frontend, dan inti emulator.

Aturan penting:

- File di `src/` selain `frontend/` tidak boleh memakai header SDL2.
- Header internal (`*_internal.h`) hanya dipakai antarfungsi dalam modul.
- Setiap file sumber memiliki dokumentasi `<basename>-readme.md`.
