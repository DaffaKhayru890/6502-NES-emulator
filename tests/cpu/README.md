# Test CPU

Menguji CPU secara terisolasi memakai `FakeBus` 64 KiB.

- `test_cpu_registers.c`: nilai reset, flag, stack, interrupt.
- `test_cpu_addressing.c`: seluruh addressing mode dan bug `JMP ($xxFF)`.
- `test_cpu_instructions.c`: semantik instruksi resmi dan beberapa opcode
  tidak resmi.

Test memuat kode mesin ke `$8000` lalu memanggil `cpu_step()`. Karena
menggunakan `FakeBus`, tidak ada PPU/APU yang terlibat.
