/*
 * mapper_nrom.c - Mapper 0 (NROM).
 *
 * Peta memori:
 *   $6000-$7FFF : PRG RAM 8 KiB (bila ada)
 *   $8000-$FFFF : PRG ROM; ukuran 16 KiB otomatis tercermin dua kali
 *   $0000-$1FFF : CHR ROM atau CHR RAM 8 KiB
 */
#include "cartridge/mapper_nrom.h"

#include "cartridge_internal.h"

static uint8_t nrom_cpu_read(void *context, uint16_t address)
{
    Cartridge *cart = (Cartridge *)context;

    if (cart == NULL) {
        return 0;
    }

    if (address >= 0x6000U && address < 0x8000U) {
        return cart->prg_ram[address - 0x6000U];
    }

    if (address >= 0x8000U && cart->prg_rom != NULL && cart->prg_rom_size > 0U) {
        /* Mask membuat PRG 16 KiB terpetakan di $8000-$BFFF dan $C000-$FFFF. */
        size_t index = (size_t)(address - 0x8000U) & (cart->prg_rom_size - 1U);
        return cart->prg_rom[index];
    }

    return 0;
}

static void nrom_cpu_write(void *context, uint16_t address, uint8_t value)
{
    Cartridge *cart = (Cartridge *)context;

    if (cart == NULL) {
        return;
    }

    if (address >= 0x6000U && address < 0x8000U) {
        cart->prg_ram[address - 0x6000U] = value;
    }
    /* Penulisan ke ROM diabaikan, sesuai perilaku NROM. */
}

static uint8_t nrom_ppu_read(void *context, uint16_t address)
{
    Cartridge *cart = (Cartridge *)context;
    size_t index = 0;

    if (cart == NULL || cart->chr_mem == NULL || cart->chr_size == 0U) {
        return 0;
    }

    index = (size_t)(address & 0x1FFFU) % cart->chr_size;
    return cart->chr_mem[index];
}

static void nrom_ppu_write(void *context, uint16_t address, uint8_t value)
{
    Cartridge *cart = (Cartridge *)context;
    size_t index = 0;

    if (cart == NULL || !cart->chr_is_ram || cart->chr_mem == NULL || cart->chr_size == 0U) {
        return;
    }

    index = (size_t)(address & 0x1FFFU) % cart->chr_size;
    cart->chr_mem[index] = value;
}

static const MapperOps g_nrom_ops = {
    .cpu_read = nrom_cpu_read,
    .cpu_write = nrom_cpu_write,
    .ppu_read = nrom_ppu_read,
    .ppu_write = nrom_ppu_write,
    .destroy = NULL
};

Mapper *mapper_nrom_create(Cartridge *cartridge)
{
    if (cartridge == NULL) {
        return NULL;
    }

    return mapper_create(&g_nrom_ops, cartridge);
}
