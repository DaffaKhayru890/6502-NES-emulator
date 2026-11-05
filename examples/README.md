# Contoh

Direktori ini berisi alat bantu, bukan bagian dari emulator.

## ROM sintetis

`make_synthetic_rom.py` membuat file NROM minimal yang sudah divalidasi
dapat dijalankan TinyNES. ROM ini tidak menampilkan gambar game; tujuannya
menguji jalur pemuatan, scheduler, PPU, dan frontend tanpa ROM eksternal.
Header iNES 1.0 dipakai secara default; tambahkan `--nes2` untuk
menghasilkan header NES 2.0, atau `--pal` untuk NES 2.0 dengan timing PAL.

```sh
python3 examples/make_synthetic_rom.py contoh.nes
python3 examples/make_synthetic_rom.py --nes2 contoh-nes2.nes
python3 examples/make_synthetic_rom.py --pal contoh-pal.nes
./build/tinynes --no-audio --debug contoh.nes
./build/tinynes --no-audio --debug contoh-nes2.nes
./build/tinynes --region pal --no-audio --debug contoh-pal.nes
```

Dengan `--debug`, log akan menampilkan frame dan PC tetap di `$8000`:

```
[INFO ] Frame 60 | PC=$8000 A=$00 X=$00 Y=$00 SP=$FD P=$24 | PPU ... | mapper=0
```

Tekan `Escape` atau tutup jendela untuk keluar. Di lingkungan tanpa layar,
gunakan driver dummy SDL:

```sh
SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software \
  timeout 3 ./build/tinynes --no-audio --debug contoh.nes
```

## Catatan

- Skrip ini memakai Python 3 hanya sebagai alat bantu; emulator tetap
  ditulis dalam C17.
- Tidak ada ROM komersial di repositori ini.
- File `.nes` diabaikan oleh Git melalui `.gitignore`.
