/*
 * cpu_bus.h - Bus memori CPU NES.
 *
 * CPU tidak pernah mengakses array 64 KiB secara langsung. Semua alamat
 * melewati bus ini, yang mendekode region memori dan meneruskan akses ke
 * RAM, PPU, APU, controller, atau cartridge.
 */
#ifndef TINYNES_BUS_CPU_BUS_H
#define TINYNES_BUS_CPU_BUS_H

#include "apu/apu.h"
#include "cartridge/cartridge.h"
#include "input/controller.h"
#include "ppu/ppu.h"
#include "util/types.h"

typedef struct CPUBus CPUBus;

CPUBus *cpu_bus_create(void);
void cpu_bus_destroy(CPUBus *bus);

/* Koneksi komponen. Bus tidak memiliki (own) komponen tersebut. */
void cpu_bus_connect_ppu(CPUBus *bus, PPU *ppu);
void cpu_bus_connect_apu(CPUBus *bus, APU *apu);
void cpu_bus_connect_cartridge(CPUBus *bus, Cartridge *cartridge);
void cpu_bus_connect_controllers(CPUBus *bus, Controller *controller1, Controller *controller2);

uint8_t cpu_bus_read(CPUBus *bus, uint16_t address);
void cpu_bus_write(CPUBus *bus, uint16_t address, uint8_t value);

void cpu_bus_reset(CPUBus *bus);

/*
 * DMA menambahkan stall ke CPU. Scheduler mengambil nilai ini setelah
 * instruksi yang memicu DMA selesai, lalu memajukan PPU/APU sebanyak itu.
 */
int cpu_bus_take_stall(CPUBus *bus);

/* Mencatat waktu CPU berjalan; dipakai untuk paritas timing OAM DMA. */
void cpu_bus_tick(CPUBus *bus, int cycles);

uint64_t cpu_bus_cycle_count(const CPUBus *bus);

#endif /* TINYNES_BUS_CPU_BUS_H */
