# test_cpu_registers.c

**Peran:** Menguji state register dan interrupt CPU.

## Cakupan

- Nilai awal setelah `cpu_reset()`.
- Pembacaan vektor reset dari `$FFFC`.
- Set/clear flag dan helper `cpu_set_zn`.
- Push/pop stack dan perilaku `SP` wrap.
- NMI dan IRQ: vektor, flag I, dan jumlah siklus.
