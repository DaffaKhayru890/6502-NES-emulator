/*
 * types.h - Tipe dasar dan konstanta global TinyNES.
 *
 * File ini sengaja kecil. Ia hanya menyatukan header standar yang dipakai
 * hampir semua modul, agar setiap file lain tidak perlu mengulang include
 * yang sama dan agar tipe data yang digunakan konsisten.
 */
#ifndef TINYNES_UTIL_TYPES_H
#define TINYNES_UTIL_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TINYNES_VERSION "0.1.0"
#define TINYNES_TITLE "TinyNES"

/* Ukuran layar NES dalam piksel. */
#define NES_SCREEN_WIDTH 256
#define NES_SCREEN_HEIGHT 240

/*
 * Region timing. REGION_AUTO berarti "pilih dari metadata ROM"; bila ROM
 * tidak menyatakan region (iNES 1.0), dipakai NTSC.
 */
typedef enum Region {
    REGION_NTSC = 0,
    REGION_PAL = 1,
    REGION_AUTO = 2
} Region;

/* Frekuensi CPU NTSC dan PAL (Hz). */
#define NES_NTSC_CPU_FREQUENCY 1789773.0
#define NES_PAL_CPU_FREQUENCY 1662607.0

/* Rasio PPU:CPU. PAL 3,2 = 16/5, NTSC 3,0 = 15/5. */
#define NES_REGION_DOT_SCALE 5U
#define NES_NTSC_DOTS_PER_CPU_CYCLE 15U
#define NES_PAL_DOTS_PER_CPU_CYCLE 16U

/* Jumlah scanline per frame dan dot per scanline. */
#define NES_NTSC_SCANLINES_PER_FRAME 262
#define NES_PAL_SCANLINES_PER_FRAME 312
#define NES_DOTS_PER_SCANLINE 341
#define NES_VISIBLE_SCANLINES 240

/* Alias kompatibilitas untuk kode yang secara eksplisit memakai NTSC. */
#define NES_CPU_FREQUENCY NES_NTSC_CPU_FREQUENCY
#define NES_SCANLINES_PER_FRAME NES_NTSC_SCANLINES_PER_FRAME
#define NES_PPU_DOTS_PER_CPU_CYCLE 3

static inline bool nes_region_is_pal(Region region)
{
    return region == REGION_PAL;
}

static inline int nes_region_scanlines(Region region)
{
    return nes_region_is_pal(region) ? NES_PAL_SCANLINES_PER_FRAME
                                     : NES_NTSC_SCANLINES_PER_FRAME;
}

static inline int nes_region_prerender_scanline(Region region)
{
    return nes_region_scanlines(region) - 1;
}

static inline double nes_region_cpu_frequency(Region region)
{
    return nes_region_is_pal(region) ? NES_PAL_CPU_FREQUENCY : NES_NTSC_CPU_FREQUENCY;
}

/* Numerator dot per siklus CPU; pembaginya NES_REGION_DOT_SCALE. */
static inline unsigned nes_region_dot_numerator(Region region)
{
    return nes_region_is_pal(region) ? NES_PAL_DOTS_PER_CPU_CYCLE
                                     : NES_NTSC_DOTS_PER_CPU_CYCLE;
}

static inline double nes_region_frame_rate(Region region)
{
    double ppu_frequency = nes_region_cpu_frequency(region) *
                           ((double)nes_region_dot_numerator(region) /
                            (double)NES_REGION_DOT_SCALE);
    double dots_per_frame =
        (double)nes_region_scanlines(region) * (double)NES_DOTS_PER_SCANLINE;

    return ppu_frequency / dots_per_frame;
}

/* Jumlah byte internal RAM CPU NES. */
#define NES_CPU_RAM_SIZE 2048

/* Jumlah byte OAM (Object Attribute Memory) PPU. */
#define NES_OAM_SIZE 256

/* Jumlah byte palette RAM PPU. */
#define NES_PALETTE_SIZE 32

/* Jumlah byte CHR RAM cadangan bila cartridge tidak memiliki CHR ROM. */
#define NES_CHR_RAM_SIZE 8192

/* Jumlah byte PRG RAM ($6000-$7FFF). */
#define NES_PRG_RAM_SIZE 8192

/* Batas jumlah sprite yang dapat ditampilkan per scanline. */
#define NES_MAX_SPRITES_PER_LINE 8

#endif /* TINYNES_UTIL_TYPES_H */
