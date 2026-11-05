# file.c

**Peran:** Membaca seluruh file biner ke buffer `malloc`.

## Isi utama

- `file_read_all()`: buka file, cari ukuran, alokasikan, baca, dan tutup.
  Mengembalikan `ERR_IO` atau `ERR_ALLOCATION` bila gagal.
- `file_data_free()`: bebaskan buffer dan kosongkan struktur; aman dipanggil
  berulang kali.

## Catatan

Pemanggil memiliki buffer hasil dan wajib membebaskannya.
