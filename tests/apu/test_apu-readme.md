# test_apu.c

**Peran:** Menguji APU.

## Cakupan

- Status `$4015` sebelum dan sesudah enable.
- Length counter dan pengaruh disable.
- Register pulse/triangle/noise/DMC tidak crash.
- Frame IRQ dan pembacaan status, termasuk periode PAL (IRQ di siklus
  33253).
- Sampling pada sample rate yang diminta untuk NTSC dan PAL.
- Pengosongan ring buffer.
