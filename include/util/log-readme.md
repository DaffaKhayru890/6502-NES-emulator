# log.h

**Peran:** API publik logging.

## Isi utama

- enum `LogLevel`.
- `log_set_level`, `log_get_level`, `log_message`.
- makro `log_error`, `log_warn`, `log_info`, `log_debug`.

## Catatan

`log_message` memakai atribut format printf pada GCC/Clang agar kesalahan
format terdeteksi saat kompilasi.
