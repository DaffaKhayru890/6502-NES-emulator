# Modul Util

Utilitas kecil yang dipakai seluruh proyek.

## File

| File | Peran |
|---|---|
| `log.c` | Logging berlevel ke `stderr` |
| `file.c` | Membaca seluruh file biner ke buffer `malloc` |

## Logging

Level: `ERROR`, `WARN`, `INFO`, `DEBUG`. Default `INFO`; `--debug` menaikkan
ke `DEBUG`. Makro `log_error`, `log_warn`, `log_info`, dan `log_debug`
meneruskan format printf.

Log bukan jalur panas. Jangan memanggil `log_debug()` per instruksi.

## File

`file_read_all()` membuka file, menentukan ukuran, mengalokasikan buffer,
membaca seluruh isi, dan mengembalikan `ErrorCode`. Pemanggil wajib memanggil
`file_data_free()`.

`file_data_free()` aman dipanggil berulang kali dan mengosongkan struktur.
