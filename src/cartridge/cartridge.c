/*
 * cartridge.c - Siklus hidup cartridge dan penerusan akses ke mapper.
 */
#include "cartridge/cartridge.h"

#include "cartridge/cartridge_internal.h"
#include "cartridge/mapper.h"
#include "util/log.h"

#include <stdlib.h>
#include <string.h>

Cartridge *cartridge_allocate(void)
{
    Cartridge *cart = (Cartridge *)calloc(1, sizeof(*cart));

    if (cart == NULL) {
        log_error("Alokasi memori Cartridge gagal.");
    }
    return cart;
}

void cartridge_destroy(Cartridge *cartridge)
{
    if (cartridge == NULL) {
        return;
    }

    mapper_destroy(cartridge->mapper);
    free(cartridge->prg_rom);
    free(cartridge->chr_mem);
    free(cartridge);
}

uint8_t cartridge_cpu_read(Cartridge *cartridge, uint16_t address)
{
    if (cartridge == NULL) {
        return 0;
    }
    return mapper_cpu_read(cartridge->mapper, address);
}

void cartridge_cpu_write(Cartridge *cartridge, uint16_t address, uint8_t value)
{
    if (cartridge == NULL) {
        return;
    }
    mapper_cpu_write(cartridge->mapper, address, value);
}

uint8_t cartridge_ppu_read(Cartridge *cartridge, uint16_t address)
{
    if (cartridge == NULL) {
        return 0;
    }
    return mapper_ppu_read(cartridge->mapper, address);
}

void cartridge_ppu_write(Cartridge *cartridge, uint16_t address, uint8_t value)
{
    if (cartridge == NULL) {
        return;
    }
    mapper_ppu_write(cartridge->mapper, address, value);
}

int cartridge_mapper_number(const Cartridge *cartridge)
{
    return (cartridge != NULL) ? cartridge->mapper_number : -1;
}

uint8_t cartridge_submapper(const Cartridge *cartridge)
{
    return (cartridge != NULL) ? cartridge->submapper : 0U;
}

bool cartridge_is_nes2(const Cartridge *cartridge)
{
    return cartridge != NULL && cartridge->nes2;
}

Region cartridge_region(const Cartridge *cartridge)
{
    return (cartridge != NULL) ? cartridge->region : REGION_NTSC;
}

Mirroring cartridge_mirroring(const Cartridge *cartridge)
{
    return (cartridge != NULL) ? cartridge->mirroring : MIRRORING_HORIZONTAL;
}

size_t cartridge_prg_rom_size(const Cartridge *cartridge)
{
    return (cartridge != NULL) ? cartridge->prg_rom_size : 0U;
}

size_t cartridge_chr_size(const Cartridge *cartridge)
{
    return (cartridge != NULL) ? cartridge->chr_size : 0U;
}

bool cartridge_chr_is_ram(const Cartridge *cartridge)
{
    return cartridge != NULL && cartridge->chr_is_ram;
}

bool cartridge_has_battery(const Cartridge *cartridge)
{
    return cartridge != NULL && cartridge->battery;
}

bool cartridge_has_trainer(const Cartridge *cartridge)
{
    return cartridge != NULL && cartridge->trainer;
}
