# Timing Emulasi

## Region

TinyNES mendukung dua region timing:

| Region | CPU | PPU:CPU | Scanline | Frame rate |
|---|---|---|---|---|
| NTSC | 1.789.773 Hz | 3,0 | 262 | ~60,0988 Hz |
| PAL | 1.662.607 Hz | 3,2 | 312 | ~50,0070 Hz |

Region dipilih dengan `--region auto|ntsc|pal`. Mode `auto` (default)
mengambil timing dari metadata NES 2.0; ROM iNES 1.0 tidak menyimpan
informasi region sehingga dijalankan sebagai NTSC. Region eksplisit dari
CLI selalu menang atas metadata ROM.

## Hubungan Clock

Hardware menjalankan CPU, PPU, dan APU dari clock yang sama:

- CPU: 1.789.773 Hz (NTSC) atau 1.662.607 Hz (PAL)
- PPU: 3,0x CPU (NTSC) atau 3,2x CPU (PAL)
- APU: 1 langkah per siklus CPU

TinyNES memakai model sederhana: setelah satu instruksi CPU selesai,
scheduler memajukan PPU sejumlah dot sesuai region dan APU satu langkah
untuk setiap siklus yang dihasilkan instruksi tersebut. Tidak ada thread
emulasi terpisah.

```
NTSC: 1 siklus CPU = 15/5 = 3,0 dot PPU = 1 tick APU
PAL : 1 siklus CPU = 16/5 = 3,2 dot PPU = 1 tick APU
```

Rasio PAL yang pecahan disimpan di akumulator integer dengan satuan 1/5
dot. Setiap siklus CPU menambah 16 ke akumulator; selama nilainya >= 5,
satu dot PPU diproses dan akumulator dikurangi 5. Tidak ada pembulatan
yang menumpuk.

## Frame NTSC

Satu frame terdiri dari 262 scanline, masing-masing 341 dot:

| Scanline | Fungsi |
|---|---|
| 0-239 | Area visible, 256 piksel per baris |
| 240 | Post-render, tidak menggambar |
| 241-260 | VBlank |
| 261 | Pre-render |

Total dot per frame genap: `262 x 341 = 89342`. Pada hardware asli, frame
ganjil melewati satu dot pre-render sehingga totalnya 89341. TinyNES juga
melewati dot 340 pada scanline pre-render saat `odd_frame` aktif.

Pada 60,0988 Hz, durasi satu frame sekitar 16,639 ms.

## Frame PAL

PAL tetap memakai 341 dot per scanline dan 240 scanline visible, tetapi
memiliki 312 scanline total:

| Scanline | Fungsi |
|---|---|
| 0-239 | Area visible |
| 240 | Post-render |
| 241-310 | VBlank |
| 311 | Pre-render |

Total dot per frame genap: `312 x 341 = 106392`. Frame rate PAL adalah
`1.662.607 x 3,2 / 106392`, sekitar 50,0070 Hz, dengan durasi sekitar
19,997 ms. VBlank PAL lebih panjang (70 scanline) daripada NTSC (20
scanline), sesuai hardware PAL.

## VBlank dan NMI

- Flag VBlank di-set saat memproses scanline 241 dot 1 pada kedua region.
- Bila bit NMI enable (`$2000` bit 7) aktif, sebuah edge NMI dicatat.
- `ppu_take_nmi()` mengonsumsi edge tersebut sekali.
- Membaca `$2002` menghapus flag VBlank dan mereset latch alamat `$2006`.
- Flag VBlank, sprite 0, dan overflow dibersihkan pada dot 1 scanline
  pre-render (261 NTSC, 311 PAL).

Scheduler memeriksa NMI di antara instruksi. Artinya NMI bisa tertunda
sampai instruksi saat ini selesai (maksimal beberapa siklus), bukan tepat
pada batas siklus.

## Sprite 0 Hit dan Overflow

- Evaluasi sprite dilakukan pada awal setiap scanline visible (dot 0).
- Maksimal 8 sprite diambil; bila ada sprite kesembilan yang cocok,
  flag overflow di-set dan evaluasi berhenti.
- Sprite 0 hit di-set saat sprite 0 opaque bertabrakan dengan background
  opaque, kecuali pada x=255.
- Bug overflow hardware yang rumit tidak ditiru.

## OAM DMA

Menulis `$4014` menyalin 256 byte dari halaman memori ke OAM. Penyalinan
dilakukan seketika di emulator, lalu bus mencatat stall:

- 513 siklus bila operasi dimulai pada siklus genap;
- 514 siklus bila dimulai pada siklus ganjil.

Stall ini dikembalikan oleh `cpu_bus_take_stall()` dan ikut memajukan PPU/APU,
sehingga frame tidak kehilangan waktu. Nilai stall sama untuk NTSC dan PAL
karena dihitung dalam siklus CPU.

## APU Sampling

APU menghasilkan sampel mono 16-bit dengan akumulator pecahan:

```
sample_step = sample_rate / frekuensi_cpu_region
accumulator += sample_step
bila accumulator >= 1: keluarkan sampel, accumulator -= 1
```

Frekuensi CPU adalah 1.789.773 Hz untuk NTSC dan 1.662.607 Hz untuk PAL.
Pada 44.100 Hz, NTSC menghasilkan sekitar 4.928 sampel per 200.000 siklus,
sedangkan PAL sekitar 5.305 sampel.

Frame counter APU juga mengikuti region:

| Sequencer | NTSC | PAL |
|---|---|---|
| 4-step quarter 1 | 7457 | 8313 |
| 4-step half 1 | 14913 | 16639 |
| 4-step quarter 2 | 22371 | 24939 |
| 4-step half 2 | 29828 | 33252 |
| 4-step IRQ | 29829 | 33253 |
| 4-step reset | 29830 | 33254 |
| 5-step terakhir | 37281 | 41565 |
| 5-step reset | 37282 | 41566 |

Tabel periode noise dan DMC juga memiliki varian PAL yang dipilih saat
register ditulis.

Sampel disimpan di ring buffer 8192 entri. Bila penuh, sampel terlama dibuang
agar produsen tidak pernah memblokir. Frontend mengambil sampel setiap frame
dan mengirimkannya ke `SDL_QueueAudio`.

## Pacing Host

Emulasi dan presentasi dipisahkan. Loop utama memakai frame rate dari
`nes_frame_rate()`:

- NTSC: sekitar 60,0988 frame per detik;
- PAL: sekitar 50,0070 frame per detik.

Waktu dihitung dengan `SDL_GetPerformanceCounter()`:

- bila masih lebih awal, tidur sekitar sisa waktu dikurangi 1 ms, lalu
  busy-wait untuk sisa di bawah 1 ms;
- bila tertinggal lebih dari 4 frame (misalnya jendela dipindahkan), jadwal
  di-reset agar tidak mengejar secara berlebihan.

Pacing ini tidak mengubah kecepatan emulasi di dalam satu frame; ia hanya
mengatur kapan frame berikutnya dijalankan.

## Keterbatasan Timing

- CPU tidak memodelkan setiap siklus bus (read/write cycle) secara terpisah.
- Interrupt latency dibulatkan ke batas instruksi.
- PPU tidak memodelkan konflik akses VRAM saat rendering.
- DMC tidak menambah stall DMA.
- Dendy belum didukung; NES 2.0 dengan timing Dendy ditolak.
