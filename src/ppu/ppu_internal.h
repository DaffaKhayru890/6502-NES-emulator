/*
 * ppu_internal.h - State lengkap PPU dan fungsi antar-file internal.
 *
 * Header ini tidak dipublikasikan. Struktur dibuat terbuka di antara file
 * ppu_*.c agar pipeline background, sprite, dan register dapat bekerja pada
 * objek yang sama tanpa banyak getter/setter.
 */
#ifndef TINYNES_PPU_INTERNAL_H
#define TINYNES_PPU_INTERNAL_H

#include "ppu/ppu.h"
#include "ppu/ppu_memory.h"
#include "ppu/ppu_registers.h"

typedef struct PPUSpriteSlot {
    uint8_t y;
    uint8_t tile;
    uint8_t attr;
    uint8_t x;
    uint8_t pattern_low;
    uint8_t pattern_high;
    bool active;
    bool sprite_zero;
} PPUSpriteSlot;

struct PPU {
    Cartridge *cartridge;

    /* Register yang ditulis CPU. */
    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint8_t oam_addr;

    uint8_t oam[NES_OAM_SIZE];
    uint8_t palette[NES_PALETTE_SIZE];
    uint8_t nametable[4096]; /* Cukup untuk four-screen. */
    uint8_t read_buffer;
    uint8_t io_latch;

    /* Register scrolling internal. */
    uint16_t v;
    uint16_t t;
    uint8_t fine_x;
    bool w;

    /* Region timing: NTSC 262 scanline, PAL 312 scanline. */
    Region region;

    /* Timing. */
    int scanline; /* 0-239 visible, 240 post-render, VBlank, lalu pre-render. */
    int dot;      /* 0-340 */
    uint64_t frame_count;
    bool odd_frame;
    bool nmi_pending;
    bool frame_complete;

    /* Pipeline background. */
    uint8_t nt_latch;
    uint8_t at_latch;
    uint8_t pt_low_latch;
    uint8_t pt_high_latch;
    uint16_t bg_shift_lo;
    uint16_t bg_shift_hi;
    uint16_t bg_attr_lo;
    uint16_t bg_attr_hi;

    /* Sprite hasil evaluasi untuk scanline saat ini. */
    PPUSpriteSlot sprite_line[NES_MAX_SPRITES_PER_LINE];
    uint8_t sprite_count;
    bool sprite_zero_on_line;

    uint8_t framebuffer[NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT];
};

/* ppu_background.c */
void ppu_background_reset(PPU *ppu);
void ppu_background_tick(PPU *ppu);

/* ppu_render.c */
void ppu_render_pixel(PPU *ppu);

/* ppu_sprites.c */
void ppu_sprites_evaluate(PPU *ppu);
bool ppu_sprite_pixel(PPU *ppu, int x, bool background_opaque, uint8_t *pixel,
                      uint8_t *palette, bool *behind);

/* ppu.c: helper scroll yang dipakai pipeline. */
void ppu_increment_x(PPU *ppu);
void ppu_increment_y(PPU *ppu);
void ppu_copy_x(PPU *ppu);
void ppu_copy_y(PPU *ppu);

#endif /* TINYNES_PPU_INTERNAL_H */
