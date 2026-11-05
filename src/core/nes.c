/*
 * nes.c - Komposisi dan scheduler NES.
 *
 * Scheduler menjalankan CPU dalam satuan instruksi. Setelah satu instruksi,
 * PPU dimajukan dengan rasio region (3,0 dot NTSC atau 3,2 dot PAL) per
 * siklus CPU dan APU satu langkah per siklus. Rasio PAL yang pecahan dijaga
 * dengan akumulator integer 1/5 dot, tanpa thread atau timer host.
 */
#include "core/nes.h"

#include "cpu/cpu_2a03.h"
#include "util/log.h"

#include <stdlib.h>

struct Nes {
    CPU *cpu;
    PPU *ppu;
    APU *apu;
    CPUBus *bus;
    Cartridge *cartridge;
    Controller controller1;
    Controller controller2;
    Region region;          /* Region efektif yang sedang berjalan. */
    Region requested_region;/* REGION_AUTO atau region yang dipaksa pengguna. */
    bool region_auto;
    unsigned ppu_dot_accumulator; /* Sisa dot dalam satuan 1/5 dot. */
    bool debug;
};

static uint8_t nes_bus_read_callback(void *context, uint16_t address)
{
    return cpu_bus_read((CPUBus *)context, address);
}

static void nes_bus_write_callback(void *context, uint16_t address, uint8_t value)
{
    cpu_bus_write((CPUBus *)context, address, value);
}

static uint8_t nes_dmc_read_callback(void *context, uint16_t address)
{
    return cpu_bus_read((CPUBus *)context, address);
}

Nes *nes_create(uint32_t audio_sample_rate, Region region)
{
    Nes *nes = (Nes *)calloc(1, sizeof(*nes));
    Region initial_region = REGION_NTSC;

    if (nes == NULL) {
        log_error("Alokasi memori NES gagal.");
        return NULL;
    }

    nes->requested_region = region;
    nes->region_auto = (region == REGION_AUTO);
    initial_region = nes->region_auto ? REGION_NTSC : region;
    nes->region = initial_region;

    controller_init(&nes->controller1);
    controller_init(&nes->controller2);

    nes->apu = apu_create(audio_sample_rate, initial_region);
    nes->ppu = ppu_create(NULL, initial_region);
    nes->bus = cpu_bus_create();

    if (nes->apu == NULL || nes->ppu == NULL || nes->bus == NULL) {
        nes_destroy(nes);
        return NULL;
    }

    nes->cpu = cpu_create(nes_bus_read_callback, nes_bus_write_callback, nes->bus);
    if (nes->cpu == NULL) {
        nes_destroy(nes);
        return NULL;
    }

    cpu_bus_connect_ppu(nes->bus, nes->ppu);
    cpu_bus_connect_apu(nes->bus, nes->apu);
    cpu_bus_connect_controllers(nes->bus, &nes->controller1, &nes->controller2);
    apu_set_dmc_reader(nes->apu, nes_dmc_read_callback, nes->bus);

    cpu_reset(nes->cpu);
    return nes;
}

void nes_destroy(Nes *nes)
{
    if (nes == NULL) {
        return;
    }

    cpu_destroy(nes->cpu);
    cpu_bus_destroy(nes->bus);
    ppu_destroy(nes->ppu);
    apu_destroy(nes->apu);
    cartridge_destroy(nes->cartridge);
    free(nes);
}

static void nes_resolve_region(Nes *nes, const Cartridge *cartridge)
{
    Region resolved = REGION_NTSC;

    if (nes == NULL) {
        return;
    }

    resolved = nes->region_auto ? cartridge_region(cartridge) : nes->requested_region;
    if (resolved == REGION_AUTO) {
        resolved = REGION_NTSC;
    }

    if (resolved != nes->region) {
        nes->region = resolved;
        ppu_set_region(nes->ppu, resolved);
        apu_set_region(nes->apu, resolved);
    }

    log_info("Region timing: %s%s.", nes_region_is_pal(resolved) ? "PAL" : "NTSC",
             nes->region_auto ? " (otomatis dari ROM)" : " (dipilih pengguna)");
}

static void nes_attach_cartridge(Nes *nes, Cartridge *cartridge)
{
    cartridge_destroy(nes->cartridge);
    nes->cartridge = cartridge;
    ppu_set_cartridge(nes->ppu, cartridge);
    cpu_bus_connect_cartridge(nes->bus, cartridge);
    nes_resolve_region(nes, cartridge);
    nes_reset(nes);
}

ErrorCode nes_load_rom_data(Nes *nes, const uint8_t *data, size_t size)
{
    ErrorCode error = ERR_OK;
    Cartridge *cartridge = NULL;

    if (nes == NULL || data == NULL || size == 0U) {
        return ERR_INVALID_ARGUMENT;
    }

    cartridge = cartridge_load_memory(data, size, &error);
    if (cartridge == NULL) {
        return (error != ERR_OK) ? error : ERR_INES_FORMAT;
    }

    nes_attach_cartridge(nes, cartridge);
    return ERR_OK;
}

