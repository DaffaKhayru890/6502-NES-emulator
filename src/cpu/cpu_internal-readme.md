# cpu_internal.h

**Peran:** Deklarasi internal modul CPU.

## Isi utama

- enum `AddressingMode` untuk seluruh mode 6502.
- struct `OpcodeInfo` (nama, mode, fungsi eksekusi, siklus, page penalty).
- deklarasi `cpu_resolve_address`, `cpu_bus_read8`, `cpu_bus_write8`,
  `cpu_read16`, dan tabel opcode.

## Catatan

Header ini berada di `src/cpu/`, bukan `include/`, karena bukan API publik.
