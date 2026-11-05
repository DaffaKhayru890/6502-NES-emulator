/*
 * test_nrom.c - Header iNES, validasi, NROM, CHR RAM, dan trainer.
 */
#include "../test.h"
#include "../test_helpers.h"

#include "cartridge/cartridge.h"
#include "cartridge/mapper_nrom.h"

#include <string.h>

void test_cartridge_nrom_run(void)
{
    uint8_t *rom = NULL;
    size_t size = 0;
    size_t prg = 0;
    Cartridge *cart = NULL;
    ErrorCode error = ERR_OK;

    /* Header valid 16 KiB PRG + 8 KiB CHR. */
    rom = test_ines_create(1, 1, 0x00, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom == NULL) {
        return;
    }

    prg = test_ines_prg_offset(false);
    rom[prg] = 0x42;
    cart = cartridge_load_memory(rom, size, &error);
    CHECK_EQ_INT(error, ERR_OK);
    CHECK_TRUE(cart != NULL);
    if (cart != NULL) {
        CHECK_EQ_INT(cartridge_mapper_number(cart), 0);
        CHECK_FALSE(cartridge_is_nes2(cart));
        CHECK_EQ_INT(cartridge_submapper(cart), 0);
        CHECK_EQ_INT(cartridge_region(cart), REGION_NTSC);
        CHECK_EQ_INT(cartridge_mirroring(cart), MIRRORING_HORIZONTAL);
        CHECK_EQ_INT(cartridge_prg_rom_size(cart), 16384);
        CHECK_EQ_INT(cartridge_chr_size(cart), 8192);
        CHECK_FALSE(cartridge_chr_is_ram(cart));
        CHECK_EQ_U8(cartridge_cpu_read(cart, 0x8000), 0x42);
        CHECK_EQ_U8(cartridge_cpu_read(cart, 0xC000), 0x42); /* mirror NROM-128 */
        cartridge_destroy(cart);
    }
    test_ines_free(rom);

    /* 32 KiB PRG: $8000 dan $C000 berbeda. */
    rom = test_ines_create(2, 1, 0x01, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        prg = test_ines_prg_offset(false);
        rom[prg] = 0x10;
        rom[prg + 16384U] = 0x20;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_EQ_INT(cartridge_mirroring(cart), MIRRORING_VERTICAL);
            CHECK_EQ_U8(cartridge_cpu_read(cart, 0x8000), 0x10);
            CHECK_EQ_U8(cartridge_cpu_read(cart, 0xC000), 0x20);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /* CHR RAM ketika CHR ROM = 0. */
    rom = test_ines_create(1, 0, 0x00, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_TRUE(cartridge_chr_is_ram(cart));
            cartridge_ppu_write(cart, 0x0000, 0x5A);
            CHECK_EQ_U8(cartridge_ppu_read(cart, 0x0000), 0x5A);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /* Trainer 512 byte dilewati. */
    rom = test_ines_create(1, 0, 0x04, 0x00, true, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        prg = test_ines_prg_offset(true);
        rom[prg] = 0x77;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_TRUE(cartridge_has_trainer(cart));
            CHECK_EQ_U8(cartridge_cpu_read(cart, 0x8000), 0x77);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /* Battery flag. */
    rom = test_ines_create(1, 1, 0x02, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_TRUE(cartridge_has_battery(cart));
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /* Magic bytes salah. */
    rom = test_ines_create(1, 0, 0x00, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[0] = 0x00;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_INES_MAGIC);
        test_ines_free(rom);
    }

    /* NES 2.0 dasar: mapper 0, submapper 1, PRG 16 KiB, CHR 8 KiB. */
    rom = test_ines_create(1, 1, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x01; /* mapper tinggi 0, submapper 1 */
        rom[9] = 0x00; /* ukuran tinggi PRG/CHR = 0 */
        prg = test_ines_prg_offset(false);
        rom[prg] = 0x2A;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_TRUE(cartridge_is_nes2(cart));
            CHECK_EQ_INT(cartridge_mapper_number(cart), 0);
            CHECK_EQ_INT(cartridge_submapper(cart), 1);
            CHECK_EQ_INT(cartridge_prg_rom_size(cart), 16384);
            CHECK_EQ_INT(cartridge_chr_size(cart), 8192);
            CHECK_EQ_U8(cartridge_cpu_read(cart, 0x8000), 0x2A);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /* NES 2.0 CHR RAM bila CHR ROM = 0. */
    rom = test_ines_create(1, 0, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x00;
        rom[9] = 0x00;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_TRUE(cartridge_chr_is_ram(cart));
            cartridge_ppu_write(cart, 0x0000, 0x5A);
            CHECK_EQ_U8(cartridge_ppu_read(cart, 0x0000), 0x5A);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /*
     * NES 2.0 encoding eksponen: PRG 32 KiB = 2^15, sehingga nilai 12-bit
     * menjadi 0xF3C (nibble tinggi F, E=15, multiplier=1).
     */
    rom = test_ines_create(2, 1, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[4] = 0x3C;
        rom[8] = 0x00;
        rom[9] = 0x0F;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_EQ_INT(cartridge_prg_rom_size(cart), 32768);
            CHECK_EQ_INT(cartridge_chr_size(cart), 8192);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /*
     * NES 2.0 encoding eksponen pada CHR: 8 KiB = 2^13, sehingga nilai
     * 12-bit menjadi 0xF34 dan byte 9 bagian tinggi bernilai 0xF.
     */
    rom = test_ines_create(1, 1, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[5] = 0x34;
        rom[8] = 0x00;
        rom[9] = 0xF0;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_EQ_INT(cartridge_prg_rom_size(cart), 16384);
            CHECK_EQ_INT(cartridge_chr_size(cart), 8192);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /* NES 2.0: mapper tinggi dari byte 8 (mapper 256) ditolak. */
    rom = test_ines_create(1, 0, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x10;
        rom[9] = 0x00;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_UNSUPPORTED_MAPPER);
        test_ines_free(rom);
    }

    /* NES 2.0: console type Vs. System ditolak. */
    rom = test_ines_create(1, 0, 0x00, 0x09, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x00;
        rom[9] = 0x00;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_UNSUPPORTED_CONSOLE);
        test_ines_free(rom);
    }

    /* NES 2.0: timing PAL diterima dan terekspos lewat metadata. */
    rom = test_ines_create(1, 0, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x00;
        rom[9] = 0x00;
        rom[12] = 0x01;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_EQ_INT(cartridge_region(cart), REGION_PAL);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /* NES 2.0: multi-region dijalankan sebagai NTSC. */
    rom = test_ines_create(1, 0, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x00;
        rom[9] = 0x00;
        rom[12] = 0x02;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_EQ_INT(error, ERR_OK);
        CHECK_TRUE(cart != NULL);
        if (cart != NULL) {
            CHECK_EQ_INT(cartridge_region(cart), REGION_NTSC);
            cartridge_destroy(cart);
        }
        test_ines_free(rom);
    }

    /* NES 2.0: timing Dendy tetap ditolak. */
    rom = test_ines_create(1, 0, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x00;
        rom[9] = 0x00;
        rom[12] = 0x03;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_UNSUPPORTED_REGION);
        test_ines_free(rom);
    }

    /* NES 2.0: encoding eksponen yang overflow ditolak. */
    rom = test_ines_create(1, 0, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[4] = 0xFF; /* E=63, di luar batas aman */
        rom[8] = 0x00;
        rom[9] = 0x0F;
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_INES_FORMAT);
        test_ines_free(rom);
    }

    /* Mapper tidak didukung (flags6 high nibble = 2). */
    rom = test_ines_create(1, 0, 0x20, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_UNSUPPORTED_MAPPER);
        test_ines_free(rom);
    }

    /* File terpotong. */
    rom = test_ines_create(1, 1, 0x00, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        cart = cartridge_load_memory(rom, size - 1U, &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_INVALID_ROM_SIZE);
        test_ines_free(rom);
    }

    /* Header terlalu pendek. */
    {
        uint8_t tiny[8] = {0};
        cart = cartridge_load_memory(tiny, sizeof(tiny), &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_INES_FORMAT);
    }

    /* PRG 0 unit ditolak. */
    rom = test_ines_create(0, 0, 0x00, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        cart = cartridge_load_memory(rom, size, &error);
        CHECK_TRUE(cart == NULL);
        CHECK_EQ_INT(error, ERR_INES_FORMAT);
        test_ines_free(rom);
    }
}
