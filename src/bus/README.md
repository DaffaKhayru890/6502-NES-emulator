# Modul Bus

`cpu_bus.c` adalah satu-satunya jalur akses memori CPU.

## Tanggung jawab

- RAM internal 2 KiB dan mirror `$0800-$1FFF`.
- Mirror register PPU `$2008-$3FFF`.
- Penerusan register PPU `$2000-$2007` dan APU `$4000-$4017`.
- Controller `$4016`/`$4017`.
- OAM DMA `$4014`.
- PRG RAM/ROM dan ekspansi cartridge.
- Open bus untuk alamat tanpa perangkat.
- Pencatatan stall DMA.

## Koneksi

Bus tidak membuat komponen. Pemanggil menghubungkan dengan:

```c
cpu_bus_connect_ppu(bus, ppu);
cpu_bus_connect_apu(bus, apu);
cpu_bus_connect_cartridge(bus, cart);
cpu_bus_connect_controllers(bus, &c1, &c2);
```

Pointer boleh `NULL`; bus akan memperlakukannya sebagai open bus. Ini membuat
bus mudah diuji secara terisolasi.

## OAM DMA

Menulis `$4014`:

1. menyalin 256 byte dari halaman `value << 8` ke OAM PPU;
2. mencatat stall 513 siklus (genap) atau 514 siklus (ganjil);
3. `cpu_bus_take_stall()` mengembalikan dan menghapus nilai tersebut.

Scheduler menambahkan stall ke total siklus instruksi sehingga PPU dan APU
ikut maju.
