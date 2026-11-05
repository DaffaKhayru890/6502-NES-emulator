/*
 * ppu.c - Siklus hidup, timing, dan NMI PPU.
 *
 * Model dot-driven ini mendukung dua region:
 *   NTSC : 262 scanline x 341 dot, pre-render di scanline 261
 *   PAL  : 312 scanline x 341 dot, pre-render di scanline 311
 *
 * Pada kedua region, area visible tetap scanline 0-239, post-render 240,
 * dan VBlank dimulai pada scanline 241. Scheduler memajukan PPU dengan
 * rasio 3,0 dot/siklus (NTSC) atau 3,2 dot/siklus (PAL).
 */
#include "ppu/ppu.h"

#include "ppu_internal.h"
#include "util/log.h"

#include <stdlib.h>
#include <string.h>

PPU *ppu_create(Cartridge *cartridge, Region region)
{
    PPU *ppu = (PPU *)calloc(1, sizeof(*ppu));

    if (ppu == NULL) {
        log_error("Alokasi memori untuk PPU gagal.");
        return NULL;
    }

    ppu->cartridge = cartridge;
    ppu->region = (region == REGION_AUTO) ? REGION_NTSC : region;
    ppu_reset(ppu);
    return ppu;
}

void ppu_destroy(PPU *ppu)
{
    free(ppu);
}

void ppu_set_cartridge(PPU *ppu, Cartridge *cartridge)
{
    if (ppu != NULL) {
        ppu->cartridge = cartridge;
    }
}

void ppu_set_region(PPU *ppu, Region region)
{
    if (ppu == NULL || region == REGION_AUTO) {
        return;
    }

    ppu->region = region;
    ppu_reset(ppu);
}

Region ppu_region(const PPU *ppu)
{
    return (ppu != NULL) ? ppu->region : REGION_NTSC;
}

void ppu_reset(PPU *ppu)
{
    if (ppu == NULL) {
        return;
    }

    ppu->ctrl = 0;
    ppu->mask = 0;
    ppu->status = 0;
    ppu->oam_addr = 0;
    memset(ppu->oam, 0, sizeof(ppu->oam));
    memset(ppu->nametable, 0, sizeof(ppu->nametable));
    memset(ppu->framebuffer, 0, sizeof(ppu->framebuffer));
    ppu->read_buffer = 0;
    ppu->io_latch = 0;

    ppu->v = 0;
    ppu->t = 0;
    ppu->fine_x = 0;
    ppu->w = false;

    /* Mulai di pre-render agar frame pertama tetap lengkap. */
    ppu->scanline = nes_region_prerender_scanline(ppu->region);
    ppu->dot = 0;
    ppu->frame_count = 0;
    ppu->odd_frame = false;
    ppu->nmi_pending = false;
    ppu->frame_complete = false;

    ppu_background_reset(ppu);
    ppu->sprite_count = 0;
    ppu->sprite_zero_on_line = false;
}

void ppu_increment_x(PPU *ppu)
{
    if ((ppu->v & 0x001FU) == 31U) {
        ppu->v = (uint16_t)(ppu->v & (uint16_t)~0x001FU);
        ppu->v = (uint16_t)(ppu->v ^ 0x0400U); /* Ganti nametable horizontal. */
    } else {
        ppu->v = (uint16_t)(ppu->v + 1U);
    }
}

void ppu_increment_y(PPU *ppu)
{
    if ((ppu->v & PPU_FINE_Y_MASK) != PPU_FINE_Y_MASK) {
        ppu->v = (uint16_t)(ppu->v + 0x1000U);
        return;
    }

    ppu->v = (uint16_t)(ppu->v & (uint16_t)~PPU_FINE_Y_MASK);

    {
        uint16_t coarse_y = (uint16_t)((ppu->v & PPU_COARSE_Y_MASK) >> 5);

        if (coarse_y == 29U) {
            coarse_y = 0;
            ppu->v = (uint16_t)(ppu->v ^ 0x0800U); /* Ganti nametable vertikal. */
        } else if (coarse_y == 31U) {
            coarse_y = 0; /* Tidak mengganti nametable pada kasus ini. */
        } else {
            ++coarse_y;
        }
        ppu->v = (uint16_t)((ppu->v & (uint16_t)~PPU_COARSE_Y_MASK) | (coarse_y << 5));
    }
}

void ppu_copy_x(PPU *ppu)
{
    ppu->v = (uint16_t)((ppu->v & (uint16_t)~0x041FU) | (ppu->t & 0x041FU));
}

