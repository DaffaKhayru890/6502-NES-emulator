# TinyNES

TinyNES adalah emulator Nintendo Entertainment System (NES) yang ditulis
sepenuhnya dalam **C17** dengan **SDL2** sebagai lapisan video, audio, dan
input. Proyek ini dibuat sebagai media belajar: setiap modul perangkat keras
dipisahkan ke direktori sendiri, berkomunikasi lewat antarmuka eksplisit, dan
tidak memakai state perangkat keras global.

Status proyek: **edukasi, NTSC/PAL, mapper NROM**. Emulator dapat memuat
ROM iNES 1.0 maupun NES 2.0 ber-mapper 0, menjalankan CPU 2A03, PPU, APU
lima kanal, dua controller, dan menampilkan gambar 256x240 melalui SDL2.

> Proyek ini tidak menyertakan ROM komersial. Gunakan ROM homebrew yang
> boleh didistribusikan, atau ROM sintetis yang dibuat oleh pengujian.

## Fitur

| Bagian | Dukungan |
|---|---|
| CPU | Ricoh RP2A03, seluruh opcode resmi, beberapa opcode tidak resmi umum, interrupt NMI/IRQ/BRK, siklus dasar dan penalti page crossing |
| Bus CPU | RAM 2 KiB + mirror, register PPU/APU, OAM DMA, controller, open bus, PRG RAM/ROM |
| Cartridge | iNES 1.0 dan NES 2.0, NROM (mapper 0), PRG 16/32 KiB, CHR ROM atau CHR RAM, trainer, battery flag |
| PPU | Pola dot-driven 341x262 (NTSC) atau 341x312 (PAL), background, sprite 8x8/8x16, sprite 0 hit, overflow flag, VBlank/NMI, palette, mirroring nametable, grayscale, clipping 8 piksel kiri |
| APU | Dua pulse, triangle, noise, DMC, envelope, sweep, length counter, linear counter, frame sequencer regional, mixer nonlinear |
| Input | Dua controller NES lewat keyboard dan game controller SDL2 |
| Frontend | Jendela SDL2, penskalaan integer, audio queue, pacing 60,0988 Hz (NTSC) atau 50,007 Hz (PAL) |

## Kebutuhan

- Linux x86-64 (kode portabel, tetapi baru divalidasi di Linux)
- Compiler dengan dukungan C17 (GCC atau Clang)
- CMake >= 3.16
- pkg-config
- SDL2 dan header pengembangannya

Contoh pemasangan di Debian/Ubuntu:

```sh
sudo apt install build-essential cmake pkg-config libsdl2-dev
```

## Membangun

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Atau memakai pembungkus Make:

```sh
make            # build Release
make debug      # build Debug + test
make test       # build Debug dan jalankan CTest
make release    # build Release
make clean
```

Hasil build ada di `build/tinynes`.

## Menjalankan

```sh
./build/tinynes [opsi] <rom.nes>
```

Opsi yang tersedia:

| Opsi | Keterangan |
|---|---|
| `--scale N` | Faktor pembesaran integer 1..10 (default 3) |
| `--mute` | Matikan keluaran suara, APU tetap berjalan |
| `--no-audio` | Jangan membuka device audio SDL2 |
| `--debug` | Cetak statistik CPU/PPU setiap 60 frame |
| `--region auto` | Pilih timing dari metadata ROM (default); iNES 1.0 menjadi NTSC |
| `--region ntsc` | Paksa timing NTSC |
| `--region pal` | Paksa timing PAL |
| `--help` | Tampilkan bantuan |

Region eksplisit mengalahkan metadata NES 2.0. Tanpa opsi ini, ROM PAL
NES 2.0 otomatis dijalankan dengan timing PAL.

Contoh:

```sh
./build/tinynes --scale 4 --debug game.nes
./build/tinynes --region pal game-pal.nes
```

Tanpa ROM eksternal, Anda dapat membuat ROM sintetis yang sudah divalidasi:

```sh
python3 examples/make_synthetic_rom.py contoh.nes
python3 examples/make_synthetic_rom.py --pal contoh-pal.nes
./build/tinynes --no-audio --debug contoh.nes
```

## Kontrol

| Tombol NES | Keyboard |
|---|---|
| A | `Z` |
| B | `X` |
| Select | `Right Shift` |
| Start | `Enter` |
| D-pad | Tombol panah |
| Keluar | `Escape` atau tutup jendela |

Game controller SDL2 yang terhubung otomatis dipetakan: controller pertama
ke port 1, controller kedua ke port 2.

## Pengujian

```sh
make test
```

atau langsung:

```sh
ctest --test-dir build --output-on-failure
```

Test terdiri dari 9 suite: register CPU, addressing mode, semantik
instruksi, bus CPU, memori PPU, register/timing PPU (NTSC dan PAL),
cartridge NROM, APU (NTSC dan PAL), dan integrasi NES dengan ROM sintetis.
Seluruh pengujian tidak membutuhkan ROM komersial.

## Struktur Proyek

```
include/            Header publik per modul
src/core/           Komposisi Nes dan scheduler
src/cpu/            CPU RP2A03
src/bus/            Bus memori CPU
src/ppu/            PPU RP2C02
src/apu/            APU RP2A03
src/cartridge/      Loader iNES 1.0/NES 2.0 dan mapper NROM
src/input/          Controller shift register
src/frontend/       Video, audio, input SDL2
src/util/           Tipe dasar, log, dan baca file
tests/              Unit test dan integrasi
examples/           Pembuat ROM sintetis untuk mencoba emulator
assets/             Keterangan bahwa tidak ada aset/ROM yang disertakan
docs/               Dokumentasi teknis
```

Setiap direktori modul memiliki `README.md`. Setiap file `.c`/`.h` memiliki
file dokumentasi bersebelahan bernama `<basename>-readme.md`.

## Dokumentasi

- [Arsitektur](docs/arsitektur.md)
- [Membangun dan menjalankan](docs/membangun.md)
- [Penggunaan dan kontrol](docs/penggunaan.md)
- [Timing emulasi](docs/timing.md)
- [Peta memori](docs/peta-memori.md)
- [Pengujian](docs/pengujian.md)
- [Format ROM iNES 1.0/NES 2.0](docs/format-rom.md)
- [Batasan dan akurasi](docs/batasan.md)
- [Indeks dokumentasi](docs/README.md)

## Batasan Utama

TinyNES sengaja dibuat sederhana. Yang **belum** didukung atau belum presisi:

- Hanya mapper 0 (NROM); mapper lain ditolak, baik pada iNES 1.0 maupun
  NES 2.0.
- NES 2.0 didukung untuk metadata mapper/ukuran/timing, tetapi console type
  Vs./PlayChoice, timing Dendy, dan ukuran RAM khusus ditolak atau
  diabaikan.
- Timing NTSC dan PAL didukung; Dendy belum.
- PPU tidak meniru seluruh konflik bus, open bus PPU, dan bug overflow
  sprite secara persis.
- Evaluasi sprite dilakukan di awal scanline, bukan pada siklus 65-256.
- DMC belum menambahkan stall DMA ke CPU.
- Audio memakai satu kanal mono; tidak ada filter analog atau resampling
  berkualitas tinggi.
- Opcode tidak resmi yang "tidak stabil" dapat berperilaku berbeda dari
  hardware asli.

Rincian lengkap ada di [docs/batasan.md](docs/batasan.md).

## Lisensi

Kode proyek ini ditulis untuk tujuan edukasi. Anda bebas mempelajari dan
memodifikasinya. ROM yang dijalankan tetap tunduk pada lisensi masing-masing.
