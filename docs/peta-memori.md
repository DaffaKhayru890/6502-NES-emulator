# Peta Memori

## Peta Memori CPU

| Rentang | Ukuran | Isi |
|---|---|---|
| `$0000-$07FF` | 2 KiB | RAM internal |
| `$0800-$1FFF` | 6 KiB | Mirror RAM setiap 2 KiB |
| `$2000-$2007` | 8 byte | Register PPU |
| `$2008-$3FFF` | - | Mirror register PPU setiap 8 byte |
| `$4000-$4013` | - | Register APU |
| `$4014` | 1 byte | OAM DMA |
| `$4015` | 1 byte | Status/aktifkan APU |
| `$4016` | 1 byte | Strobe controller / baca port 1 |
| `$4017` | 1 byte | Baca port 2 / frame counter APU |
| `$4018-$401F` | - | Fitur tes, diperlakukan open bus |
| `$4020-$5FFF` | - | Ekspansi cartridge, open bus pada NROM |
| `$6000-$7FFF` | 8 KiB | PRG RAM cartridge |
| `$8000-$FFFF` | 32 KiB | PRG ROM cartridge |

Alamat RAM dimask dengan `0x07FF`. Alamat register PPU dimask dengan
`0x2007` setelah mengurangi `0x2000`, sehingga mirror otomatis tertangani.

### Open Bus

Nilai yang dikembalikan untuk alamat tanpa perangkat adalah byte terakhir
yang pernah melintasi bus. Ini penting karena beberapa game membacanya
sebagai bagian dari logika.

### OAM DMA

Menulis `$4014` menyalin 256 byte dari `value << 8` ke OAM PPU. Bus mencatat
stall 513/514 siklus yang diambil scheduler setelah instruksi selesai.

## Peta Memori PPU

| Rentang | Isi |
|---|---|
| `$0000-$1FFF` | Pattern table: CHR ROM/RAM cartridge |
| `$2000-$2FFF` | Nametable (1 KiB per tabel) + attribute table |
| `$3000-$3EFF` | Mirror `$2000-$2EFF` |
| `$3F00-$3F1F` | Palette RAM |

### Mirroring Nametable

Cartridge menentukan mirroring:

- **Horizontal**: tabel 0 dan 1 memakai bank yang sama; tabel 2 dan 3
  memakai bank kedua.
- **Vertical**: tabel 0 dan 2 memakai bank pertama; tabel 1 dan 3 memakai
  bank kedua.
- **Four-screen**: empat tabel independen (memerlukan VRAM tambahan; NROM
  umumnya tidak memakainya).

Implementasi:

```
horizontal: bank = table >> 1
vertical  : bank = table & 1
```

`$3000-$3EFF` dipetakan kembali ke `$2000-$2EFF` dengan mengurangi `0x1000`.

### Palette RAM

- 32 byte, masing-masing indeks warna 6 bit.
- Alamat `$3F10`, `$3F14`, `$3F18`, dan `$3F1C` adalah mirror dari
  `$3F00`, `$3F04`, `$3F08`, dan `$3F0C`.
- Warna 0 setiap palette hanya dipakai untuk transparansi; warna latar
  universal tetap `$3F00`.
- Framebuffer menyimpan indeks palette, bukan RGB.

## Pattern Table dan Sprite

- Background memakai pattern table yang dipilih `$2000` bit 4.
- Sprite 8x8 memakai pattern table `$2000` bit 3.
- Sprite 8x16 memakai bit 0 tile untuk memilih pattern table, dan tile
  dipaksa genap.
- Setiap tile 8x8 menempati 16 byte: 8 byte plane rendah, 8 byte plane
  tinggi.
- Attribute sprite: bit 0-1 palette, bit 5 prioritas di belakang background,
  bit 6 flip horizontal, bit 7 flip vertikal.

## Register PPU

| Alamat | Nama | Efek penting |
|---|---|---|
| `$2000` | PPUCTRL | NMI enable, ukuran sprite, pemilihan pattern/nametable, increment VRAM |
| `$2001` | PPUMASK | Enable background/sprite, clipping kiri, grayscale |
| `$2002` | PPUSTATUS | VBlank, sprite 0, overflow; membaca menghapus VBlank |
| `$2003` | OAMADDR | Alamat OAM untuk `$2004` |
| `$2004` | OAMDATA | Baca/tulis OAM |
| `$2005` | PPUSCROLL | Scroll X/Y, ditulis dua kali |
| `$2006` | PPUADDR | Alamat VRAM, ditulis dua kali |
| `$2007` | PPUDATA | Baca/tulis VRAM dengan buffer satu byte |

## Register APU

| Alamat | Fungsi |
|---|---|
| `$4000-$4003` | Pulse 1 |
| `$4004-$4007` | Pulse 2 |
| `$4008-$400B` | Triangle |
| `$400C-$400F` | Noise |
| `$4010-$4013` | DMC |
| `$4015` | Status/aktifkan kanal |
| `$4017` | Frame counter |

## Register Controller

`$4016` adalah strobe bersama. Menulis bit 0 = 1 menahan shift register;
menulis 0 memulai penggeseran. Setiap pembacaan `$4016` (port 1) atau
`$4017` (port 2) mengembalikan satu bit, dimulai dari A, lalu B, Select,
Start, Up, Down, Left, Right. Setelah 8 pembacaan, bit berikutnya bernilai 1.
