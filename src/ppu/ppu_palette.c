/*
 * ppu_palette.c - Tabel warna master NTSC dan akses palette RAM.
 *
 * Palette NES berisi 64 warna (6 bit). PPU hanya menyimpan indeks warna
 * (0-63) di framebuffer internal; frontend mengubahnya menjadi RGB saat
 * mengunggah texture.
 */
#include "ppu_internal.h"

typedef struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

/* Tabel warna 2C02 NTSC yang umum dipakai emulator. */
static const RGB g_ntsc_palette[64] = {
    {84, 84, 84},    {0, 30, 116},   {8, 16, 144},   {48, 0, 136},   {68, 0, 100},
    {92, 0, 48},     {84, 4, 0},     {60, 24, 0},    {32, 42, 0},    {8, 58, 0},
    {0, 64, 0},      {0, 60, 0},     {0, 50, 60},    {0, 0, 0},      {0, 0, 0},
    {0, 0, 0},       {152, 150, 152}, {8, 76, 196},  {48, 50, 236},  {92, 30, 228},
    {136, 20, 176},  {160, 20, 100}, {152, 34, 32},  {120, 60, 0},   {84, 90, 0},
    {40, 114, 0},    {8, 124, 0},    {0, 118, 40},   {0, 102, 120},  {0, 0, 0},
    {0, 0, 0},       {0, 0, 0},      {236, 238, 236}, {76, 154, 236}, {120, 124, 236},
    {176, 98, 236},  {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32},
    {160, 170, 0},   {116, 196, 0},  {76, 208, 32},  {56, 204, 108}, {56, 180, 204},
    {60, 60, 60},    {0, 0, 0},      {0, 0, 0},      {236, 238, 236}, {168, 204, 236},
    {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176},
    {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180},
    {160, 214, 228}, {160, 162, 160}, {0, 0, 0},       {0, 0, 0}
};

uint32_t ppu_palette_rgb(uint8_t palette_index)
{
    const RGB *color = &g_ntsc_palette[palette_index & 0x3FU];
    return ((uint32_t)color->r << 16) | ((uint32_t)color->g << 8) | (uint32_t)color->b;
}

uint8_t ppu_palette_read(PPU *ppu, uint8_t index)
{
    uint8_t mirrored = (uint8_t)(index & 0x1FU);

    if (ppu == NULL) {
        return 0;
    }

    /* Alamat $3F10/$3F14/$3F18/$3F1C memantulkan warna universal. */
    if (mirrored == 0x10U || mirrored == 0x14U || mirrored == 0x18U || mirrored == 0x1CU) {
        mirrored = (uint8_t)(mirrored - 0x10U);
    }

    return ppu->palette[mirrored];
}

void ppu_palette_write(PPU *ppu, uint8_t index, uint8_t value)
{
    uint8_t mirrored = (uint8_t)(index & 0x1FU);

    if (ppu == NULL) {
        return;
    }

    if (mirrored == 0x10U || mirrored == 0x14U || mirrored == 0x18U || mirrored == 0x1CU) {
        mirrored = (uint8_t)(mirrored - 0x10U);
    }

    ppu->palette[mirrored] = (uint8_t)(value & 0x3FU);
}
