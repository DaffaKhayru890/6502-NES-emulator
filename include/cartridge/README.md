# Header Cartridge

- `cartridge.h`: enum `Mirroring`, tipe buram `Cartridge`, pemuatan dari file
  atau buffer, akses CPU/PPU, dan metadata.
- `mapper.h`: `MapperOps`, struct `Mapper`, dan pembungkus akses.
- `mapper_nrom.h`: pembuat mapper 0.

Mapper memakai pola polimorfisme C: `MapperOps` berisi pointer fungsi dan
`context` menunjuk state implementasi. Untuk NROM, context adalah
`Cartridge` itu sendiri sehingga tidak ada alokasi tambahan.
