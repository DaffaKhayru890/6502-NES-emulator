# Membangun TinyNES

## Prasyarat

- CMake 3.16 atau lebih baru
- Compiler C17 (GCC 8+ atau Clang 7+)
- `pkg-config`
- SDL2 versi 2.x beserta header pengembangan

Debian/Ubuntu:

```sh
sudo apt update
sudo apt install build-essential cmake pkg-config libsdl2-dev
```

Fedora:

```sh
sudo dnf install gcc cmake pkgconf-pkg-config SDL2-devel
```

Arch Linux:

```sh
sudo pacman -S base-devel cmake pkgconf sdl2
```

## Build Dasar

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Binary dihasilkan di `build/tinynes`.

Jika CMake tidak menemukan SDL2, pastikan `pkg-config --modversion sdl2`
berhasil. CMake memakai `pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)`.

## Opsi CMake

| Opsi | Default | Keterangan |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Release` | `Debug`, `Release`, `RelWithDebInfo`, atau `MinSizeRel` |
| `TINYNES_BUILD_TESTS` | `ON` | Bangun executable unit test |
| `TINYNES_SANITIZE` | `OFF` | Aktifkan AddressSanitizer dan UBSan |

Contoh build Debug dengan test:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DTINYNES_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Build dengan sanitizer:

```sh
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DTINYNES_BUILD_TESTS=ON \
  -DTINYNES_SANITIZE=ON
cmake --build build-asan --parallel
ctest --test-dir build-asan --output-on-failure
```

## Make Wrapper

```sh
make            # Release
make debug      # Debug + test, tanpa menjalankan test
make test       # Debug + bangun + jalankan CTest
make release    # Release
make rebuild    # hapus build lalu build ulang
make format     # clang-format seluruh .c/.h
make clean      # hapus direktori build
```

Variabel yang dapat dioverride:

```sh
make BUILD_DIR=build-opt JOBS=8
```

## Struktur Target

| Target | Isi | Dependensi |
|---|---|---|
| `tinynes_core` | CPU, bus, PPU, APU, cartridge, controller, util | pustaka C standar |
| `tinynes_frontend` | Video/audio/input SDL2 | `tinynes_core`, SDL2 |
| `tinynes` | `src/main.c` | `tinynes_frontend` |
| `tinynes_tests` | Seluruh suite test | `tinynes_core` |

Memisahkan inti dari frontend membuat unit test dapat berjalan tanpa membuka
jendela atau device audio.

## Peringatan Compiler

Build memakai:

```
-Wall -Wextra -Wpedantic -Wshadow -Wconversion
```

`-Wconversion` sengaja aktif agar konversi sempit (misalnya `int` ke
`uint8_t`) ditulis eksplisit dengan cast. Build harus bersih tanpa peringatan.

## Pemecahan Masalah

**`SDL2 tidak ditemukan`**

Periksa `pkg-config --cflags --libs sdl2`. Bila SDL2 dipasang di prefix
khusus, set `PKG_CONFIG_PATH`.

**`undefined reference` saat link**

Pastikan seluruh file baru sudah ditambahkan ke `CMakeLists.txt` pada target
yang tepat (`tinynes_core`, bukan frontend, bila tidak memakai SDL2).

**Test gagal karena ROM**

Tidak ada ROM eksternal yang dibutuhkan. Suite integrasi membuat ROM iNES
sintetis di memori. Bila test gagal, itu masalah kode atau ekspektasi test,
bukan file ROM yang hilang.

**Audio tidak keluar**

Coba `--no-audio` untuk memastikan sisa aplikasi berjalan. Periksa apakah
device audio tersedia di lingkungan (container/headless sering tidak punya
device). Emulator tetap berjalan tanpa audio.
