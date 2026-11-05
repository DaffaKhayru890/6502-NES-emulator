# cpu_instructions.c

**Peran:** Semantik seluruh opcode dan tabel dispatch.

## Isi utama

- Helper aritmetika: `do_adc`, `do_compare`, `do_asl`, `do_lsr`, `do_rol`,
  `do_ror`.
- Fungsi `op_*` untuk setiap instruksi resmi.
- Opcode tidak resmi umum: LAX, SAX, DCP, SLO, dan varian lain yang ada di
  tabel.
- `g_opcode_table[256]`: `OpcodeInfo` berisi nama, mode, fungsi, siklus, dan
  penalti page crossing.

## Desain

Decoding dan eksekusi dipisah. `cpu_step()` hanya memanggil tabel; fungsi
`op_*` menerima alamat efektif yang sudah dihitung.

## Catatan

Opcode tidak dikenal memakai handler yang mencatat peringatan sekali dan
dapat membuat CPU `jammed`.
