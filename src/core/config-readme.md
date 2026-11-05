# config.c

**Peran:** Parsing command line TinyNES.

## Opsi

`--scale N`, `--mute`, `--no-audio`, `--debug`, `--region auto|ntsc|pal`,
`--help`/`-h`. Argumen non-opsi pertama menjadi `rom_path`; ROM kedua
ditolak.

## Perilaku

- `config_init_defaults()` mengisi skala 3, audio aktif, region otomatis.
- `parse_scale()` memvalidasi bilangan bulat 1..10.
- `--region auto` memilih timing dari metadata NES 2.0; iNES 1.0 jatuh ke
  NTSC.
- `--region ntsc` dan `--region pal` memaksa region dan mengalahkan metadata
  ROM.
- `config_print_usage()` mencetak bantuan ke stdout.

## Catatan

`Config` hanya menyimpan pointer ke `argv`; ia tidak menyalin string.
