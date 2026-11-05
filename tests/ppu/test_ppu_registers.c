/*
 * test_ppu_registers.c - VBlank, NMI, alamat VRAM, OAM, dan increment.
 */
#include "../test.h"
#include "../test_helpers.h"

#include "cartridge/cartridge.h"
#include "ppu/ppu.h"
#include "ppu/ppu_memory.h"

#include <string.h>

static Cartridge *create_cart(void)
{
    uint8_t *rom = test_ines_create(1, 1, 0x00, 0x00, false, NULL);
    Cartridge *cart = NULL;
    ErrorCode error = ERR_OK;

    if (rom == NULL) {
        return NULL;
    }

    cart = cartridge_load_memory(rom, 16U + 16384U + 8192U, &error);
    test_ines_free(rom);
    return cart;
}

void test_ppu_registers_run(void)
{
    Cartridge *cart = create_cart();
    PPU *ppu = NULL;

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

    /* Alamat VRAM dan increment 1. */
    ppu_write_register(ppu, 6, 0x20);
    ppu_write_register(ppu, 6, 0x00);
    ppu_write_register(ppu, 7, 0x11);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2000), 0x11);

    /* Increment 32. */
    ppu_write_register(ppu, 0, 0x04);
    ppu_write_register(ppu, 6, 0x20);
    ppu_write_register(ppu, 6, 0x00);
    ppu_write_register(ppu, 7, 0x22);
    ppu_write_register(ppu, 7, 0x33);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2000), 0x22);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2020), 0x33);

    /* OAMADDR/OAMDATA: tulis $10, alamat otomatis menjadi $11. */
    ppu_write_register(ppu, 3, 0x10);
    ppu_write_register(ppu, 4, 0xAB);
    CHECK_EQ_U8(ppu_oam_read(ppu, 0x10), 0xAB);
    ppu_write_register(ppu, 3, 0x10);
    CHECK_EQ_U8(ppu_read_register(ppu, 4), 0xAB);

    /*
     * VBlank: reset berada di scanline 261 dot 0. Setelah 82523 dot,
     * PPU berada di scanline 241 dekat dot 1 dan flag VBlank aktif.
     */
    ppu_reset(ppu);
    ppu_write_register(ppu, 0, 0x80); /* aktifkan NMI */
    for (int i = 0; i < 82524; ++i) {
        ppu_clock(ppu);
    }
    CHECK_EQ_INT(ppu_scanline(ppu), 241);
    CHECK_EQ_INT(ppu_dot(ppu), 2);
    CHECK_TRUE(ppu_take_nmi(ppu));

    {
        uint8_t status = ppu_read_register(ppu, 2);
        CHECK_TRUE((status & 0x80) != 0);
        CHECK_TRUE((ppu_read_register(ppu, 2) & 0x80) == 0);
    }

    /* Membaca $2002 mereset latch alamat: dua tulisan berikutnya membentuk alamat. */
    ppu_write_register(ppu, 6, 0x20);
    ppu_write_register(ppu, 6, 0x01);
    (void)ppu_read_register(ppu, 2);
    ppu_write_register(ppu, 6, 0x20);
    ppu_write_register(ppu, 6, 0x02);
    ppu_write_register(ppu, 7, 0x44);
    CHECK_EQ_U8(ppu_memory_read(ppu, 0x2002), 0x44);

    /* NMI tidak dikirim bila bit enable mati. */
    ppu_reset(ppu);
    ppu_write_register(ppu, 0, 0x00);
    for (int i = 0; i < 82524; ++i) {
        ppu_clock(ppu);
    }
    CHECK_FALSE(ppu_take_nmi(ppu));

    /*
     * PAL: pre-render ada di scanline 311, VBlank tetap mulai di 241.
     * Jarak dari pre-render ke VBlank sama dengan NTSC karena area visible
     * juga 0-239.
     */
    ppu_set_region(ppu, REGION_PAL);
    CHECK_EQ_INT(ppu_region(ppu), REGION_PAL);
    CHECK_EQ_INT(ppu_scanline(ppu), 311);
    CHECK_EQ_INT(ppu_dot(ppu), 0);

    ppu_write_register(ppu, 0, 0x80);
    for (int i = 0; i < 82524; ++i) {
        ppu_clock(ppu);
    }
    CHECK_EQ_INT(ppu_scanline(ppu), 241);
    CHECK_EQ_INT(ppu_dot(ppu), 2);
    CHECK_TRUE(ppu_take_nmi(ppu));

    /* Setelah pre-render 311 selesai, PPU kembali ke scanline 0. */
    ppu_set_region(ppu, REGION_PAL);
    for (int i = 0; i < 341; ++i) {
        ppu_clock(ppu);
    }
    CHECK_EQ_INT(ppu_scanline(ppu), 0);
    CHECK_EQ_INT(ppu_dot(ppu), 0);

    /* Frame PAL selesai setelah area visible 240 baris (scanline 239 dot 340). */
    ppu_set_region(ppu, REGION_PAL);
    for (int i = 0; i < 82181; ++i) {
        ppu_clock(ppu);
    }
    CHECK_TRUE(ppu_frame_complete(ppu));
    CHECK_EQ_INT(ppu_scanline(ppu), 240);
    CHECK_EQ_INT(ppu_dot(ppu), 0);
    CHECK_EQ_INT(ppu_frame_count(ppu), 1);

    /* Sisa VBlank PAL mengantar PPU kembali ke pre-render scanline 311. */
    for (int i = 0; i < 24211; ++i) {
        ppu_clock(ppu);
    }
    CHECK_EQ_INT(ppu_scanline(ppu), 311);
    CHECK_EQ_INT(ppu_dot(ppu), 0);

    ppu_destroy(ppu);
    cartridge_destroy(cart);
}
