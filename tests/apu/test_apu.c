/*
 * test_apu.c - Aktivasi kanal, timer, frame counter, dan ring buffer.
 */
#include "../test.h"
#include "../test_helpers.h"

#include "apu/apu.h"

#include <string.h>

static bool samples_have_signal(APU *apu)
{
    int16_t samples[512];
    size_t count = apu_read_samples(apu, samples, 512);
    size_t i = 0;

    for (i = 0; i < count; ++i) {
        if (samples[i] > 500 || samples[i] < -500) {
            return true;
        }
    }
    return false;
}

void test_apu_run(void)
{
    APU *apu = apu_create(44100, REGION_NTSC);
    int16_t samples[8192];
    size_t count = 0;

    CHECK_TRUE(apu != NULL);
    if (apu == NULL) {
        return;
    }

    /* Pulse 1 aktif dan length counter terisi. */
    apu_write_register(apu, 0x4000, 0xBF);
    apu_write_register(apu, 0x4002, 0x80);
    apu_write_register(apu, 0x4003, 0x08);
    apu_write_register(apu, 0x4015, 0x01);
    CHECK_TRUE((apu_read_status(apu) & 0x01) != 0);
    for (int i = 0; i < 2000; ++i) {
        apu_clock(apu);
    }
    CHECK_TRUE(samples_have_signal(apu));

    /* Menonaktifkan kanal menghapus length counter. */
    apu_write_register(apu, 0x4015, 0x00);
    CHECK_EQ_U8(apu_read_status(apu) & 0x01, 0);

    /* Triangle. */
    apu_reset(apu);
    apu_write_register(apu, 0x4008, 0xFF);
    apu_write_register(apu, 0x400A, 0x80);
    apu_write_register(apu, 0x400B, 0x08);
    apu_write_register(apu, 0x4015, 0x04);
    for (int i = 0; i < 4000; ++i) {
        apu_clock(apu);
    }
    CHECK_TRUE(samples_have_signal(apu));

    /* Noise. */
    apu_reset(apu);
    apu_write_register(apu, 0x400C, 0x3F);
    apu_write_register(apu, 0x400E, 0x00);
    apu_write_register(apu, 0x400F, 0x08);
    apu_write_register(apu, 0x4015, 0x08);
    for (int i = 0; i < 4000; ++i) {
        apu_clock(apu);
    }
    CHECK_TRUE(samples_have_signal(apu));

    /* Frame IRQ mode 4-step. */
    apu_reset(apu);
    apu_write_register(apu, 0x4017, 0x00);
    for (int i = 0; i < 29829; ++i) {
        apu_clock(apu);
    }
    CHECK_TRUE(apu_irq_pending(apu));
    CHECK_TRUE(apu_frame_irq(apu));
    CHECK_TRUE((apu_read_status(apu) & 0x40) != 0);
    CHECK_FALSE(apu_irq_pending(apu));

    /* Mode 5-step tidak menghasilkan IRQ. */
    apu_reset(apu);
    apu_write_register(apu, 0x4017, 0x80);
    for (int i = 0; i < 40000; ++i) {
        apu_clock(apu);
    }
    CHECK_FALSE(apu_irq_pending(apu));

    /* Sampling: sekitar 4928 sampel untuk 200.000 siklus CPU. */
    apu_reset(apu);
    for (int i = 0; i < 200000; ++i) {
        apu_clock(apu);
    }
    count = apu_read_samples(apu, samples, 8192);
    CHECK_TRUE(count > 4000);
    CHECK_TRUE(count < 6000);
    CHECK_EQ_INT(apu_read_samples(apu, samples, 8192), 0);

    /* Ring buffer penuh tidak boleh crash; sampel terlama dibuang. */
    for (int i = 0; i < 2000000; ++i) {
        apu_clock(apu);
    }
    count = apu_read_samples(apu, samples, 8192);
    CHECK_EQ_INT(count, 8192);

    CHECK_EQ_INT(apu_sample_rate(apu), 44100);
    apu_destroy(apu);

    /*
     * PAL memakai clock CPU lebih lambat (1.662.607 Hz). Frame counter dan
     * sampling harus mengikuti periode PAL, bukan NTSC.
     */
    {
        APU *pal = apu_create(44100, REGION_PAL);

        CHECK_TRUE(pal != NULL);
        if (pal != NULL) {
            CHECK_EQ_INT(apu_region(pal), REGION_PAL);

            /* Mode 4-step PAL memicu IRQ pada siklus 33253. */
            apu_write_register(pal, 0x4017, 0x00);
            for (int i = 0; i < 33253; ++i) {
                apu_clock(pal);
            }
            CHECK_TRUE(apu_irq_pending(pal));
            CHECK_TRUE(apu_frame_irq(pal));

            /* Sampling PAL menghasilkan sekitar 5305 sampel per 200.000 siklus. */
            apu_reset(pal);
            for (int i = 0; i < 200000; ++i) {
                apu_clock(pal);
            }
            count = apu_read_samples(pal, samples, 8192);
            CHECK_TRUE(count > 4800);
            CHECK_TRUE(count < 5800);

            /* Noise dan DMC memakai tabel periode PAL tanpa crash. */
            apu_reset(pal);
            apu_write_register(pal, 0x400C, 0x3F);
            apu_write_register(pal, 0x400E, 0x0F);
            apu_write_register(pal, 0x400F, 0x08);
            apu_write_register(pal, 0x4015, 0x08);
            apu_write_register(pal, 0x4010, 0x0F);
            for (int i = 0; i < 4000; ++i) {
                apu_clock(pal);
            }

            apu_destroy(pal);
        }
    }
}
