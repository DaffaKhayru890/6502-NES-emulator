/*
 * ppu_memory.c - Peta memori video PPU.
 *
 * $0000-$1FFF : pattern table (CHR ROM/RAM dari cartridge)
 * $2000-$2FFF : nametable + attribute table (VRAM internal, dimirror)
 * $3000-$3EFF : cermin $2000-$2EFF
 * $3F00-$3F1F : palette RAM
 */
#include "ppu/ppu_memory.h"

#include "ppu_internal.h"

static size_t nametable_index(PPU *ppu, uint16_t address)
{
    Mirroring mirroring = MIRRORING_HORIZONTAL;
    uint16_t offset = 0;
    uint16_t table = 0;
    size_t bank = 0;

    if (ppu->cartridge != NULL) {
        mirroring = cartridge_mirroring(ppu->cartridge);
    }

    offset = (uint16_t)(address & 0x03FFU);
    table = (uint16_t)((address >> 10) & 0x0003U);

    switch (mirroring) {
    case MIRRORING_VERTICAL:
        bank = (size_t)(table & 0x0001U);
        break;
    case MIRRORING_FOUR_SCREEN:
        bank = (size_t)table;
        break;
    case MIRRORING_HORIZONTAL:
    default:
        bank = (size_t)(table >> 1);
        break;
    }

    return (bank * 1024U) + (size_t)offset;
}

uint8_t ppu_memory_read(PPU *ppu, uint16_t address)
{
    uint16_t addr = (uint16_t)(address & 0x3FFFU);

    if (ppu == NULL) {
        return 0;
    }

    if (addr < 0x2000U) {
        return cartridge_ppu_read(ppu->cartridge, addr);
    }

    if (addr >= 0x3000U && addr < 0x3F00U) {
        addr = (uint16_t)(addr - 0x1000U);
    }

    if (addr < 0x3F00U) {
        return ppu->nametable[nametable_index(ppu, addr)];
    }

    return ppu_palette_read(ppu, (uint8_t)(addr & 0x001FU));
}

void ppu_memory_write(PPU *ppu, uint16_t address, uint8_t value)
{
    uint16_t addr = (uint16_t)(address & 0x3FFFU);

    if (ppu == NULL) {
        return;
    }

    if (addr < 0x2000U) {
        cartridge_ppu_write(ppu->cartridge, addr, value);
        return;
    }

    if (addr >= 0x3000U && addr < 0x3F00U) {
        addr = (uint16_t)(addr - 0x1000U);
    }

    if (addr < 0x3F00U) {
        ppu->nametable[nametable_index(ppu, addr)] = value;
        return;
    }

    ppu_palette_write(ppu, (uint8_t)(addr & 0x001FU), value);
}

uint8_t ppu_oam_read(PPU *ppu, uint8_t index)
{
    return (ppu != NULL) ? ppu->oam[index] : 0;
}

void ppu_oam_write(PPU *ppu, uint8_t index, uint8_t value)
{
    if (ppu != NULL) {
        ppu->oam[index] = value;
    }
}
