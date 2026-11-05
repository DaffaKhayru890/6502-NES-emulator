# mapper.c

**Peran:** Pembungkus polimorfik untuk implementasi mapper.

## Isi utama

- `mapper_create()`: mengalokasikan `Mapper` dari `MapperOps` dan context.
- `mapper_destroy()`: memanggil `ops->destroy` bila ada, lalu membebaskan
  struct.
- `mapper_cpu_read/write()` dan `mapper_ppu_read/write()`: meneruskan ke
  pointer fungsi dengan pengecekan `NULL`.

## Catatan

Ini contoh polimorfisme di C: data (`context`) dipisahkan dari perilaku
(`ops`).
