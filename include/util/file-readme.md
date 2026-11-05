# file.h

**Peran:** API publik pembacaan file biner.

## Isi utama

- struct `FileData` (`data`, `size`).
- `file_read_all(path, out)`.
- `file_data_free(file)`.

## Catatan

Buffer dimiliki pemanggil dan wajib dibebaskan dengan `file_data_free()`.
Fungsi free aman dipanggil berulang kali.
