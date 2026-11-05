# Pengujian TinyNES

Direktori ini berisi executable `tinynes_tests`, yang menjalankan seluruh
suite tanpa jendela, audio, atau ROM eksternal.

## Menjalankan

```sh
make test
# atau
ctest --test-dir build --output-on-failure
# atau
./build/tests/tinynes_tests
```

## Struktur

| Path | Isi |
|---|---|
| `test.h` | Makro `CHECK_*` dan penghitung pemeriksaan |
| `test_main.c` | Pemanggil seluruh suite |
| `test_helpers.c/.h` | `FakeBus` dan pembuat ROM iNES sintetis |
| `test_suites.h` | Deklarasi fungsi suite |
| `cpu/` | Register, addressing, instruksi |
| `bus/` | Peta memori CPU |
| `ppu/` | Memori dan register PPU |
| `cartridge/` | Loader dan NROM |
| `apu/` | Register dan sampling APU |
| `integration/` | NES lengkap dengan ROM sintetis |

## Prinsip

- Deterministik dan tidak bergantung waktu host.
- Tidak memakai ROM komersial.
- Setiap kegagalan mencetak file, baris, aktual, dan harapan.
- Ringkasan akhir: jumlah suite, pemeriksaan, dan kegagalan.
