# test_main.c

**Peran:** Titik masuk executable test.

## Alur

1. Panggil setiap fungsi `test_*_run()` dari `test_suites.h`.
2. Setiap suite mencetak namanya dan status OK.
3. Cetak ringkasan jumlah suite, pemeriksaan, dan kegagalan.
4. Kembalikan kode bukan nol bila ada kegagalan.

## Catatan

Urutan suite tidak penting karena setiap suite membuat state sendiri.
