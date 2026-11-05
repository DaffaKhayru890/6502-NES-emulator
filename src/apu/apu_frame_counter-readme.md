# apu_frame_counter.c

**Peran:** Frame sequencer APU.

## Isi utama

- `clock_quarter_frame()`: envelope dan linear counter.
- `clock_half_frame()`: length counter dan sweep.
- `apu_frame_counter_reset()`: mode 4-step, timer awal.
- `apu_frame_counter_write()`: mode 4/5-step, IRQ inhibit, dan reload.
- `apu_frame_counter_clock()`: timer dan urutan step.

## Mode

- 4-step NTSC: step pada siklus 7457, 14913, 22371, 29828/29829; IRQ pada
  akhir bila tidak diinhibit.
- 4-step PAL: step pada siklus 8313, 16639, 24939, 33252/33253.
- 5-step NTSC: step pada 7457, 14913, 22371, 37281; reset 37282.
- 5-step PAL: step pada 8313, 16639, 24939, 41565; reset 41566.
- Mode 5-step tidak menghasilkan IRQ.
