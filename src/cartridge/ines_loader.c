/*
 * ines_loader.c - Pembaca format iNES 1.0 dan NES 2.0.
 *
 * Kedua format memakai header 16 byte:
 *   0-3   : "NES" + 0x1A
 *   4     : PRG ROM ukuran rendah (16 KiB unit)
 *   5     : CHR ROM ukuran rendah (8 KiB unit; 0 berarti CHR RAM)
 *   6     : flag 6 (mirroring, battery, trainer, mapper low nibble)
 *   7     : flag 7 (mapper tengah, console type, penanda NES 2.0)
 *   8     : NES 2.0: mapper tinggi + submapper
 *   9     : NES 2.0: ukuran PRG/CHR bagian tinggi
 *   10-15 : NES 2.0: RAM, timing, system, dan metadata lain
 *
 * NES 2.0 dideteksi dari bit 2-3 flag 7 bernilai 10b. Format ini
 * memperluas mapper menjadi 12 bit dan ukuran ROM menjadi 12 bit, dengan
 * encoding eksponen-multiplier bila nibble tinggi bernilai 0xF.
 */
#include "cartridge/cartridge.h"

#include "cartridge/cartridge_internal.h"
#include "cartridge/mapper_nrom.h"
#include "util/file.h"
#include "util/log.h"

#include <stdlib.h>
#include <string.h>

#define INES_HEADER_SIZE 16U
#define INES_TRAINER_SIZE 512U
#define INES_PRG_UNIT 16384U
#define INES_CHR_UNIT 8192U

static void loader_fail(Cartridge *cart, ErrorCode *error, ErrorCode code)
{
    if (error != NULL) {
        *error = code;
    }
    if (cart != NULL) {
        cartridge_destroy(cart);
    }
}

/*
 * Menghitung ukuran ROM dari nilai 12-bit NES 2.0.
 *
 * Nilai normal: encoded * unit. Bila nibble tinggi = 0xF, nilai memakai
 * encoding eksponen-multiplier: 1111 EEEE EEMM, dengan ukuran
 * 2^E * (MM * 2 + 1) byte.
 */
static bool decode_rom_size(uint16_t encoded, size_t unit, size_t *out_size)
{
    uint64_t size = 0;
    unsigned exponent = 0;
    unsigned multiplier = 0;

    if (out_size == NULL) {
        return false;
    }

    if ((encoded & 0x0F00U) == 0x0F00U) {
        exponent = (unsigned)((encoded >> 2) & 0x003FU);
        multiplier = (unsigned)((encoded & 0x0003U) * 2U + 1U);

        /* Hindari overflow 64-bit sebelum dikonversi ke size_t. */
        if (exponent >= 63U) {
            return false;
        }
        size = (uint64_t)1U << exponent;
        if (size > (UINT64_MAX / multiplier)) {
            return false;
        }
        size *= multiplier;
    } else {
        size = (uint64_t)encoded * (uint64_t)unit;
    }

    if (size > (uint64_t)SIZE_MAX) {
        return false;
    }

    *out_size = (size_t)size;
    return true;
}

