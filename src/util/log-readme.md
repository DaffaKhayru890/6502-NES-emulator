# log.c

**Peran:** Implementasi logging berlevel.

## Isi utama

- Level global kecil: ERROR, WARN, INFO, DEBUG.
- `log_set_level()` / `log_get_level()`.
- `log_message()`: cetak prefix level dan pesan berformat ke stderr.

## Catatan

Log bukan jalur panas. Jangan memanggil `log_debug()` setiap instruksi.
