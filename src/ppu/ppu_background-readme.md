# ppu_background.c

**Peran:** Pipeline fetch background PPU.

## Siklus per 8 dot

- `dot % 8 == 1`: reload shift register, fetch nametable.
- `dot % 8 == 3`: fetch attribute.
- `dot % 8 == 5`: fetch pattern plane rendah.
- `dot % 8 == 7`: fetch pattern plane tinggi.
- `dot % 8 == 0`: increment coarse X.

Shift register digeser setiap dot selama area fetch. `ppu_background_reset()`
mengosongkan latch dan shift register saat reset/pre-render.

## Catatan

Pattern base dipilih dari `$2000` bit 4. Fine X memilih bit keluaran saat
compositing.
