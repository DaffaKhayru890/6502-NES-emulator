# Format ROM iNES 1.0 dan NES 2.0

TinyNES membaca file `.nes` dengan header 16 byte. Kedua format dideteksi
otomatis:

- **iNES 1.0**: bit 2-3 flag 7 bukan `10b`.
- **NES 2.0**: bit 2-3 flag 7 bernilai `10b` (`flags7 & 0x0C == 0x08`).

Dukungan NES 2.0 berlaku untuk mapper 0 (NROM). Metadata NES 2.0 yang tidak
relevan untuk NROM diabaikan dengan aman.

## Header 16 Byte

| Offset | iNES 1.0 | NES 2.0 |
|---|---|---|
| 0-3 | Magic `4E 45 53 1A` | Magic `4E 45 53 1A` |
| 4 | PRG ROM, unit 16 KiB | PRG ROM ukuran rendah, unit 16 KiB |
| 5 | CHR ROM, unit 8 KiB (0 = CHR RAM) | CHR ROM ukuran rendah, unit 8 KiB |
| 6 | Flag 6 | Flag 6 |
| 7 | Flag 7 | Flag 7 + penanda NES 2.0 |
| 8 | Padding | Mapper tinggi + submapper |
| 9 | Padding | Ukuran PRG/CHR bagian tinggi |
| 10 | Padding | Ukuran PRG RAM/NVRAM |
| 11 | Padding | Ukuran CHR RAM/NVRAM |
| 12 | Padding | CPU/PPU timing |
| 13 | Padding | System type |
| 14 | Padding | Jumlah miscellaneous ROM |
| 15 | Padding | Default expansion device |

## Flag 6

| Bit | Arti |
|---|---|
| 0 | Mirroring: 0 = horizontal, 1 = vertical |
| 1 | Battery-backed PRG RAM |
| 2 | Trainer 512 byte ada |
| 3 | Four-screen mirroring |
| 4-7 | Nomor mapper bagian rendah |

## Flag 7

| Bit | Arti |
|---|---|
| 0-1 | Console type: 0 = NES/Famicom, 1 = Vs., 2 = PlayChoice, 3 = Extended |
| 2-3 | Penanda format; `10b` = NES 2.0 |
| 4-7 | Nomor mapper bagian tengah |

## Mapper

iNES 1.0:

```
mapper = (flags6 >> 4) | (flags7 & 0xF0)
```

NES 2.0 menambahkan byte 8:

```
mapper = (flags6 >> 4) | (flags7 & 0xF0) | (byte8 & 0xF0)
submapper = byte8 & 0x0F
```

TinyNES hanya menerima **mapper 0 (NROM)**. Submapper dibaca dan disimpan
sebagai metadata, tetapi belum mengubah perilaku NROM.

## Ukuran ROM

iNES 1.0:

```
PRG = byte4 * 16384
CHR = byte5 * 8192
```

NES 2.0 menggabungkan byte 4/5 dengan byte 9:

```
prg_encoded = byte4 | ((byte9 & 0x0F) << 8)
chr_encoded = byte5 | ((byte9 >> 4) << 8)
```

Nilai 12-bit di atas diproses sebagai berikut:

- Jika nibble tinggi bukan `0xF`: ukuran = `encoded * unit`.
- Jika nibble tinggi `0xF`: encoding eksponen-multiplier
  `1111 EEEE EEMM`, dengan ukuran = `2^E * (MM * 2 + 1)` byte.

Contoh: PRG 32 KiB dapat ditulis sebagai `encoded = 0xF3C`
(`E = 15`, `MM = 0`, `2^15 = 32768`).

## Batasan NROM

NROM hanya memiliki:

- PRG ROM 16 KiB (dicerminkan di `$8000-$FFFF`) atau 32 KiB;
- CHR ROM 8 KiB, atau CHR RAM 8 KiB bila CHR = 0.

Ukuran lain ditolak dengan `ERR_INES_FORMAT`, meskipun header NES 2.0 mampu
mengungkapkannya. Ini mencegah pemetaan yang salah tanpa mapper yang sesuai.

## Console dan Timing

TinyNES hanya menjalankan NES/Famicom. Timing NTSC dan PAL didukung.

| Field NES 2.0 | Nilai | Perilaku |
|---|---|---|
| Console type | 0 (NES/Famicom) | Diterima |
| Console type | 1, 2, 3 | Ditolak: `ERR_UNSUPPORTED_CONSOLE` |
| Timing | 0 (NTSC) | Diterima sebagai NTSC |
| Timing | 1 (PAL) | Diterima sebagai PAL |
| Timing | 2 (multi-region) | Diterima, dijalankan sebagai NTSC |
| Timing | 3 (Dendy) | Ditolak: `ERR_UNSUPPORTED_REGION` |

Metadata timing dari NES 2.0 diekspos lewat `cartridge_region()`. Region
efektif `Nes` mengikuti konfigurasi `--region`: `auto` memakai metadata
ini, sedangkan `ntsc`/`pal` memaksa region dan mengalahkan metadata.

## Field NES 2.0 yang Diabaikan

Field berikut dibaca oleh format tetapi belum memengaruhi emulasi:

- ukuran PRG RAM/NVRAM (TinyNES memakai PRG RAM tetap 8 KiB);
- ukuran CHR RAM/NVRAM (CHR RAM tetap 8 KiB);
- system type byte 13;
- jumlah miscellaneous ROM (byte 14) dan default expansion device (byte 15).

Byte tambahan setelah PRG/CHR (misalnya miscellaneous ROM) tidak dibaca,
tetapi tidak membuat pemuatan gagal selama data wajib sudah lengkap.

## Trainer

Jika flag 6 bit 2 aktif, loader melewati 512 byte trainer sebelum PRG ROM.
Isi trainer belum dipetakan ke ruang memori khusus.

## Validasi

Loader memeriksa, dalam urutan:

1. argumen dan ukuran minimal header;
2. magic bytes;
3. console type dan timing NES 2.0;
4. nomor mapper 12-bit;
5. decoding ukuran PRG/CHR;
6. kecocokan ukuran dengan NROM;
7. konsistensi ukuran file;
8. alokasi PRG, CHR, dan mapper.

Setiap kegagalan mengisi `ErrorCode` yang dapat dicetak dengan
`error_string()`.

## ROM Sintetis untuk Test

Test memakai `test_ines_create()` untuk header iNES 1.0 dan memodifikasi
byte header secara langsung untuk kasus NES 2.0. `examples/make_synthetic_rom.py`
juga dapat menghasilkan ROM NES 2.0 dengan opsi `--nes2` atau NES 2.0
bertiming PAL dengan opsi `--pal`.
