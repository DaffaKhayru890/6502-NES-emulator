/*
 * ppu_sprites.c - Evaluasi OAM dan compositing sprite.
 *
 * OAM berisi 64 sprite x 4 byte (Y, tile, attribute, X). Pada setiap
 * scanline visible, PPU memilih maksimal 8 sprite yang berada di scanline
 * tersebut, mengambil pattern-nya, lalu menggambar 8 piksel.
 *
 * Catatan akurasi: evaluasi dilakukan pada awal scanline (dot 0), bukan
 * siklus 65-256 seperti hardware. Hasil visual dan sprite 0 hit tetap benar;
 * bug sprite overflow hardware tidak ditiru.
 */
#include "ppu_internal.h"

static uint8_t sprite_height(const PPU *ppu)
{
    return ((ppu->ctrl & PPU_CTRL_SPRITE_SIZE) != 0U) ? 16U : 8U;
}

void ppu_sprites_evaluate(PPU *ppu)
{
    uint8_t height = 0;
    uint8_t count = 0;
    int line = 0;

    if (ppu == NULL) {
        return;
    }

    height = sprite_height(ppu);
    line = ppu->scanline;
    ppu->sprite_count = 0;
    ppu->sprite_zero_on_line = false;

    for (uint16_t i = 0; i < 64U; ++i) {
        uint8_t y = ppu->oam[i * 4U];
        int top = (int)y + 1;
        int bottom = top + (int)height;
        uint8_t row = 0;
        uint8_t tile = 0;
        uint8_t attr = 0;
        uint8_t x = 0;
        uint16_t table = 0;
        uint16_t pattern_addr = 0;
        PPUSpriteSlot *slot = NULL;

        if (line < top || line >= bottom) {
            continue;
        }

        if (count >= NES_MAX_SPRITES_PER_LINE) {
            /* Hardware memiliki bug overflow; di sini kita hanya set flag. */
            ppu->status = (uint8_t)(ppu->status | PPU_STATUS_OVERFLOW);
            break;
        }

        tile = ppu->oam[i * 4U + 1U];
        attr = ppu->oam[i * 4U + 2U];
        x = ppu->oam[i * 4U + 3U];
        row = (uint8_t)(line - top);
        if ((attr & 0x80U) != 0U) {
            row = (uint8_t)(height - 1U - row);
        }

        if (height == 16U) {
            table = ((tile & 0x01U) != 0U) ? 0x1000U : 0x0000U;
            tile = (uint8_t)(tile & 0xFEU);
            if (row >= 8U) {
                ++tile;
                row = (uint8_t)(row - 8U);
            }
        } else {
            table = ((ppu->ctrl & PPU_CTRL_SPRITE_TABLE) != 0U) ? 0x1000U : 0x0000U;
        }

        pattern_addr = (uint16_t)(table + ((uint16_t)tile << 4) + row);

        slot = &ppu->sprite_line[count];
        slot->y = y;
        slot->tile = tile;
        slot->attr = attr;
        slot->x = x;
        slot->pattern_low = ppu_memory_read(ppu, pattern_addr);
        slot->pattern_high = ppu_memory_read(ppu, (uint16_t)(pattern_addr + 8U));
        slot->active = true;
        slot->sprite_zero = (i == 0U);

        if (i == 0U) {
            ppu->sprite_zero_on_line = true;
        }

        ++count;
    }

    ppu->sprite_count = count;
}

bool ppu_sprite_pixel(PPU *ppu, int x, bool background_opaque, uint8_t *pixel,
                      uint8_t *palette, bool *behind)
{
    uint8_t i = 0;

    if (ppu == NULL || pixel == NULL || palette == NULL || behind == NULL) {
        return false;
    }

    *pixel = 0;
    *palette = 0;
    *behind = false;

    for (i = 0; i < ppu->sprite_count; ++i) {
        const PPUSpriteSlot *slot = &ppu->sprite_line[i];
        int left = (int)slot->x;
        int column = 0;
        uint8_t bit = 0;
        uint8_t low = 0;
        uint8_t high = 0;
        uint8_t value = 0;

        if (!slot->active || x < left || x >= left + 8) {
            continue;
        }

        column = x - left;
        if ((slot->attr & 0x40U) != 0U) {
            column = 7 - column;
        }
        bit = (uint8_t)(0x80U >> column);

        low = (uint8_t)((slot->pattern_low & bit) != 0U ? 1U : 0U);
        high = (uint8_t)((slot->pattern_high & bit) != 0U ? 2U : 0U);
        value = (uint8_t)(low | high);

        if (value == 0U) {
            continue;
        }

        *pixel = value;
        *palette = (uint8_t)(slot->attr & 0x03U);
        *behind = (slot->attr & 0x20U) != 0U;

        if (slot->sprite_zero && background_opaque && x != 255) {
            ppu->status = (uint8_t)(ppu->status | PPU_STATUS_SPRITE0);
        }

        return true;
    }

    return false;
}
