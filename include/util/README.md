# Header Util

- `types.h`: include standar (`stdbool`, `stddef`, `stdint`), versi, ukuran
  layar, enum dan helper `Region`, frekuensi CPU NTSC/PAL, jumlah
  scanline/dot, dan konstanta ukuran memori.
- `log.h`: level log dan makro `log_*`.
- `file.h`: `FileData`, `file_read_all`, dan `file_data_free`.

`types.h` sengaja kecil dan tidak menarik dependensi lain.
