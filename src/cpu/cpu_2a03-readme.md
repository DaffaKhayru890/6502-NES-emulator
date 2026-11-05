# cpu_2a03.c

**Peran:** Siklus hidup dan eksekusi CPU RP2A03.

## Isi utama

- `cpu_create()` / `cpu_destroy()`.
- `cpu_reset()`: `PC` dari vektor `$FFFC`, `SP = $FD`, flag I aktif.
- `cpu_step()`: fetch opcode, ambil `OpcodeInfo`, resolusi alamat, eksekusi,
  hitung siklus.
- `cpu_nmi()`: vektor `$FFFA`, selalu diterima.
- `cpu_irq()`: vektor `$FFFE`, diabaikan bila flag I aktif.

## Siklus

Total = siklus dasar + penalti page crossing + `extra_cycles`. Stall DMA
tidak dihitung di sini; bus yang mencatatnya dan scheduler menambahkannya.

## Catatan

File ini tidak berisi semantik instruksi; semuanya ada di
`cpu_instructions.c`.
