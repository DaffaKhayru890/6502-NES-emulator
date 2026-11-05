# test.h

**Peran:** Makro pemeriksaan untuk seluruh suite.

## Isi utama

- `CHECK_TRUE(cond)` dan `CHECK_FALSE(cond)`.
- `CHECK_EQ_INT`, `CHECK_EQ_U8`, `CHECK_EQ_U16`, `CHECK_EQ_SIZE`.
- Penghitung global pemeriksaan dan kegagalan.
- Makro mencetak file, baris, nilai aktual, dan harapan bila gagal.

## Catatan

Makro tidak menghentikan suite; semua kegagalan dilaporkan agar debugging
lebih cepat.
