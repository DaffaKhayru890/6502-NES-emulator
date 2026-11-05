# test_cpu_instructions.c

**Peran:** Menguji semantik instruksi.

## Cakupan

- LDA/LDX/LDY/STA/STX/STY.
- AND/ORA/EOR, ADC/SBC.
- ASL/LSR/ROL/ROR accumulator dan memori.
- CMP/CPX/CPY.
- INC/DEC memori dan register.
- BIT.
- PHA/PLA/PHP/PLP.
- Branch, JMP, JSR/RTS, BRK/RTI.
- NOP dan opcode tidak resmi LAX/SAX/DCP/SLO.
- Siklus dasar dan penalti page.

## Catatan

Alamat memori memakai halaman `$03xx`; opcode zero page dan absolute
dipilih secara eksplisit agar sesuai alamat yang diuji.
