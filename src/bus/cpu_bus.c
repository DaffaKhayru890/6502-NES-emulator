/*
 * cpu_bus.c - Implementasi peta memori CPU NES.
 *
 * Peta memori NTSC:
 *   $0000-$07FF  RAM internal 2 KiB
 *   $0800-$1FFF  cermin RAM (setiap 2 KiB)
 *   $2000-$2007  register PPU
 *   $2008-$3FFF  cermin register PPU (setiap 8 byte)
 *   $4000-$4013  register APU
 *   $4014        OAM DMA
 *   $4015        status/aktifkan APU
 *   $4016-$4017  strobe controller / APU frame counter
 *   $4018-$401F  fitur tes (open bus)
 *   $4020-$5FFF  ekspansi cartridge (open bus pada NROM)
 *   $6000-$7FFF  PRG RAM cartridge
 *   $8000-$FFFF  PRG ROM cartridge
 */
#include "bus/cpu_bus.h"

#include "util/log.h"

#include <stdlib.h>
#include <string.h>

#define CPU_RAM_MASK 0x07FFU
#define PPU_REGISTER_MASK 0x0007U
#define OAM_DMA_BASE_CYCLES 513

struct CPUBus {
    uint8_t ram[NES_CPU_RAM_SIZE];
    uint8_t open_bus;

    PPU *ppu;
    APU *apu;
    Cartridge *cartridge;
    Controller *controller1;
    Controller *controller2;

    int dma_stall;
    uint64_t cycle_count;
};

CPUBus *cpu_bus_create(void)
{
    CPUBus *bus = (CPUBus *)calloc(1, sizeof(*bus));

    if (bus == NULL) {
        log_error("Alokasi memori CPU Bus gagal.");
    }
    return bus;
}

void cpu_bus_destroy(CPUBus *bus)
{
    free(bus);
}

void cpu_bus_connect_ppu(CPUBus *bus, PPU *ppu)
{
    if (bus != NULL) {
        bus->ppu = ppu;
    }
}

void cpu_bus_connect_apu(CPUBus *bus, APU *apu)
{
    if (bus != NULL) {
        bus->apu = apu;
    }
}

void cpu_bus_connect_cartridge(CPUBus *bus, Cartridge *cartridge)
{
    if (bus != NULL) {
        bus->cartridge = cartridge;
    }
}

void cpu_bus_connect_controllers(CPUBus *bus, Controller *controller1, Controller *controller2)
{
    if (bus == NULL) {
        return;
    }
    bus->controller1 = controller1;
    bus->controller2 = controller2;
}

void cpu_bus_reset(CPUBus *bus)
{
    if (bus == NULL) {
        return;
    }

    memset(bus->ram, 0, sizeof(bus->ram));
    bus->open_bus = 0;
    bus->dma_stall = 0;
}

static uint8_t read_oam_dma_register(CPUBus *bus)
{
    /* $4014 hanya dapat ditulis; pembacaan mengembalikan open bus. */
    return bus->open_bus;
}

static void start_oam_dma(CPUBus *bus, uint8_t page)
{
    uint8_t source[256];
    uint16_t base = (uint16_t)((uint16_t)page << 8);
    size_t i = 0;

    if (bus->ppu == NULL) {
        return;
    }

    /*
     * Salin lewat bus agar mirroring RAM tetap konsisten. DMA nyata membaca
     * satu byte setiap dua siklus; detail itu tidak terlihat oleh game,
     * tetapi total stall tetap dihitung.
     */
    for (i = 0; i < 256U; ++i) {
        source[i] = cpu_bus_read(bus, (uint16_t)(base + (uint16_t)i));
    }

    ppu_oam_dma(bus->ppu, source);

    /* OAM DMA memakan 513 siklus, atau 514 bila dimulai pada siklus ganjil. */
    bus->dma_stall = OAM_DMA_BASE_CYCLES + (int)(bus->cycle_count & 1U);
    log_debug("OAM DMA dari page $%02X, stall %d siklus", page, bus->dma_stall);
}

