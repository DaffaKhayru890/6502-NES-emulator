# ines_loader.c

**Peran:** Membaca header iNES 1.0/NES 2.0 dan membentuk `Cartridge`.

## Alur

1. Validasi magic `NES\x1A`.
2. Deteksi NES 2.0 dari `(flags7 & 0x0C) == 0x08`.
3. Untuk NES 2.0: baca mapper 12-bit, submapper, console type, dan timing.
4. Tolak console type selain NES/Famicom; terima timing NTSC/PAL/
   multi-region dan tolak Dendy.
5. Hitung nomor mapper; hanya 0 yang didukung.
6. Decode ukuran PRG/CHR 12-bit, termasuk encoding eksponen-multiplier.
7. Validasi ukuran terhadap kemampuan NROM (PRG 16/32 KiB, CHR 0/8 KiB).
8. Validasi ukuran file dan trainer.
9. Alokasikan buffer, salin data, buat mapper NROM.

## Fungsi

- `decode_rom_size()`: decoder ukuran normal dan eksponen NES 2.0 dengan
  pemeriksaan overflow.
- `cartridge_load_memory()`: jalur utama, berguna untuk test.
- `cartridge_load_file()`: membaca file lalu memanggil jalur memori.

## Catatan

Kegagalan mengisi `ErrorCode` yang sesuai dan membebaskan alokasi parsial.
Metadata NES 2.0 di luar mapper/ukuran/timing/console belum diterapkan.
