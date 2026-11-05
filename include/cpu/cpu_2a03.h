/*
 * cpu_2a03.h - Antarmuka CPU Ricoh RP2A03.
 *
 * RP2A03 adalah varian MOS 6502 tanpa mode desimal yang dipakai NES.
 * CPU ini tidak memiliki jalur data langsung ke memori; semua akses lewat
 * dua fungsi callback (read/write) yang disediakan CPU Bus. Dengan cara ini
 * CPU tidak perlu mengetahui PPU, APU, atau cartridge.
 */
#ifndef TINYNES_CPU_CPU_2A03_H
#define TINYNES_CPU_CPU_2A03_H

#include "util/types.h"

typedef uint8_t (*CPUReadFn)(void *context, uint16_t address);
typedef void (*CPUWriteFn)(void *context, uint16_t address, uint8_t value);

/* Bit processor status. */
typedef enum CPUFlag {
    CPU_FLAG_C = 0x01, /* Carry */
    CPU_FLAG_Z = 0x02, /* Zero */
    CPU_FLAG_I = 0x04, /* Interrupt disable */
    CPU_FLAG_D = 0x08, /* Decimal (tidak berfungsi pada 2A03) */
    CPU_FLAG_B = 0x10, /* Break (hanya ada di salinan stack) */
    CPU_FLAG_U = 0x20, /* Unused, selalu 1 */
    CPU_FLAG_V = 0x40, /* Overflow */
    CPU_FLAG_N = 0x80  /* Negative */
} CPUFlag;

typedef struct CPU {
    /* Register yang terlihat programmer. */
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint16_t pc;
    uint8_t p;

    /* Koneksi ke bus. context biasanya berupa CPUBus *. */
    CPUReadFn read;
    CPUWriteFn write;
    void *context;

    /* Statistik dan status eksekusi. */
    uint64_t total_cycles;
    uint8_t current_opcode;
    uint8_t current_cycles;
    int extra_cycles; /* Penalti siklus yang ditambahkan instruksi tertentu. */
    bool jammed;      /* true bila opcode KIL tidak stabil ditemui. */
    bool warned_opcode[256]; /* Cegah log berulang untuk opcode tak dikenal. */
} CPU;

/* Membuat CPU. Callback read/write wajib tidak NULL. */
CPU *cpu_create(CPUReadFn read, CPUWriteFn write, void *context);
void cpu_destroy(CPU *cpu);

/* Reset vektor dari $FFFC, SP=$FD, flag I aktif. */
void cpu_reset(CPU *cpu);

/*
 * Jalankan tepat satu instruksi. Mengembalikan jumlah siklus CPU yang
 * dihabiskan instruksi tersebut (belum termasuk stall DMA).
 */
uint8_t cpu_step(CPU *cpu);

/*
 * Interrupt request. NMI selalu diterima; IRQ diabaikan bila flag I = 1.
 * Keduanya mengembalikan jumlah siklus yang dipakai (0 bila diabaikan).
 */
uint8_t cpu_nmi(CPU *cpu);
uint8_t cpu_irq(CPU *cpu);

/* Helper flag untuk modul instruksi dan pengujian. */
bool cpu_flag_get(const CPU *cpu, uint8_t flag);
void cpu_flag_set(CPU *cpu, uint8_t flag, bool value);
void cpu_set_zn(CPU *cpu, uint8_t value);

/* Helper stack, dipakai instruksi dan interrupt. */
void cpu_push(CPU *cpu, uint8_t value);
uint8_t cpu_pop(CPU *cpu);

#endif /* TINYNES_CPU_CPU_2A03_H */