uint8_t cpu_bus_read(CPUBus *bus, uint16_t address)
{
    uint8_t value = 0;

    if (bus == NULL) {
        return 0;
    }

    if (address <= 0x1FFFU) {
        value = bus->ram[address & CPU_RAM_MASK];
        bus->open_bus = value;
        return value;
    }

    if (address >= 0x2000U && address <= 0x3FFFU) {
        if (bus->ppu != NULL) {
            value = ppu_read_register(bus->ppu, (uint16_t)(address & PPU_REGISTER_MASK));
        }
        bus->open_bus = value;
        return value;
    }

    if (address == 0x4015U) {
        value = (bus->apu != NULL) ? apu_read_status(bus->apu) : 0;
        value = (uint8_t)(value | (bus->open_bus & 0x20U));
        bus->open_bus = value;
        return value;
    }

    if (address == 0x4016U) {
        uint8_t pad = (bus->controller1 != NULL) ? controller_read(bus->controller1) : 0x40U;
        value = (uint8_t)(pad | (bus->open_bus & 0xE0U));
        bus->open_bus = value;
        return value;
    }

    if (address == 0x4017U) {
        uint8_t pad = (bus->controller2 != NULL) ? controller_read(bus->controller2) : 0x40U;
        uint8_t irq = (bus->apu != NULL && apu_frame_irq(bus->apu)) ? 0x40U : 0x00U;
        value = (uint8_t)(pad | irq | (bus->open_bus & 0x20U));
        bus->open_bus = value;
        return value;
    }

    if (address == 0x4014U) {
        return read_oam_dma_register(bus);
    }

    if (address >= 0x4000U && address <= 0x401FU) {
        bus->open_bus = value;
        return value;
    }

    if (address >= 0x6000U) {
        value = cartridge_cpu_read(bus->cartridge, address);
        bus->open_bus = value;
        return value;
    }

    /* $4020-$5FFF: ekspansi cartridge; NROM membiarkannya open bus. */
    return bus->open_bus;
}

void cpu_bus_write(CPUBus *bus, uint16_t address, uint8_t value)
{
    if (bus == NULL) {
        return;
    }

    bus->open_bus = value;

    if (address <= 0x1FFFU) {
        bus->ram[address & CPU_RAM_MASK] = value;
        return;
    }

    if (address >= 0x2000U && address <= 0x3FFFU) {
        if (bus->ppu != NULL) {
            ppu_write_register(bus->ppu, (uint16_t)(address & PPU_REGISTER_MASK), value);
        }
        return;
    }

    if (address == 0x4014U) {
        start_oam_dma(bus, value);
        return;
    }

    if (address == 0x4016U) {
        if (bus->controller1 != NULL) {
            controller_write_strobe(bus->controller1, value);
        }
        if (bus->controller2 != NULL) {
            controller_write_strobe(bus->controller2, value);
        }
        return;
    }

    if (address >= 0x4000U && address <= 0x4013U) {
        if (bus->apu != NULL) {
            apu_write_register(bus->apu, address, value);
        }
        return;
    }

    if (address == 0x4015U) {
        if (bus->apu != NULL) {
            apu_write_register(bus->apu, address, value);
        }
        return;
    }

    if (address == 0x4017U) {
        if (bus->apu != NULL) {
            apu_write_register(bus->apu, address, value);
        }
        return;
    }

    if (address >= 0x4018U && address <= 0x401FU) {
        return;
    }

    if (address >= 0x6000U) {
        cartridge_cpu_write(bus->cartridge, address, value);
    }
}

int cpu_bus_take_stall(CPUBus *bus)
{
    int stall = 0;

    if (bus == NULL) {
        return 0;
    }

    stall = bus->dma_stall;
    bus->dma_stall = 0;
    return stall;
}

void cpu_bus_tick(CPUBus *bus, int cycles)
{
    if (bus == NULL || cycles <= 0) {
        return;
    }
    bus->cycle_count += (uint64_t)cycles;
}

uint64_t cpu_bus_cycle_count(const CPUBus *bus)
{
    return (bus != NULL) ? bus->cycle_count : 0U;
}
