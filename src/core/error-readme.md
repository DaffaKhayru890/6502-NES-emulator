# error.c

**Peran:** Mengubah `ErrorCode` menjadi pesan Bahasa Indonesia.

## Isi utama

- `error_string()`: switch seluruh nilai `ErrorCode`, termasuk fallback
  untuk kode tak dikenal. Pesan mencakup kegagalan NES 2.0 seperti console
  type dan timing yang tidak didukung.
- `error_report()`: mencetak `konteks: pesan` ke stderr bila kode bukan
  `ERR_OK`.

## Catatan

Pesan error tidak boleh mengandung path ROM secara otomatis; konteks
diberikan pemanggil agar tidak terjadi duplikasi.
