# Header CPU

`cpu_2a03.h` mendefinisikan:

- callback `CPUReadFn` dan `CPUWriteFn`;
- enum `CPUFlag`;
- struct `CPU` dengan register, koneksi bus, dan statistik;
- API pembuatan, reset, langkah, NMI, IRQ;
- helper flag dan stack.

CPU tidak bergantung pada bus konkret. Semua akses memori melewati callback
dan `context`, sehingga unit test dapat memakai `FakeBus`.
