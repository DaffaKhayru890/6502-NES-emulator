# Modul Cartridge

Modul ini memisahkan format file dari cara pemetaan memori.

## File

| File | Peran |
|---|---|
| `ines_loader.c` | Membaca header iNES 1.0/NES 2.0, memvalidasi, mengalokasikan ROM |
| `cartridge.c` | Siklus hidup dan penerusan akses ke mapper |
| `mapper.c` | Pembungkus polimorfik `MapperOps` |
| `mapper_nrom.c` | Implementasi mapper 0 (NROM) |
| `cartridge_internal.h` | Struct `Cartridge` untuk loader dan mapper |

## Alur Pemuatan

```
cartridge_load_file()
  -> file_read_all()
  -> cartridge_load_memory()
       -> deteksi iNES 1.0 / NES 2.0
       -> validasi header, mapper, console, timing
       -> decode ukuran PRG/CHR
       -> alokasi PRG/CHR
       -> salin data
       -> mapper_nrom_create()
```

`cartridge_load_memory()` berguna untuk test dan ROM sintetis.

## iNES 1.0 dan NES 2.0

Keduanya didukung untuk mapper 0. NES 2.0 memperluas mapper menjadi 12 bit
dan ukuran ROM menjadi 12 bit. Encoding eksponen-multiplier (`0xF00` ke
atas) juga didecode. Metadata NES 2.0 yang belum relevan untuk NROM
(ukuran RAM khusus, system type, miscellaneous ROM) diabaikan dengan aman.
Console type selain NES/Famicom ditolak. Timing NTSC dan PAL diterima;
multi-region dijalankan sebagai NTSC, sedangkan Dendy ditolak dengan error
khusus.

## NROM

- PRG 16 KiB dicerminkan ke `$8000-$BFFF` dan `$C000-$FFFF`.
- PRG 32 KiB dipetakan langsung.
- CHR 8 KiB atau CHR RAM bila header menyatakan 0.
- PRG RAM 8 KiB di `$6000-$7FFF`.
- Tidak ada bank switching.

## Menambah Mapper

1. Buat `mapper_xxx.c` dengan fungsi `cpu_read`, `cpu_write`, `ppu_read`,
   `ppu_write`, dan `destroy`.
2. Buat `mapper_xxx_create()` yang mengembalikan `Mapper *`.
3. Pilih mapper di `ines_loader.c` berdasarkan nomor mapper.
4. Tambahkan file ke `CMakeLists.txt`.
5. Tambahkan test untuk pemetaan dan validasi.

Selama ini hanya mapper 0 yang dipilih; mapper lain ditolak dengan
`ERR_UNSUPPORTED_MAPPER`.
