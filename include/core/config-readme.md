# config.h

**Peran:** Data konfigurasi hasil parsing command line.

## Isi utama

- enum `Region` didefinisikan di `util/types.h` (`REGION_NTSC`,
  `REGION_PAL`, `REGION_AUTO`).
- struct `Config`: path ROM, skala, mute, no_audio, debug, help, region.
- `config_init_defaults()`, `config_parse()`, `config_print_usage()`.

## Catatan

`rom_path` menunjuk ke `argv`; tidak dialokasikan oleh modul ini.
