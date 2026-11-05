# Batasan dan Akurasi

Dokumen ini sengaja jujur. TinyNES adalah emulator edukasi, bukan pengganti
emulator cycle-accurate. Bagian berikut menjelaskan apa yang belum ada atau
belum presisi, agar ekspektasi tetap realistis.

## Cartridge dan Mapper

- Hanya **mapper 0 (NROM)**.
- Mendukung **iNES 1.0** dan **NES 2.0** untuk mapper 0.
- NES 2.0: mapper 12-bit dan ukuran ROM 12-bit dibaca, termasuk encoding
  eksponen-multiplier; submapper disimpan tetapi belum memengaruhi perilaku.
- Console type selain NES/Famicom standar ditolak.
- Timing NTSC dan PAL didukung. Multi-region dijalankan sebagai NTSC;
  Dendy belum didukung dan ditolak.
- Ukuran PRG RAM, CHR RAM, system type, dan miscellaneous ROM dari NES 2.0
  belum diterapkan; PRG RAM dan CHR RAM tetap 8 KiB.
- PRG ROM 16 KiB (dicerminkan) atau 32 KiB.
- CHR ROM 8 KiB atau CHR RAM 8 KiB.
- Trainer 512 byte dilewati, tetapi tidak dipetakan ke memori khusus.
- Flag battery dikenali, tetapi isi PRG RAM **belum disimpan** ke file.
- Four-screen mirroring dikenali, tetapi NROM tidak menyediakan VRAM
  tambahan; perilakunya terbatas pada VRAM internal.
- Tidak ada bank switching, IRQ mapper, atau ekspansi audio cartridge.

Game yang memakai mapper MMC1, UxROM, CNROM, MMC3, dan sejenisnya tidak akan
berjalan.

## CPU

- Seluruh opcode resmi 6502 diimplementasikan.
- Beberapa opcode tidak resmi umum tersedia (LAX, SAX, DCP, SLO, dan
  sejenisnya yang ada di tabel).
- Opcode "tidak stabil" (misalnya SHA, SHX, SHY, TAS, LAS, dan varian
  KIL) tidak dijamin sama dengan hardware asli; opcode yang tidak dikenal
  akan dilaporkan dan CPU dapat berhenti (`jammed`).
- Mode desimal tidak berfungsi, sesuai RP2A03.
- Eksekusi dilakukan per instruksi; siklus bus individual tidak dimodelkan.
- Interrupt diperiksa di antara instruksi, bukan pada batas siklus.
- Penalti page crossing diterapkan untuk instruksi baca yang relevan.
- Perilaku open bus CPU disederhanakan.

## PPU

- Model dot-driven: NTSC 341x262 (3,0 dot per siklus CPU), PAL 341x312
  (3,2 dot per siklus CPU).
- Sprite dievaluasi seluruhnya pada dot 0 setiap scanline, bukan bertahap
  pada siklus 65-256 seperti hardware.
- Bug sprite overflow hardware tidak ditiru; hanya flag overflow sederhana.
- Sprite 0 hit dihitung saat compositing piksel, bukan sebagai efek samping
  pembacaan pattern.
- Tidak ada konflik akses VRAM/nametable antara CPU dan rendering.
- Tidak ada open bus PPU, decay, atau perilaku `$2002` yang sangat detail.
- Emphasize warna (bit 5-7 `$2001`) tidak diterapkan; grayscale diterapkan
  sebagai mask indeks palette.
- Framebuffer selalu 256x240; cropping overscan tidak dilakukan.
- Tabel warna RGB yang sama dipakai untuk NTSC dan PAL; perbedaan hue khas
  PAL belum dimodelkan.
- Tidak ada dukungan khusus untuk PPU revision tertentu.

## APU

- Lima kanal diimplementasikan: dua pulse, triangle, noise, DMC.
- Envelope, sweep, length counter, linear counter, dan frame sequencer ada.
- Mixer memakai rumus nonlinear yang umum, bukan replika analog.
- Output mono 16-bit; tidak ada stereo atau panning.
- Resampling sederhana dengan akumulator; belum ada filter low-pass.
- DMC memutar sampel dan menghasilkan IRQ, tetapi **stall DMA CPU belum
  ditambahkan**.
- Perilaku detail counter/timer pada reset dan write berturut-turut dapat
  berbeda dari hardware.
- Kanal delta dan efek non-linear lain tidak dipisahkan per kanal.

## Timing dan Video

- NTSC dan PAL didukung; Dendy belum.
- Satu dot pre-render dilewati pada frame ganjil, tetapi model tetap
  disederhanakan.
- Pacing host terpisah dari emulasi; frame tidak dijalankan dengan
  akumulasi siklus sempurna.
- Region PAL memakai frame rate sekitar 50,007 Hz dan VBlank lebih panjang.
- Tidak ada frame skipping otomatis.
- Tidak ada save state, rewind, atau movie playback.

## Frontend

- SDL2 saja; tidak ada backend lain.
- Tidak ada pengaturan runtime (menu, keybinding, volume).
- Tidak ada penyimpanan konfigurasi.
- Kontrol keyboard dan game controller bersifat tetap.
- Audio queue dapat bertambah bila frame host lebih lambat dari emulasi;
  tidak ada manajemen latency adaptif.
- Tidak ada screenshot atau recording bawaan.

## Pengujian

- Test mencakup unit dan integrasi dasar, bukan kepatuhan penuh terhadap
  semua test ROM perangkat keras.
- Tidak ada ROM komersial atau test ROM eksternal dalam repositori.
- Belum ada perbandingan frame-by-frame dengan emulator referensi.

## Ringkasan Kompatibilitas

| Kategori | Status |
|---|---|
| Game NROM sederhana | Diharapkan berjalan |
| Game mapper lain | Tidak didukung |
| Game PAL | Diharapkan berjalan (timing PAL) |
| Game Dendy | Tidak didukung |
| Homebrew NROM | Diharapkan berjalan |
| Test ROM CPU resmi | Sebagian besar didukung |
| Test ROM PPU tingkat lanjut | Belum divalidasi menyeluruh; detail timing dapat berbeda |
| Test ROM APU tingkat lanjut | Belum divalidasi menyeluruh; detail timer/DMA dapat berbeda |
