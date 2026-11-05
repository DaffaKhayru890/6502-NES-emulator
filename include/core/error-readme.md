# error.h

**Peran:** Kode hasil yang dipakai seluruh TinyNES.

## Isi utama

- enum `ErrorCode` dari `ERR_OK` sampai error SDL, termasuk
  `ERR_UNSUPPORTED_CONSOLE` dan `ERR_UNSUPPORTED_REGION` untuk NES 2.0.
- `error_string()` untuk pesan Bahasa Indonesia.
- `error_report()` untuk mencetak konteks + pesan ke stderr.

## Catatan

Semua operasi yang dapat gagal mengembalikan `ErrorCode`; tidak ada
kegagalan yang hilang diam-diam.
