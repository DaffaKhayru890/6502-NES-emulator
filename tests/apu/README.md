# Test APU

`test_apu.c` menguji perilaku dasar APU:

- status kanal pada `$4015`;
- enable/disable dan length counter;
- frame IRQ NTSC dan PAL;
- sampling NTSC dan PAL serta pengosongan ring buffer;
- sample rate yang dikembalikan.
