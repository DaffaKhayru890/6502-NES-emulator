/*
 * test_cpu_bus.c - Peta memori CPU, mirroring, controller, dan OAM DMA.
 */
#include "../test.h"
#include "../test_helpers.h"

#include "bus/cpu_bus.h"
#include "core/nes.h"
#include "ppu/ppu_memory.h"

#include <string.h>

void test_cpu_bus_run(void)
{
    uint8_t *rom = NULL;
    size_t rom_size = 0;
    size_t prg = 0;
    Nes *nes = NULL;
    CPUBus *bus = NULL;
    PPU *ppu = NULL;
    ErrorCode error = ERR_OK;

    rom = test_ines_create(1, 0, 0x00, 0x00, false, &rom_size);
    CHECK_TRUE(rom != NULL);
    if (rom == NULL) {
        return;
    }

    prg = test_ines_prg_offset(false);
    rom[prg] = 0xAB;

    nes = nes_create(44100, REGION_NTSC);
    CHECK_TRUE(nes != NULL);
    if (nes == NULL) {
        test_ines_free(rom);
        return;
    }

    error = nes_load_rom_data(nes, rom, rom_size);
    CHECK_EQ_INT(error, ERR_OK);
    bus = nes_bus(nes);
    ppu = nes_ppu(nes);
    CHECK_TRUE(bus != NULL);
    CHECK_TRUE(ppu != NULL);
    test_ines_free(rom);

    /* RAM internal dan mirroring 2 KiB. */
    cpu_bus_write(bus, 0x0000, 0x11);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x0800), 0x11);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x1000), 0x11);
    cpu_bus_write(bus, 0x1FFF, 0x22);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x07FF), 0x22);

    /* Register PPU dimirror setiap 8 byte ($3FF6 -> $2006, $3FF7 -> $2007). */
    cpu_bus_write(bus, 0x3FF6, 0x20);
    cpu_bus_write(bus, 0x3FF6, 0x00);
    cpu_bus_write(bus, 0x3FF7, 0x5A);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2000), 0x5A);

    /* Register APU diteruskan; length counter pulse 1 membuat status bit 0. */
    cpu_bus_write(bus, 0x4000, 0xBF);
    cpu_bus_write(bus, 0x4002, 0xFF);
    cpu_bus_write(bus, 0x4003, 0x08);
    cpu_bus_write(bus, 0x4015, 0x01);
    CHECK_TRUE((cpu_bus_read(bus, 0x4015) & 0x01) != 0);

    /* Controller serial: A, B, Select, Start, lalu arah. */
    nes_set_buttons(nes, 0, (uint8_t)(CONTROLLER_BUTTON_A | CONTROLLER_BUTTON_START));
    cpu_bus_write(bus, 0x4016, 0x01);
    cpu_bus_write(bus, 0x4016, 0x00);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4016) & 0x01, 1);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4016) & 0x01, 0);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4016) & 0x01, 0);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4016) & 0x01, 1);

    nes_set_buttons(nes, 1, (uint8_t)(CONTROLLER_BUTTON_UP | CONTROLLER_BUTTON_RIGHT));
    cpu_bus_write(bus, 0x4016, 0x01);
    cpu_bus_write(bus, 0x4016, 0x00);
    for (int i = 0; i < 4; ++i) {
        (void)cpu_bus_read(bus, 0x4017); /* lewati A, B, Select, Start */
    }
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4017) & 0x01, 1); /* Up */
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4017) & 0x01, 0); /* Down */
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4017) & 0x01, 0); /* Left */
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4017) & 0x01, 1); /* Right */

    /* Cartridge: PRG 16 KiB tercermin di $C000, ROM tidak dapat ditulis. */
    CHECK_EQ_U8(cpu_bus_read(bus, 0x8000), 0xAB);
    CHECK_EQ_U8(cpu_bus_read(bus, 0xC000), 0xAB);
    cpu_bus_write(bus, 0x8000, 0xCD);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x8000), 0xAB);

    /* PRG RAM cartridge. */
    cpu_bus_write(bus, 0x6000, 0x77);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x6000), 0x77);

    /* OAM DMA: 256 byte dari page $02, stall 513/514 siklus. */
    for (int i = 0; i < 256; ++i) {
        cpu_bus_write(bus, (uint16_t)(0x0200 + i), (uint8_t)i);
    }
    cpu_bus_write(bus, 0x4014, 0x02);
    CHECK_EQ_U8(ppu_oam_read(ppu, 0), 0);
    CHECK_EQ_U8(ppu_oam_read(ppu, 255), 255);
    CHECK_TRUE(cpu_bus_take_stall(bus) >= 513);
    CHECK_EQ_INT(cpu_bus_take_stall(bus), 0);

    /* Open bus sederhana di area ekspansi. */
    cpu_bus_write(bus, 0x4018, 0x5A);
    CHECK_EQ_U8(cpu_bus_read(bus, 0x4020), 0x5A);

    nes_destroy(nes);
}
