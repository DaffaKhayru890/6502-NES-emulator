/*
 * ppu_registers.c - Perilaku delapan register PPU.
 *
 * Register ini adalah satu-satunya jendela CPU ke PPU. Beberapa register
 * memiliki efek samping penting:
 * - Membaca $2002 menghapus flag VBlank dan mereset latch alamat ($2006).
 * - Menulis $2006 dua kali mengatur alamat VRAM (v) sekaligus.
 * - Membaca $2007 memakai buffer internal satu byte.
 */
#include "ppu/ppu_registers.h"

#include "ppu_internal.h"

static void increment_vram_address(PPU *ppu)
{
    uint16_t step = ((ppu->ctrl & PPU_CTRL_INCREMENT32) != 0U) ? 32U : 1U;

    ppu->v = (uint16_t)((ppu->v + step) & 0x7FFFU);
}

uint8_t ppu_read_register(PPU *ppu, uint16_t address)
{
    uint8_t value = 0;

    if (ppu == NULL) {
        return 0;
    }

    switch (address & 0x0007U) {
    case PPU_REG_STATUS:
        value = (uint8_t)(ppu->status | (ppu->io_latch & 0x1FU));
        ppu->status = (uint8_t)(ppu->status & (uint8_t)~PPU_STATUS_VBLANK);
        ppu->w = false;
        ppu->io_latch = value;
        return value;

    case PPU_REG_OAMDATA:
        return ppu->oam[ppu->oam_addr];

    case PPU_REG_DATA: {
        uint16_t addr = (uint16_t)(ppu->v & 0x3FFFU);

        if (addr < 0x3F00U) {
            value = ppu->read_buffer;
            ppu->read_buffer = ppu_memory_read(ppu, addr);
        } else {
            value = (uint8_t)(ppu_palette_read(ppu, (uint8_t)(addr & 0x001FU)) |
                              (ppu->io_latch & 0xC0U));
            ppu->read_buffer = ppu_memory_read(ppu, (uint16_t)(addr - 0x1000U));
        }

        increment_vram_address(ppu);
        ppu->io_latch = value;
        return value;
    }

    default:
        /* $2000, $2001, $2003, $2005, $2006: open bus. */
        return ppu->io_latch;
    }
}

void ppu_write_register(PPU *ppu, uint16_t address, uint8_t value)
{
    if (ppu == NULL) {
        return;
    }

    ppu->io_latch = value;

    switch (address & 0x0007U) {
    case PPU_REG_CTRL:
        ppu->ctrl = value;
        ppu->t = (uint16_t)((ppu->t & (uint16_t)~PPU_NAMETABLE_MASK) |
                            ((uint16_t)(value & PPU_CTRL_NAMETABLE_MASK) << 10));
        if ((value & PPU_CTRL_NMI_ENABLE) != 0U &&
            (ppu->status & PPU_STATUS_VBLANK) != 0U) {
            ppu->nmi_pending = true;
        }
        break;

    case PPU_REG_MASK:
        ppu->mask = value;
        break;

    case PPU_REG_STATUS:
        /* Read-only. */
        break;

    case PPU_REG_OAMADDR:
        ppu->oam_addr = value;
        break;

    case PPU_REG_OAMDATA:
        ppu->oam[ppu->oam_addr] = value;
        ppu->oam_addr = (uint8_t)(ppu->oam_addr + 1U);
        break;

    case PPU_REG_SCROLL:
        if (!ppu->w) {
            ppu->fine_x = (uint8_t)(value & 0x07U);
            ppu->t = (uint16_t)((ppu->t & (uint16_t)~0x001FU) | (uint16_t)(value >> 3));
            ppu->w = true;
        } else {
            ppu->t = (uint16_t)((ppu->t & (uint16_t)~0x73E0U) |
                                ((uint16_t)(value & 0x07U) << 12) |
                                ((uint16_t)(value & 0xF8U) << 2));
            ppu->w = false;
        }
        break;

    case PPU_REG_ADDR:
        if (!ppu->w) {
            ppu->t = (uint16_t)((ppu->t & 0x00FFU) | ((uint16_t)(value & 0x3FU) << 8));
            ppu->w = true;
        } else {
            ppu->t = (uint16_t)((ppu->t & 0xFF00U) | value);
            ppu->v = ppu->t;
            ppu->w = false;
        }
        break;

    case PPU_REG_DATA:
        ppu_memory_write(ppu, ppu->v, value);
        increment_vram_address(ppu);
        break;

    default:
        break;
    }
}