void ppu_copy_y(PPU *ppu)
{
    ppu->v = (uint16_t)((ppu->v & (uint16_t)~0x7BE0U) | (ppu->t & 0x7BE0U));
}

void ppu_clock(PPU *ppu)
{
    bool rendering = false;
    int prerender = 0;
    int scanlines = 0;

    if (ppu == NULL) {
        return;
    }

    rendering = (ppu->mask & PPU_MASK_RENDERING) != 0U;
    prerender = nes_region_prerender_scanline(ppu->region);
    scanlines = nes_region_scanlines(ppu->region);

    /* Pre-render: bersihkan flag status yang hanya berlaku satu frame. */
    if (ppu->scanline == prerender && ppu->dot == 1) {
        ppu->status = (uint8_t)(ppu->status &
                                (uint8_t)~(PPU_STATUS_VBLANK | PPU_STATUS_SPRITE0 |
                                           PPU_STATUS_OVERFLOW));
    }

    /* Awal VBlank: set flag dan kirim edge NMI bila diaktifkan. */
    if (ppu->scanline == 241 && ppu->dot == 1) {
        ppu->status = (uint8_t)(ppu->status | PPU_STATUS_VBLANK);
        if ((ppu->ctrl & PPU_CTRL_NMI_ENABLE) != 0U) {
            ppu->nmi_pending = true;
        }
    }

    if (rendering && (ppu->scanline < NES_VISIBLE_SCANLINES ||
                      ppu->scanline == prerender)) {
        ppu_background_tick(ppu);
    }

    /* Evaluasi sprite untuk scanline yang akan digambar. */
    if (rendering && ppu->scanline < NES_VISIBLE_SCANLINES && ppu->dot == 0) {
        ppu_sprites_evaluate(ppu);
    }

    if (rendering && (ppu->scanline < NES_VISIBLE_SCANLINES ||
                      ppu->scanline == prerender)) {
        if (ppu->dot == 256) {
            ppu_increment_y(ppu);
        } else if (ppu->dot == 257) {
            ppu_copy_x(ppu);
        }
    }

    if (ppu->scanline == prerender && rendering && ppu->dot >= 280 && ppu->dot <= 304) {
        ppu_copy_y(ppu);
    }

    if (ppu->scanline < NES_VISIBLE_SCANLINES && ppu->dot >= 1 && ppu->dot <= 256) {
        ppu_render_pixel(ppu);
    }

    /* Akhir visible: frame siap dipresentasikan. */
    if (ppu->scanline == (NES_VISIBLE_SCANLINES - 1) && ppu->dot == 340) {
        ppu->frame_complete = true;
        ++ppu->frame_count;
        ppu->odd_frame = !ppu->odd_frame;
    }

    /* Maju ke dot berikutnya. Frame ganjil melewati satu dot pre-render. */
    ++ppu->dot;
    if (ppu->odd_frame && ppu->scanline == prerender && ppu->dot == 340) {
        ppu->dot = 341;
    }
    if (ppu->dot >= NES_DOTS_PER_SCANLINE) {
        ppu->dot = 0;
        ++ppu->scanline;
        if (ppu->scanline >= scanlines) {
            ppu->scanline = 0;
        }
    }
}

bool ppu_frame_complete(const PPU *ppu)
{
    return ppu != NULL && ppu->frame_complete;
}

void ppu_clear_frame_complete(PPU *ppu)
{
    if (ppu != NULL) {
        ppu->frame_complete = false;
    }
}

bool ppu_take_nmi(PPU *ppu)
{
    bool pending = false;

    if (ppu == NULL) {
        return false;
    }

    pending = ppu->nmi_pending;
    ppu->nmi_pending = false;
    return pending;
}

void ppu_oam_dma(PPU *ppu, const uint8_t *source)
{
    if (ppu == NULL || source == NULL) {
        return;
    }

    memcpy(ppu->oam, source, NES_OAM_SIZE);
}

const uint8_t *ppu_framebuffer(const PPU *ppu)
{
    return (ppu != NULL) ? ppu->framebuffer : NULL;
}

int ppu_scanline(const PPU *ppu)
{
    return (ppu != NULL) ? ppu->scanline : 0;
}

int ppu_dot(const PPU *ppu)
{
    return (ppu != NULL) ? ppu->dot : 0;
}

uint64_t ppu_frame_count(const PPU *ppu)
{
    return (ppu != NULL) ? ppu->frame_count : 0U;
}
