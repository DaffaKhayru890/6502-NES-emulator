# Indeks Dokumentasi TinyNES

Dokumentasi ini menjelaskan cara kerja emulator, cara membangunnya, dan
batas akurasi yang perlu diketahui.

| Dokumen | Isi |
|---|---|
| [arsitektur.md](arsitektur.md) | Pembagian modul, aliran data, kepemilikan objek, dan scheduler |
| [membangun.md](membangun.md) | Prasyarat, perintah build, opsi CMake, sanitizer, dan pemecahan masalah |
| [penggunaan.md](penggunaan.md) | Opsi command line, kontrol, dan contoh pemakaian |
| [timing.md](timing.md) | Hubungan clock CPU/PPU/APU, frame NTSC/PAL, NMI, dan pacing host |
| [peta-memori.md](peta-memori.md) | Peta memori CPU dan PPU beserta perilaku mirroring |
| [pengujian.md](pengujian.md) | Struktur test, cakupan, dan cara menambah test |
| [batasan.md](batasan.md) | Daftar jujur fitur yang belum ada atau belum presisi |
| [format-rom.md](format-rom.md) | Format iNES 1.0/NES 2.0 yang diterima dan validasi yang dilakukan |

Dokumentasi per modul ada di `src/<modul>/README.md`, sedangkan dokumentasi
per file ada di `<nama-file>-readme.md` di samping file sumbernya.
