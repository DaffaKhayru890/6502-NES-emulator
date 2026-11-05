/*
 * ppu_background.c - Pipeline fetch background PPU.
 *
 * PPU mengambil data background setiap 8 dot:
 *   dot % 8 == 1 : reload shift register + fetch nametable
 *   dot % 8 == 3 : fetch attribute
 *   dot % 8 == 5 : fetch pattern plane rendah
 *   dot % 8 == 7 : fetch pattern plane tinggi
 *   dot % 8 == 0 : increment coarse X
 * Shift register digeser setiap dot selama area fetch.
 */
#include "ppu_internal.h"

void ppu_background_reset(PPU *ppu)
{
    if (ppu == NULL) {
        return;
    }

    ppu->nt_latch = 0;
    ppu->at_latch = 0;
    ppu->pt_low_latch = 0;
    ppu->pt_high_latch = 0;
    ppu->bg_shift_lo = 0;
    ppu->bg_shift_hi = 0;
    ppu->bg_attr_lo = 0;
    ppu->bg_attr_hi = 0;
}

static void reload_shift_registers(PPU *ppu)
{
    ppu->bg_shift_lo = (uint16_t)((ppu->bg_shift_lo & 0xFF00U) | ppu->pt_low_latch);
    ppu->bg_shift_hi = (uint16_t)((ppu->bg_shift_hi & 0xFF00U) | ppu->pt_high_latch);
    ppu->bg_attr_lo = (uint16_t)((ppu->bg_attr_lo & 0xFF00U) |
                                 ((ppu->at_latch & 0x01U) != 0U ? 0xFFU : 0x00U));
    ppu->bg_attr_hi = (uint16_t)((ppu->bg_attr_hi & 0xFF00U) |
                                 ((ppu->at_latch & 0x02U) != 0U ? 0xFFU : 0x00U));
}

static uint16_t pattern_base(const PPU *ppu)
{
    return ((ppu->ctrl & PPU_CTRL_BG_TABLE) != 0U) ? 0x1000U : 0x0000U;
}

void ppu_background_tick(PPU *ppu)
{
    int dot = 0;
    bool fetch_phase = false;
    bool shift_phase = false;

    if (ppu == NULL) {
        return;
    }

    dot = ppu->dot;
    fetch_phase = ((dot >= 1 && dot <= 256) || (dot >= 321 && dot <= 336));
    shift_phase = ((dot >= 2 && dot <= 257) || (dot >= 322 && dot <= 337));

    if (shift_phase) {
        ppu->bg_shift_lo = (uint16_t)(ppu->bg_shift_lo << 1);
        ppu->bg_shift_hi = (uint16_t)(ppu->bg_shift_hi << 1);
        ppu->bg_attr_lo = (uint16_t)(ppu->bg_attr_lo << 1);
        ppu->bg_attr_hi = (uint16_t)(ppu->bg_attr_hi << 1);
    }

    if (!fetch_phase) {
        return;
    }

    switch (dot & 7) {
    case 1:
        reload_shift_registers(ppu);
        ppu->nt_latch = ppu_memory_read(ppu, (uint16_t)(0x2000U | (ppu->v & 0x0FFFU)));
        break;

    case 3: {
        uint16_t at_addr = (uint16_t)(0x23C0U | (ppu->v & 0x0C00U) |
                                      ((ppu->v >> 4) & 0x0038U) |
                                      ((ppu->v >> 2) & 0x0007U));
        uint8_t attr = ppu_memory_read(ppu, at_addr);

        /* Pilih kuadran 2x2 tile yang benar berdasarkan coarse X/Y. */
        if ((ppu->v & 0x0040U) != 0U) {
            attr = (uint8_t)(attr >> 4);
        }
        if ((ppu->v & 0x0002U) != 0U) {
            attr = (uint8_t)(attr >> 2);
        }
        ppu->at_latch = (uint8_t)(attr & 0x03U);
        break;
    }

    case 5: {
        uint16_t fine_y = (uint16_t)((ppu->v >> 12) & 0x0007U);
        uint16_t addr = (uint16_t)(pattern_base(ppu) + ((uint16_t)ppu->nt_latch << 4) + fine_y);
        ppu->pt_low_latch = ppu_memory_read(ppu, addr);
        break;
    }

    case 7: {
        uint16_t fine_y = (uint16_t)((ppu->v >> 12) & 0x0007U);
        uint16_t addr = (uint16_t)((uint16_t)(pattern_base(ppu) +
                                              ((uint16_t)ppu->nt_latch << 4) + fine_y) +
                                   8U);
        ppu->pt_high_latch = ppu_memory_read(ppu, addr);
        break;
    }

    case 0:
        ppu_increment_x(ppu);
        break;

    default:
        break;
    }
}
