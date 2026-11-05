# test_cpu_addressing.c

**Peran:** Menguji resolusi alamat setiap addressing mode.

## Cakupan

- Immediate, zero page, zero page,X/Y.
- Absolute, absolute,X/Y.
- Indirect (`JMP`) dan bug `$xxFF`.
- Indexed indirect `($xx,X)`.
- Indirect indexed `($xx),Y` dan penalti page crossing.
- Wrap zero page.
