/*
 * ppu_render.c - Menggabungkan background dan sprite menjadi satu piksel.
 *
 * Warna akhir adalah indeks palette (0-63) yang disimpan di framebuffer.
 * Prioritas:
 * 1. Background transparan + sprite opaque -> sprite.
 * 2. Keduanya opaque -> sprite di depan, kecuali attribute "behind".
 * 3. Background opaque -> background.
 * 4. Tidak ada yang opaque -> warna universal background ($3F00).
 */
#include "ppu_internal.h"

static void background_pixel(PPU *ppu, int x, uint8_t *pixel, uint8_t *palette)
{
    uint16_t pattern_bit = 0;
    uint8_t attr_bit = 0;
    uint8_t low = 0;
    uint8_t high = 0;
    uint8_t attr_low = 0;
    uint8_t attr_high = 0;

    *pixel = 0;
    *palette = 0;

    if ((ppu->mask & PPU_MASK_BG_ENABLE) == 0U) {
        return;
    }

    /* Delapan piksel paling kiri dapat di-clip oleh PPUMASK bit 1. */
    if (x < 8 && (ppu->mask & PPU_MASK_BG_LEFT) == 0U) {
        return;
    }

    pattern_bit = (uint16_t)(0x8000U >> ppu->fine_x);
    attr_bit = (uint8_t)(0x80U >> ppu->fine_x);

    low = (uint8_t)((ppu->bg_shift_lo & pattern_bit) != 0U ? 1U : 0U);
    high = (uint8_t)((ppu->bg_shift_hi & pattern_bit) != 0U ? 2U : 0U);
    *pixel = (uint8_t)(low | high);

    if (*pixel == 0U) {
        return;
    }

    attr_low = (uint8_t)((ppu->bg_attr_lo & ((uint16_t)attr_bit << 8)) != 0U ? 1U : 0U);
    attr_high = (uint8_t)((ppu->bg_attr_hi & ((uint16_t)attr_bit << 8)) != 0U ? 2U : 0U);
    *palette = (uint8_t)(attr_low | attr_high);
}

void ppu_render_pixel(PPU *ppu)
{
    int x = 0;
    uint8_t bg_pixel = 0;
    uint8_t bg_palette = 0;
    uint8_t sprite_pixel = 0;
    uint8_t sprite_palette = 0;
    bool sprite_behind = false;
    bool sprite_opaque = false;
    bool show_sprites = false;
    bool background_opaque = false;
    uint8_t palette_index = 0;

    if (ppu == NULL) {
        return;
    }

    x = ppu->dot - 1;
    background_pixel(ppu, x, &bg_pixel, &bg_palette);
    background_opaque = bg_pixel != 0U;

    show_sprites = (ppu->mask & PPU_MASK_SPRITE_ENABLE) != 0U;
    if (x < 8 && (ppu->mask & PPU_MASK_SPRITE_LEFT) == 0U) {
        show_sprites = false;
    }

    if (show_sprites) {
        sprite_opaque = ppu_sprite_pixel(ppu, x, background_opaque, &sprite_pixel,
                                        &sprite_palette, &sprite_behind);
    }

    if (sprite_opaque && (!background_opaque || !sprite_behind)) {
        palette_index = (uint8_t)(0x10U + (uint8_t)(sprite_palette << 2) + sprite_pixel);
    } else if (background_opaque) {
        palette_index = (uint8_t)((uint8_t)(bg_palette << 2) + bg_pixel);
    } else {
        palette_index = 0;
    }

    /* Bit grayscale memaksa warna menjadi skala abu-abu NES. */
    if ((ppu->mask & PPU_MASK_GRAYSCALE) != 0U) {
        palette_index = (uint8_t)(palette_index & 0x30U);
    }

    ppu->framebuffer[(size_t)ppu->scanline * NES_SCREEN_WIDTH + (size_t)x] =
        ppu_palette_read(ppu, palette_index);
}
