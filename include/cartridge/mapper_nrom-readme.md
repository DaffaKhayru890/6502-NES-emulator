# mapper_nrom.h

**Peran:** Deklarasi pembuat mapper 0 (NROM).

## Isi utama

- `mapper_nrom_create(Cartridge *cartridge)`.

## Catatan

Context mapper adalah `Cartridge` itu sendiri, sehingga tidak ada alokasi
tambahan dan tidak perlu fungsi destroy khusus.
