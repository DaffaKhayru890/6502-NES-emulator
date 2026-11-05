# Modul CPU

Implementasi Ricoh RP2A03 (varian 6502 tanpa mode desimal).

## File

| File | Peran |
|---|---|
| `cpu_2a03.c` | Siklus hidup, reset, `cpu_step`, NMI, IRQ |
| `cpu_addressing.c` | Fetch byte/word dan resolusi 13 addressing mode |
| `cpu_instructions.c` | Tabel 256 opcode dan semantik instruksi |
| `cpu_flags.c` | Helper flag, zero/negative, stack |
| `cpu_internal.h` | `OpcodeInfo`, `AddressingMode`, dan deklarasi internal |

## Desain Tabel

Setiap opcode dipetakan ke `OpcodeInfo`:

```c
typedef struct OpcodeInfo {
    const char *name;
    AddressingMode mode;
    void (*execute)(CPU *cpu, uint16_t address);
    uint8_t cycles;
    bool page_penalty;
} OpcodeInfo;
```

`cpu_step()`:

1. ambil opcode di `PC`;
2. ambil `OpcodeInfo`;
3. minta `cpu_resolve_address()` menghitung alamat efektif;
4. panggil fungsi eksekusi;
5. jumlahkan siklus dasar, penalti page crossing, dan `extra_cycles`;
6. kembalikan total siklus.

Pendekatan ini menghindari `switch` raksasa dan memisahkan decoding dari
semantik.

## Interrupt

- Reset: `PC = [$FFFC]`, `SP = $FD`, flag I aktif.
- NMI: vektor `$FFFA`, selalu diterima.
- IRQ/BRK: vektor `$FFFE`, IRQ diabaikan bila flag I aktif.
- NMI dan IRQ mendorong `P` dengan bit U set dan bit B clear.

## Opcode Tidak Resmi

Beberapa opcode tidak resmi yang umum dipakai game tersedia (LAX, SAX, DCP,
SLO, dan lainnya sesuai tabel). Opcode tidak dikenal dicatat sekali dan
CPU dapat berhenti dengan flag `jammed`.
