# Penggunaan

## Sintaks

```sh
tinynes [opsi] <rom.nes>
```

Hanya satu ROM yang boleh diberikan. Argumen pertama yang bukan opsi
dianggap sebagai path ROM.

## Opsi

| Opsi | Nilai | Default | Keterangan |
|---|---|---|---|
| `--scale N` | 1..10 | 3 | Faktor pembesaran integer |
| `--mute` | - | mati | Suara tidak dikirim ke device, APU tetap berjalan |
| `--no-audio` | - | mati | Device audio tidak dibuka sama sekali |
| `--debug` | - | mati | Statistik CPU/PPU dicetak setiap 60 frame |
| `--region auto` | `auto` | `auto` | Pilih timing dari metadata ROM; iNES 1.0 menjadi NTSC |
| `--region ntsc` | `ntsc` | - | Paksa timing NTSC |
| `--region pal` | `pal` | - | Paksa timing PAL |
| `--help`, `-h` | - | - | Tampilkan bantuan lalu keluar |

Contoh:

```sh
./build/tinynes game.nes
./build/tinynes --scale 5 --mute game.nes
./build/tinynes --debug game.nes
./build/tinynes --no-audio game.nes
./build/tinynes --region pal game-pal.nes
```

Tanpa `--region`, TinyNES memakai `auto`: ROM NES 2.0 bertiming PAL
dijalankan dengan timing PAL, sedangkan iNES 1.0 dijalankan sebagai NTSC.
Region eksplisit selalu menang atas metadata ROM.

`--debug` mencetak baris seperti:

```
[INFO ] Frame 60 | PC=$8123 A=$00 X=$05 Y=$10 SP=$FD P=$24 | PPU 241:1 | mapper=0
```

Baris ini berguna untuk memastikan CPU tidak terjebak dan PPU mencapai
VBlank.

## Kontrol Keyboard

| Aksi | Tombol |
|---|---|
| A | `Z` |
| B | `X` |
| Select | `Right Shift` |
| Start | `Enter` |
| Atas | Panah atas |
| Bawah | Panah bawah |
| Kiri | Panah kiri |
| Kanan | Panah kanan |
| Keluar | `Escape` |

## Game Controller

SDL2 GameController yang terhubung saat aplikasi berjalan akan dibuka
otomatis. Controller pertama mengisi port 1, controller kedua port 2.
Pemetaan tombol mengikuti layout standar SDL2:

- A/Cross -> A
- B/Circle -> B
- Back/Select -> Select
- Start -> Start
- D-pad dan stick kiri -> arah

## Jendela

- Ukuran internal 256x240.
- Penskalaan integer dengan letterbox; rasio piksel tidak di-stretch.
- Menutup jendela sama dengan meminta keluar.
- Tidak ada vsync; pacing dilakukan manual mengikuti region (sekitar
  60,0988 Hz NTSC atau 50,007 Hz PAL).

## Format ROM

TinyNES menerima file `.nes` dengan:

- magic `NES\x1A`;
- format iNES 1.0 atau NES 2.0;
- mapper 0 (NROM), termasuk mapper 12-bit NES 2.0;
- PRG ROM 16 KiB atau 32 KiB;
- CHR ROM 8 KiB atau CHR RAM bila jumlah CHR = 0;
- mirroring horizontal/vertical/four-screen;
- trainer 512 byte (opsional, dilewati dengan benar);
- flag battery (dicatat, belum menyimpan isi PRG RAM ke disk);
- NES 2.0 dengan console type NES/Famicom dan timing NTSC, PAL, atau
  multi-region (multi-region dijalankan sebagai NTSC).

ROM dengan mapper lain, console type Vs./PlayChoice, atau timing Dendy
akan ditolak dengan pesan yang jelas. Detail field NES 2.0 ada di
[format-rom.md](format-rom.md).

## Di Mana Mendapatkan ROM?

Gunakan ROM domain publik atau homebrew yang lisensinya mengizinkan
distribusi. Proyek ini tidak menyertakan dan tidak mendukung pembajakan ROM
komersial.
