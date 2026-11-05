# mapper.h

**Peran:** Antarmuka polimorfik mapper.

## Isi utama

- `MapperOps` berisi pointer `cpu_read`, `cpu_write`, `ppu_read`,
  `ppu_write`, dan `destroy`.
- struct `Mapper` berisi `ops` dan `context`.
- fungsi pembungkus `mapper_create/destroy` dan akses memori.

## Catatan

`destroy` boleh `NULL` bila context tidak dialokasikan terpisah, seperti
pada NROM.
