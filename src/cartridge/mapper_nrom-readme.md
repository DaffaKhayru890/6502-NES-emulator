# mapper_nrom.c

**Peran:** Implementasi mapper 0 (NROM).

## Pemetaan

- `$6000-$7FFF`: PRG RAM 8 KiB.
- `$8000-$FFFF`: PRG ROM; 16 KiB dicerminkan dua kali, 32 KiB langsung.
- `$0000-$1FFF`: CHR ROM atau CHR RAM 8 KiB.

## Fungsi

`nrom_cpu_read`, `nrom_cpu_write`, `nrom_ppu_read`, `nrom_ppu_write`, dan
`mapper_nrom_create`.

## Catatan

Context mapper adalah `Cartridge` itu sendiri; tidak ada alokasi tambahan
dan `destroy` bernilai `NULL`.