Cartridge *cartridge_load_memory(const uint8_t *data, size_t size, ErrorCode *error)
{
    Cartridge *cart = NULL;
    uint8_t flags6 = 0;
    uint8_t flags7 = 0;
    uint8_t submapper = 0;
    uint8_t console_type = 0;
    uint8_t timing = 0;
    uint16_t prg_encoded = 0;
    uint16_t chr_encoded = 0;
    int mapper_number = 0;
    Region region = REGION_NTSC;
    size_t prg_size = 0;
    size_t chr_size = 0;
    size_t payload_offset = 0;
    size_t expected_size = 0;
    bool has_trainer = false;
    bool nes2 = false;

    if (error != NULL) {
        *error = ERR_OK;
    }

    if (data == NULL || size < INES_HEADER_SIZE) {
        if (error != NULL) {
            *error = ERR_INES_FORMAT;
        }
        return NULL;
    }

    if (data[0] != 0x4EU || data[1] != 0x45U || data[2] != 0x53U || data[3] != 0x1AU) {
        if (error != NULL) {
            *error = ERR_INES_MAGIC;
        }
        return NULL;
    }

    flags6 = data[6];
    flags7 = data[7];
    nes2 = ((flags7 & 0x0CU) == 0x08U);

    prg_encoded = (uint16_t)data[4];
    chr_encoded = (uint16_t)data[5];
    mapper_number = (int)((uint16_t)(flags7 & 0xF0U) | (uint16_t)(flags6 >> 4));

    if (nes2) {
        submapper = (uint8_t)(data[8] & 0x0FU);
        console_type = (uint8_t)(flags7 & 0x03U);
        timing = (uint8_t)(data[12] & 0x03U);

        prg_encoded = (uint16_t)(prg_encoded | ((uint16_t)(data[9] & 0x0FU) << 8));
        chr_encoded = (uint16_t)(chr_encoded | ((uint16_t)(data[9] >> 4) << 8));
        mapper_number = (int)((uint16_t)mapper_number | (uint16_t)(data[8] & 0xF0U));

        if (console_type != 0U) {
            if (error != NULL) {
                *error = ERR_UNSUPPORTED_CONSOLE;
            }
            log_error("Console type NES 2.0 %u tidak didukung. TinyNES hanya "
                      "menjalankan NES/Famicom standar.",
                      (unsigned)console_type);
            return NULL;
        }

        /*
         * Timing NES 2.0: 0 = NTSC, 1 = PAL, 2 = multi-region, 3 = Dendy.
         * TinyNES mendukung NTSC dan PAL. Multi-region dijalankan sebagai
         * NTSC secara default. Dendy belum didukung.
         */
        if (timing == 3U) {
            if (error != NULL) {
                *error = ERR_UNSUPPORTED_REGION;
            }
            log_error("Timing NES 2.0 Dendy tidak didukung. TinyNES mendukung "
                      "NTSC dan PAL.");
            return NULL;
        }
        region = (timing == 1U) ? REGION_PAL : REGION_NTSC;
    }

    if (mapper_number != 0) {
        if (error != NULL) {
            *error = ERR_UNSUPPORTED_MAPPER;
        }
        log_error("Mapper %d tidak didukung. TinyNES tahap ini hanya NROM (0).",
                  mapper_number);
        return NULL;
    }

    if (!decode_rom_size(prg_encoded, INES_PRG_UNIT, &prg_size) || prg_size == 0U) {
        if (error != NULL) {
            *error = ERR_INES_FORMAT;
        }
        log_error("Ukuran PRG ROM pada header tidak valid.");
        return NULL;
    }

    if (!decode_rom_size(chr_encoded, INES_CHR_UNIT, &chr_size)) {
        if (error != NULL) {
            *error = ERR_INES_FORMAT;
        }
        log_error("Ukuran CHR ROM pada header tidak valid.");
        return NULL;
    }

    /* NROM hanya memiliki PRG 16/32 KiB dan CHR 0/8 KiB. */
    if ((prg_size != 16384U && prg_size != 32768U) ||
        (chr_size != 0U && chr_size != 8192U)) {
        if (error != NULL) {
            *error = ERR_INES_FORMAT;
        }
        log_error("Ukuran ROM tidak cocok untuk NROM: PRG %zu byte, CHR %zu byte. "
                  "NROM mendukung PRG 16/32 KiB dan CHR 0/8 KiB.",
                  prg_size, chr_size);
        return NULL;
    }

    has_trainer = (flags6 & 0x04U) != 0U;
    payload_offset = INES_HEADER_SIZE + (has_trainer ? INES_TRAINER_SIZE : 0U);
    expected_size = payload_offset + prg_size + chr_size;

    if (size < expected_size) {
        if (error != NULL) {
            *error = ERR_INVALID_ROM_SIZE;
        }
        log_error("Ukuran file %zu byte, header menyatakan minimal %zu byte.", size,
                  expected_size);
        return NULL;
    }

    cart = cartridge_allocate();
    if (cart == NULL) {
        loader_fail(NULL, error, ERR_ALLOCATION);
        return NULL;
    }

    cart->prg_rom_size = prg_size;
    cart->prg_rom = (uint8_t *)malloc(prg_size);
    if (cart->prg_rom == NULL) {
        loader_fail(cart, error, ERR_ALLOCATION);
        return NULL;
    }
    memcpy(cart->prg_rom, data + payload_offset, prg_size);

    /* Jika CHR ROM 0, cartridge memakai CHR RAM yang dapat ditulis PPU. */
    cart->chr_is_ram = (chr_size == 0U);
    cart->chr_size = cart->chr_is_ram ? NES_CHR_RAM_SIZE : chr_size;
    cart->chr_mem = (uint8_t *)calloc(1, cart->chr_size);
    if (cart->chr_mem == NULL) {
        loader_fail(cart, error, ERR_ALLOCATION);
        return NULL;
    }
    if (!cart->chr_is_ram) {
        memcpy(cart->chr_mem, data + payload_offset + prg_size, chr_size);
    }

    cart->mapper_number = mapper_number;
    cart->submapper = submapper;
    cart->nes2 = nes2;
    cart->region = region;
    cart->battery = (flags6 & 0x02U) != 0U;
    cart->trainer = has_trainer;
    cart->mirroring = ((flags6 & 0x08U) != 0U)
                          ? MIRRORING_FOUR_SCREEN
                          : (((flags6 & 0x01U) != 0U) ? MIRRORING_VERTICAL
                                                     : MIRRORING_HORIZONTAL);

    cart->mapper = mapper_nrom_create(cart);
    if (cart->mapper == NULL) {
        loader_fail(cart, error, ERR_ALLOCATION);
        return NULL;
    }

    if (nes2) {
        log_info("Cartridge dimuat (iNES 2.0): mapper=%d submapper=%u PRG=%zu KiB "
                 "CHR=%zu KiB (%s) mirroring=%s region=%s trainer=%s",
                 mapper_number, (unsigned)submapper, prg_size / 1024U, chr_size / 1024U,
                 cart->chr_is_ram ? "RAM" : "ROM",
                 cart->mirroring == MIRRORING_VERTICAL
                     ? "vertikal"
                     : (cart->mirroring == MIRRORING_FOUR_SCREEN ? "four-screen"
                                                                : "horizontal"),
                 nes_region_is_pal(region) ? "PAL" : "NTSC",
                 has_trainer ? "ya" : "tidak");
    } else {
        log_info("Cartridge dimuat (iNES 1.0): mapper=%d PRG=%zu KiB CHR=%zu KiB (%s) "
                 "mirroring=%s region=%s trainer=%s",
                 mapper_number, prg_size / 1024U, chr_size / 1024U,
                 cart->chr_is_ram ? "RAM" : "ROM",
                 cart->mirroring == MIRRORING_VERTICAL
                     ? "vertikal"
                     : (cart->mirroring == MIRRORING_FOUR_SCREEN ? "four-screen"
                                                                : "horizontal"),
                 nes_region_is_pal(region) ? "PAL" : "NTSC",
                 has_trainer ? "ya" : "tidak");
    }

    return cart;
}

Cartridge *cartridge_load_file(const char *path, ErrorCode *error)
{
    FileData file = {0};
    ErrorCode result = ERR_OK;
    Cartridge *cart = NULL;

    if (error != NULL) {
        *error = ERR_OK;
    }

    if (path == NULL) {
        if (error != NULL) {
            *error = ERR_INVALID_ARGUMENT;
        }
        return NULL;
    }

    result = file_read_all(path, &file);
    if (result != ERR_OK) {
        if (error != NULL) {
            *error = result;
        }
        return NULL;
    }

    cart = cartridge_load_memory(file.data, file.size, error);
    file_data_free(&file);
    return cart;
}
