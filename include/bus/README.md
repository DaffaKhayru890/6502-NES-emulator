# Header Bus

`cpu_bus.h` mendeklarasikan tipe buram `CPUBus`, fungsi koneksi komponen,
baca/tulis memori, reset, pengambilan stall DMA, pencatatan siklus, dan
penghitung siklus.

Bus tidak memiliki komponen yang dihubungkan. Semua pointer boleh `NULL`
untuk mensimulasikan perangkat yang tidak ada.