ErrorCode nes_load_rom(Nes *nes, const char *path)
{
    ErrorCode error = ERR_OK;
    Cartridge *cartridge = NULL;

    if (nes == NULL || path == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    cartridge = cartridge_load_file(path, &error);
    if (cartridge == NULL) {
        return (error != ERR_OK) ? error : ERR_IO;
    }

    nes_attach_cartridge(nes, cartridge);
    log_info("ROM siap dijalankan.");
    return ERR_OK;
}

void nes_reset(Nes *nes)
{
    if (nes == NULL) {
        return;
    }

    cpu_bus_reset(nes->bus);
    cpu_reset(nes->cpu);
    ppu_reset(nes->ppu);
    apu_reset(nes->apu);
    nes->ppu_dot_accumulator = 0;
}

void nes_run_frame(Nes *nes)
{
    if (nes == NULL) {
        return;
    }

    ppu_clear_frame_complete(nes->ppu);

    while (!ppu_frame_complete(nes->ppu)) {
        uint32_t cycles = 0;
        int stall = 0;
        uint32_t i = 0;
        unsigned dot_step = nes_region_dot_numerator(nes->region);

        /* Interrupt diperiksa di antara instruksi. */
        if (ppu_take_nmi(nes->ppu)) {
            cycles += (uint32_t)cpu_nmi(nes->cpu);
        }
        if (apu_irq_pending(nes->apu)) {
            cycles += (uint32_t)cpu_irq(nes->cpu);
        }

        cycles += (uint32_t)cpu_step(nes->cpu);
        stall = cpu_bus_take_stall(nes->bus);
        if (stall > 0) {
            cycles += (uint32_t)stall;
        }

        cpu_bus_tick(nes->bus, (int)cycles);

        /*
         * Rasio hardware: 1 siklus CPU = 15/5 dot (NTSC) atau 16/5 dot (PAL)
         * = 1 langkah APU. Akumulator menyimpan sisa dalam satuan 1/5 dot.
         */
        for (i = 0; i < cycles; ++i) {
            nes->ppu_dot_accumulator += dot_step;
            while (nes->ppu_dot_accumulator >= NES_REGION_DOT_SCALE) {
                ppu_clock(nes->ppu);
                nes->ppu_dot_accumulator -= NES_REGION_DOT_SCALE;
            }
            apu_clock(nes->apu);
        }
    }

    if (nes->debug && (ppu_frame_count(nes->ppu) % 60U) == 0U) {
        log_info("Frame %llu | PC=$%04X A=$%02X X=$%02X Y=$%02X SP=$%02X P=$%02X "
                 "| PPU %d:%d | mapper=%d",
                 (unsigned long long)ppu_frame_count(nes->ppu), nes->cpu->pc, nes->cpu->a,
                 nes->cpu->x, nes->cpu->y, nes->cpu->sp, nes->cpu->p,
                 ppu_scanline(nes->ppu), ppu_dot(nes->ppu),
                 cartridge_mapper_number(nes->cartridge));
    }
}

void nes_set_buttons(Nes *nes, int port, uint8_t buttons)
{
    if (nes == NULL) {
        return;
    }

    if (port == 0) {
        controller_set_buttons(&nes->controller1, buttons);
    } else if (port == 1) {
        controller_set_buttons(&nes->controller2, buttons);
    }
}

const uint8_t *nes_framebuffer(const Nes *nes)
{
    return (nes != NULL) ? ppu_framebuffer(nes->ppu) : NULL;
}

size_t nes_read_audio(Nes *nes, int16_t *out, size_t max_samples)
{
    return (nes != NULL) ? apu_read_samples(nes->apu, out, max_samples) : 0U;
}

void nes_set_debug(Nes *nes, bool enabled)
{
    if (nes != NULL) {
        nes->debug = enabled;
    }
}

bool nes_debug_enabled(const Nes *nes)
{
    return nes != NULL && nes->debug;
}

Region nes_region(const Nes *nes)
{
    return (nes != NULL) ? nes->region : REGION_NTSC;
}

double nes_frame_rate(const Nes *nes)
{
    return nes_region_frame_rate((nes != NULL) ? nes->region : REGION_NTSC);
}

CPU *nes_cpu(Nes *nes)
{
    return (nes != NULL) ? nes->cpu : NULL;
}

PPU *nes_ppu(Nes *nes)
{
    return (nes != NULL) ? nes->ppu : NULL;
}

APU *nes_apu(Nes *nes)
{
    return (nes != NULL) ? nes->apu : NULL;
}

CPUBus *nes_bus(Nes *nes)
{
    return (nes != NULL) ? nes->bus : NULL;
}

Cartridge *nes_cartridge(Nes *nes)
{
    return (nes != NULL) ? nes->cartridge : NULL;
}
