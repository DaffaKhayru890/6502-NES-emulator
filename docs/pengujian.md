# Pengujian

## Menjalankan

```sh
make test
```

atau:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DTINYNES_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Executable test juga dapat dijalankan langsung untuk melihat ringkasan:

```sh
./build/tests/tinynes_tests
```

Keluaran akhir berbentuk:

```
Ringkasan: 9 suite, 312 pemeriksaan, 0 kegagalan.
```

## Struktur

```
tests/
  test.h                  Makro CHECK_*
  test_main.c             Menjalankan seluruh suite
  test_helpers.c/.h       FakeBus dan pembuat ROM iNES sintetis
  test_suites.h           Deklarasi suite
  cpu/                    Register, addressing, instruksi
  bus/                    Peta memori dan mirroring
  ppu/                    Memori dan register/timing
  cartridge/              Loader iNES dan NROM
  apu/                    Register dan perilaku kanal
  integration/            NES lengkap dengan ROM sintetis
```

## Cakupan

### CPU

- Nilai awal register setelah reset dan vektor reset.
- Setiap addressing mode: immediate, zero page, zero page,X/Y, absolute,
  absolute,X/Y, indirect, indexed indirect, dan indirect indexed.
- Bug `JMP ($xxFF)`.
- Instruksi load/store, aritmetika, logika, shift/rotate, compare,
  increment/decrement, branch, jump/subroutine, stack, flag, dan NOP.
- Opcode tidak resmi umum: LAX, SAX, DCP, SLO.
- Perhitungan siklus dasar dan penalti page crossing.

### Bus

- Mirror RAM.
- Mirror register PPU.
- Penerusan ke PPU/APU/cartridge.
- Open bus.
- OAM DMA dan stall.
- Pembacaan controller.

### PPU

- Pattern table dari CHR.
- Mirroring nametable horizontal/vertical.
- Mirror palette.
- Increment alamat VRAM 1 dan 32.
- Buffer baca `$2007`.
- OAMADDR/OAMDATA.
- VBlank, NMI, dan pembacaan `$2002`.
- Pembersihan flag di pre-render.
- Timing PAL: pre-render di scanline 311, VBlank di 241, dan frame selesai
  setelah 240 scanline visible.

### Cartridge

- Validasi magic dan ukuran.
- PRG 16 KiB dicerminkan.
- PRG 32 KiB.
- CHR ROM dan CHR RAM.
- Trainer.
- Penerimaan NES 2.0: mapper 12-bit, submapper, CHR RAM, dan encoding
  ukuran eksponen.
- Region dari timing NES 2.0: NTSC, PAL, dan multi-region.
- Penolakan mapper yang tidak didukung, console type NES 2.0 selain
  NES/Famicom, timing Dendy, dan file terpotong.

### APU

- Status kanal setelah enable/disable.
- Panjang gelombang dan register.
- IRQ frame counter NTSC dan PAL.
- Sampling NTSC dan PAL serta pengosongan ring buffer.

### Integrasi

- Membuat `Nes`, memuat ROM sintetis 16 KiB PRG + CHR RAM.
- Menjalankan beberapa frame.
- Memastikan framebuffer dan NMI berjalan tanpa crash.
- Region PAL otomatis dari NES 2.0, frame rate sekitar 50 Hz, dan override
  eksplisit NTSC pada ROM PAL.

## Menambah Test

1. Tambahkan fungsi `void test_xxx_run(void);` di `tests/test_suites.h`.
2. Implementasikan di file baru, misalnya `tests/ppu/test_xxx.c`.
3. Panggil fungsi dari `tests/test_main.c`.
4. Tambahkan file `.c` ke `tests/CMakeLists.txt`.
5. Gunakan makro dari `tests/test.h`.

Contoh:

```c
#include "../test.h"

void test_contoh_run(void)
{
    CHECK_EQ_INT(2 + 2, 4);
    CHECK_TRUE(true);
    CHECK_EQ_U8(0x42, 0x42);
}
```

## Prinsip Test

- Tidak memakai ROM komersial.
- ROM sintetis dibuat di memori oleh `test_ines_create()`.
- Test tidak membuka jendela atau device audio.
- Setiap kegagalan mencetak file, baris, nilai aktual, dan nilai harapan.
- Test harus deterministik dan tidak bergantung pada waktu host.

## Sanitizer

Untuk memeriksa kebocoran dan undefined behavior:

```sh
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DTINYNES_BUILD_TESTS=ON -DTINYNES_SANITIZE=ON
cmake --build build-asan --parallel
ctest --test-dir build-asan --output-on-failure
```

Sanitizer diaktifkan untuk library inti, frontend, aplikasi, dan executable
test. Dengan begitu objek yang diinstrumentasi dan runtime ASan/UBSan
ditautkan secara konsisten.
