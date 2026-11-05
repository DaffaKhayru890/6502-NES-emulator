/*
 * test_nes_integration.c - ROM sintetis lengkap: CPU + bus + PPU.
 *
 * Program 6502 menulis palette $3F00 = $0F, mengaktifkan background, lalu
 * berputar di JMP. Karena pattern table (CHR RAM) masih nol, seluruh layar
 * harus berwarna palette 0.
 */
#include "../test.h"
#include "../test_helpers.h"

#include "core/nes.h"
#include "ppu/ppu.h"

#include <string.h>

void test_nes_integration_run(void)
{
    uint8_t *rom = NULL;
    size_t size = 0;
    size_t prg = 0;
    Nes *nes = NULL;
    ErrorCode error = ERR_OK;
    const uint8_t code[] = {
        0x78,             /* SEI */
        0xA9, 0x3F,       /* LDA #$3F */
        0x8D, 0x06, 0x20, /* STA $2006 */
        0xA9, 0x00,       /* LDA #$00 */
        0x8D, 0x06, 0x20, /* STA $2006 */
        0xA9, 0x0F,       /* LDA #$0F */
        0x8D, 0x07, 0x20, /* STA $2007 */
        0xA9, 0x0A,       /* LDA #$0A */
        0x8D, 0x01, 0x20, /* STA $2001 */
        0x4C, 0x15, 0x80  /* JMP $8015 */
    };
    const uint8_t *framebuffer = NULL;

    rom = test_ines_create(1, 0, 0x00, 0x00, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom == NULL) {
        return;
    }

    prg = test_ines_prg_offset(false);
    memcpy(rom + prg, code, sizeof(code));
    /* Vektor reset dan NMI/IRQ menunjuk ke awal program. */
    rom[prg + 0x3FFC] = 0x00;
    rom[prg + 0x3FFD] = 0x80;
    rom[prg + 0x3FFA] = 0x00;
    rom[prg + 0x3FFB] = 0x80;
    rom[prg + 0x3FFE] = 0x00;
    rom[prg + 0x3FFF] = 0x80;

    nes = nes_create(44100, REGION_NTSC);
    CHECK_TRUE(nes != NULL);
    if (nes == NULL) {
        test_ines_free(rom);
        return;
    }

    error = nes_load_rom_data(nes, rom, size);
    test_ines_free(rom);
    CHECK_EQ_INT(error, ERR_OK);
    if (error != ERR_OK) {
        nes_destroy(nes);
        return;
    }

    nes_run_frame(nes);
    CHECK_EQ_INT(ppu_frame_count(nes_ppu(nes)), 1);
    CHECK_TRUE(nes_cpu(nes)->total_cycles > 0);
    CHECK_EQ_U16(nes_cpu(nes)->pc & 0x8000U, 0x8000);

    framebuffer = nes_framebuffer(nes);
    CHECK_TRUE(framebuffer != NULL);
    if (framebuffer != NULL) {
        CHECK_EQ_U8(framebuffer[0], 0x0F);
        CHECK_EQ_U8(framebuffer[100 * 256 + 128], 0x0F);
        CHECK_EQ_U8(framebuffer[239 * 256 + 255], 0x0F);
    }

    /* Frame kedua tetap stabil. */
    nes_run_frame(nes);
    CHECK_EQ_INT(ppu_frame_count(nes_ppu(nes)), 2);
    if (framebuffer != NULL) {
        CHECK_EQ_U8(framebuffer[0], 0x0F);
    }

    nes_destroy(nes);

    /*
     * Integrasi PAL: region dideteksi otomatis dari timing NES 2.0.
     * PPU dan APU harus ikut beralih ke PAL.
     */
    rom = test_ines_create(1, 0, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x00;
        rom[9] = 0x00;
        rom[12] = 0x01; /* timing PAL */
        prg = test_ines_prg_offset(false);
        memcpy(rom + prg, code, sizeof(code));
        rom[prg + 0x3FFC] = 0x00;
        rom[prg + 0x3FFD] = 0x80;
        rom[prg + 0x3FFA] = 0x00;
        rom[prg + 0x3FFB] = 0x80;
        rom[prg + 0x3FFE] = 0x00;
        rom[prg + 0x3FFF] = 0x80;

        nes = nes_create(44100, REGION_AUTO);
        CHECK_TRUE(nes != NULL);
        if (nes != NULL) {
            error = nes_load_rom_data(nes, rom, size);
            CHECK_EQ_INT(error, ERR_OK);
            CHECK_EQ_INT(nes_region(nes), REGION_PAL);
            CHECK_EQ_INT(ppu_region(nes_ppu(nes)), REGION_PAL);
            CHECK_EQ_INT(apu_region(nes_apu(nes)), REGION_PAL);
            CHECK_TRUE(nes_frame_rate(nes) > 49.9);
            CHECK_TRUE(nes_frame_rate(nes) < 50.1);

            nes_run_frame(nes);
            CHECK_EQ_INT(ppu_frame_count(nes_ppu(nes)), 1);
            CHECK_EQ_INT(ppu_scanline(nes_ppu(nes)), 240);
            nes_destroy(nes);
        }
        test_ines_free(rom);
    }

    /* Override eksplisit NTSC harus menang atas metadata PAL. */
    rom = test_ines_create(1, 0, 0x00, 0x08, false, &size);
    CHECK_TRUE(rom != NULL);
    if (rom != NULL) {
        rom[8] = 0x00;
        rom[9] = 0x00;
        rom[12] = 0x01;
        prg = test_ines_prg_offset(false);
        memcpy(rom + prg, code, sizeof(code));
        rom[prg + 0x3FFC] = 0x00;
        rom[prg + 0x3FFD] = 0x80;
        rom[prg + 0x3FFA] = 0x00;
        rom[prg + 0x3FFB] = 0x80;
        rom[prg + 0x3FFE] = 0x00;
        rom[prg + 0x3FFF] = 0x80;

        nes = nes_create(44100, REGION_NTSC);
        CHECK_TRUE(nes != NULL);
        if (nes != NULL) {
            error = nes_load_rom_data(nes, rom, size);
            CHECK_EQ_INT(error, ERR_OK);
            CHECK_EQ_INT(nes_region(nes), REGION_NTSC);
            CHECK_EQ_INT(ppu_region(nes_ppu(nes)), REGION_NTSC);
            CHECK_TRUE(nes_frame_rate(nes) > 59.9);
            CHECK_TRUE(nes_frame_rate(nes) < 60.2);
            nes_destroy(nes);
        }
        test_ines_free(rom);
    }
}
