/*
 * test_ppu_memory.c - Pattern/nametable/palette/OAM dan mirroring.
 */
#include "../test.h"
#include "../test_helpers.h"

#include "cartridge/cartridge.h"
#include "ppu/ppu.h"
#include "ppu/ppu_memory.h"

#include <string.h>

static Cartridge *load_cart(uint8_t flags6)
{
    uint8_t *rom = test_ines_create(1, 1, flags6, 0x00, false, NULL);
    Cartridge *cart = NULL;
    ErrorCode error = ERR_OK;

    if (rom == NULL) {
        return NULL;
    }

    cart = cartridge_load_memory(rom, 16U + 16384U + 8192U, &error);
    test_ines_free(rom);
    return cart;
}

void test_ppu_memory_run(void)
{
    Cartridge *cart = NULL;
    PPU *ppu = NULL;

    cart = load_cart(0x00); /* horizontal */
    CHECK_TRUE(cart != NULL);
    if (cart == NULL) {
        return;
    }

    ppu = ppu_create(cart, REGION_NTSC);
    CHECK_TRUE(ppu != NULL);
    if (ppu == NULL) {
        cartridge_destroy(cart);
        return;
    }

    /* Mirroring horizontal: $2000 = $2400, $2800 berbeda. */
    ppu_memory_write(ppu, 0x2000, 0x12);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2400), 0x12);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2800), 0x00);

    /* Cermin $3000-$3EFF. */
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x3000), 0x12);

    /* Palette dan mirroring warna universal. */
    ppu_memory_write(ppu, 0x3F00, 0x21);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x3F10), 0x21);
    ppu_memory_write(ppu, 0x3F10, 0x30);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x3F00), 0x30);

    /* Palette hanya 6 bit. */
    ppu_memory_write(ppu, 0x3F01, 0xFF);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x3F01), 0x3F);

    /* OAM. */
    ppu_oam_write(ppu, 0x10, 0xAB);
    CHECK_EQ_U8(ppu_oam_read(ppu, 0x10), 0xAB);

    /* CHR ROM tidak dapat ditulis. */
    ppu_memory_write(ppu, 0x0000, 0x99);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x0000), 0x00);

    ppu_destroy(ppu);
    cartridge_destroy(cart);

    /* Mirroring vertikal: $2000 = $2800, $2400 berbeda. */
    cart = load_cart(0x01);
    CHECK_TRUE(cart != NULL);
    if (cart == NULL) {
        return;
    }

    ppu = ppu_create(cart, REGION_NTSC);
    CHECK_TRUE(ppu != NULL);
    if (ppu == NULL) {
        cartridge_destroy(cart);
        return;
    }

    ppu_memory_write(ppu, 0x2000, 0x34);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2800), 0x34);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2400), 0x00);

    ppu_destroy(ppu);
    cartridge_destroy(cart);
}
