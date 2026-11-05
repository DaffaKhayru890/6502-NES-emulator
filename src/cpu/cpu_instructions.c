/*
 * cpu_instructions.c - Semantik instruksi dan tabel opcode 6502.
 *
 * Desain tabel:
 * Setiap opcode dipetakan ke satu OpcodeInfo berisi nama, addressing mode,
 * fungsi operasi, jumlah siklus dasar, dan apakah ada penalti page crossing.
 * Pendekatan ini menghindari switch raksasa dan memisahkan decoding
 * (cpu_addressing.c) dari eksekusi (file ini).
 */
#include "cpu_internal.h"

#include "util/log.h"

#include <string.h>

/* ------------------------------------------------------------------------- */
/* Helper aritmetika dan flag                                                */
/* ------------------------------------------------------------------------- */

static void do_adc(CPU *cpu, uint8_t value)
{
    uint16_t sum = (uint16_t)((uint16_t)cpu->a + (uint16_t)value +
                              (uint16_t)(cpu_flag_get(cpu, CPU_FLAG_C) ? 1U : 0U));
    uint8_t result = (uint8_t)(sum & 0xFFU);

    cpu_flag_set(cpu, CPU_FLAG_C, sum > 0xFFU);
    cpu_flag_set(cpu, CPU_FLAG_V,
                 ((uint8_t)(~(cpu->a ^ value) & (cpu->a ^ result) & 0x80U)) != 0U);
    cpu->a = result;
    cpu_set_zn(cpu, cpu->a);
}

static void do_compare(CPU *cpu, uint8_t reg, uint8_t value)
{
    uint8_t result = (uint8_t)(reg - value);

    cpu_flag_set(cpu, CPU_FLAG_C, reg >= value);
    cpu_set_zn(cpu, result);
}

static uint8_t do_asl(CPU *cpu, uint8_t value)
{
    cpu_flag_set(cpu, CPU_FLAG_C, (value & 0x80U) != 0U);
    value = (uint8_t)(value << 1);
    cpu_set_zn(cpu, value);
    return value;
}

static uint8_t do_lsr(CPU *cpu, uint8_t value)
{
    cpu_flag_set(cpu, CPU_FLAG_C, (value & 0x01U) != 0U);
    value = (uint8_t)(value >> 1);
    cpu_set_zn(cpu, value);
    return value;
}

static uint8_t do_rol(CPU *cpu, uint8_t value)
{
    uint8_t carry_in = (uint8_t)(cpu_flag_get(cpu, CPU_FLAG_C) ? 1U : 0U);
    uint8_t carry_out = (uint8_t)((value & 0x80U) != 0U);

    value = (uint8_t)((value << 1) | carry_in);
    cpu_flag_set(cpu, CPU_FLAG_C, carry_out != 0U);
    cpu_set_zn(cpu, value);
    return value;
}

static uint8_t do_ror(CPU *cpu, uint8_t value)
{
    uint8_t carry_in = (uint8_t)(cpu_flag_get(cpu, CPU_FLAG_C) ? 0x80U : 0x00U);
    uint8_t carry_out = (uint8_t)(value & 0x01U);

    value = (uint8_t)((value >> 1) | carry_in);
    cpu_flag_set(cpu, CPU_FLAG_C, carry_out != 0U);
    cpu_set_zn(cpu, value);
    return value;
}

/* ------------------------------------------------------------------------- */
/* Load / store                                                              */
/* ------------------------------------------------------------------------- */

static void op_lda(CPU *cpu, uint16_t address)
{
    cpu->a = cpu_bus_read8(cpu, address);
    cpu_set_zn(cpu, cpu->a);
}

static void op_ldx(CPU *cpu, uint16_t address)
{
    cpu->x = cpu_bus_read8(cpu, address);
    cpu_set_zn(cpu, cpu->x);
}

static void op_ldy(CPU *cpu, uint16_t address)
{
    cpu->y = cpu_bus_read8(cpu, address);
    cpu_set_zn(cpu, cpu->y);
}

static void op_sta(CPU *cpu, uint16_t address)
{
    cpu_bus_write8(cpu, address, cpu->a);
}

static void op_stx(CPU *cpu, uint16_t address)
{
    cpu_bus_write8(cpu, address, cpu->x);
}

static void op_sty(CPU *cpu, uint16_t address)
{
    cpu_bus_write8(cpu, address, cpu->y);
}

/* ------------------------------------------------------------------------- */
/* Aritmetika, logika, perbandingan                                          */
/* ------------------------------------------------------------------------- */

static void op_adc(CPU *cpu, uint16_t address)
{
    do_adc(cpu, cpu_bus_read8(cpu, address));
}

static void op_sbc(CPU *cpu, uint16_t address)
{
    /* Pengurangan biner: tambahkan komplemen + carry. Flag D diabaikan. */
    do_adc(cpu, (uint8_t)~cpu_bus_read8(cpu, address));
}

static void op_and(CPU *cpu, uint16_t address)
{
    cpu->a = (uint8_t)(cpu->a & cpu_bus_read8(cpu, address));
    cpu_set_zn(cpu, cpu->a);
}

static void op_ora(CPU *cpu, uint16_t address)
{
    cpu->a = (uint8_t)(cpu->a | cpu_bus_read8(cpu, address));
    cpu_set_zn(cpu, cpu->a);
}

static void op_eor(CPU *cpu, uint16_t address)
{
    cpu->a = (uint8_t)(cpu->a ^ cpu_bus_read8(cpu, address));
    cpu_set_zn(cpu, cpu->a);
}

static void op_cmp(CPU *cpu, uint16_t address)
{
    do_compare(cpu, cpu->a, cpu_bus_read8(cpu, address));
}

static void op_cpx(CPU *cpu, uint16_t address)
{
    do_compare(cpu, cpu->x, cpu_bus_read8(cpu, address));
}

static void op_cpy(CPU *cpu, uint16_t address)
{
    do_compare(cpu, cpu->y, cpu_bus_read8(cpu, address));
}

static void op_bit(CPU *cpu, uint16_t address)
{
    uint8_t value = cpu_bus_read8(cpu, address);

    cpu_flag_set(cpu, CPU_FLAG_Z, (uint8_t)(cpu->a & value) == 0U);
    cpu_flag_set(cpu, CPU_FLAG_N, (value & 0x80U) != 0U);
    cpu_flag_set(cpu, CPU_FLAG_V, (value & 0x40U) != 0U);
}

/* ------------------------------------------------------------------------- */
/* Increment / decrement                                                     */
/* ------------------------------------------------------------------------- */

static void op_inc(CPU *cpu, uint16_t address)
{
    uint8_t value = (uint8_t)(cpu_bus_read8(cpu, address) + 1U);
    cpu_bus_write8(cpu, address, value);
    cpu_set_zn(cpu, value);
}

static void op_dec(CPU *cpu, uint16_t address)
{
    uint8_t value = (uint8_t)(cpu_bus_read8(cpu, address) - 1U);
    cpu_bus_write8(cpu, address, value);
    cpu_set_zn(cpu, value);
}

static void op_inx(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->x = (uint8_t)(cpu->x + 1U);
    cpu_set_zn(cpu, cpu->x);
}

static void op_iny(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->y = (uint8_t)(cpu->y + 1U);
    cpu_set_zn(cpu, cpu->y);
}

static void op_dex(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->x = (uint8_t)(cpu->x - 1U);
    cpu_set_zn(cpu, cpu->x);
}

static void op_dey(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->y = (uint8_t)(cpu->y - 1U);
    cpu_set_zn(cpu, cpu->y);
}

/* ------------------------------------------------------------------------- */
/* Shift / rotate                                                            */
/* ------------------------------------------------------------------------- */

static void op_asl_acc(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->a = do_asl(cpu, cpu->a);
}

static void op_asl(CPU *cpu, uint16_t address)
{
    cpu_bus_write8(cpu, address, do_asl(cpu, cpu_bus_read8(cpu, address)));
}

static void op_lsr_acc(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->a = do_lsr(cpu, cpu->a);
}

static void op_lsr(CPU *cpu, uint16_t address)
{
    cpu_bus_write8(cpu, address, do_lsr(cpu, cpu_bus_read8(cpu, address)));
}

static void op_rol_acc(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->a = do_rol(cpu, cpu->a);
}

static void op_rol(CPU *cpu, uint16_t address)
{
    cpu_bus_write8(cpu, address, do_rol(cpu, cpu_bus_read8(cpu, address)));
}

static void op_ror_acc(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->a = do_ror(cpu, cpu->a);
}

static void op_ror(CPU *cpu, uint16_t address)
{
    cpu_bus_write8(cpu, address, do_ror(cpu, cpu_bus_read8(cpu, address)));
}

/* ------------------------------------------------------------------------- */
/* Transfer register                                                         */
/* ------------------------------------------------------------------------- */

static void op_tax(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->x = cpu->a;
    cpu_set_zn(cpu, cpu->x);
}

static void op_tay(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->y = cpu->a;
    cpu_set_zn(cpu, cpu->y);
}

static void op_txa(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->a = cpu->x;
    cpu_set_zn(cpu, cpu->a);
}

static void op_tya(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->a = cpu->y;
    cpu_set_zn(cpu, cpu->a);
}

static void op_tsx(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->x = cpu->sp;
    cpu_set_zn(cpu, cpu->x);
}

static void op_txs(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->sp = cpu->x; /* TXS tidak mengubah flag. */
}

/* ------------------------------------------------------------------------- */
/* Stack                                                                     */
/* ------------------------------------------------------------------------- */

static void op_pha(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_push(cpu, cpu->a);
}

static void op_php(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_push(cpu, (uint8_t)(cpu->p | CPU_FLAG_B | CPU_FLAG_U));
}

static void op_pla(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->a = cpu_pop(cpu);
    cpu_set_zn(cpu, cpu->a);
}

static void op_plp(CPU *cpu, uint16_t address)
{
    (void)address;
    /* Bit B tidak nyata: selalu bersihkan, lalu paksa U = 1. */
    cpu->p = (uint8_t)((cpu_pop(cpu) & (uint8_t)~CPU_FLAG_B) | CPU_FLAG_U);
}

/* ------------------------------------------------------------------------- */
/* Branch                                                                    */
/* ------------------------------------------------------------------------- */

static void branch_if(CPU *cpu, uint16_t target, bool condition)
{
    if (!condition) {
        return;
    }

    cpu->extra_cycles += 1;
    if ((cpu->pc & 0xFF00U) != (target & 0xFF00U)) {
        cpu->extra_cycles += 1;
    }
    cpu->pc = target;
}

static void op_bcc(CPU *cpu, uint16_t target)
{
    branch_if(cpu, target, !cpu_flag_get(cpu, CPU_FLAG_C));
}

static void op_bcs(CPU *cpu, uint16_t target)
{
    branch_if(cpu, target, cpu_flag_get(cpu, CPU_FLAG_C));
}

static void op_beq(CPU *cpu, uint16_t target)
{
    branch_if(cpu, target, cpu_flag_get(cpu, CPU_FLAG_Z));
}

static void op_bne(CPU *cpu, uint16_t target)
{
    branch_if(cpu, target, !cpu_flag_get(cpu, CPU_FLAG_Z));
}

static void op_bmi(CPU *cpu, uint16_t target)
{
    branch_if(cpu, target, cpu_flag_get(cpu, CPU_FLAG_N));
}

static void op_bpl(CPU *cpu, uint16_t target)
{
    branch_if(cpu, target, !cpu_flag_get(cpu, CPU_FLAG_N));
}

static void op_bvc(CPU *cpu, uint16_t target)
{
    branch_if(cpu, target, !cpu_flag_get(cpu, CPU_FLAG_V));
}

static void op_bvs(CPU *cpu, uint16_t target)
{
    branch_if(cpu, target, cpu_flag_get(cpu, CPU_FLAG_V));
}

/* ------------------------------------------------------------------------- */
/* Control                                                                   */
/* ------------------------------------------------------------------------- */

static void op_jmp(CPU *cpu, uint16_t address)
{
    cpu->pc = address;
}

static void op_jsr(CPU *cpu, uint16_t address)
{
    uint16_t return_address = (uint16_t)(cpu->pc - 1U);

    cpu_push(cpu, (uint8_t)(return_address >> 8));
    cpu_push(cpu, (uint8_t)(return_address & 0x00FFU));
    cpu->pc = address;
}

static void op_rts(CPU *cpu, uint16_t address)
{
    uint8_t low = cpu_pop(cpu);
    uint8_t high = cpu_pop(cpu);

    (void)address;
    cpu->pc = (uint16_t)(((uint16_t)high << 8) | (uint16_t)low);
    cpu->pc = (uint16_t)(cpu->pc + 1U);
}

static void op_brk(CPU *cpu, uint16_t address)
{
    (void)address;
    /* BRK adalah instruksi 2 byte: lewati byte padding. */
    cpu->pc = (uint16_t)(cpu->pc + 1U);
    cpu_push(cpu, (uint8_t)(cpu->pc >> 8));
    cpu_push(cpu, (uint8_t)(cpu->pc & 0x00FFU));
    cpu_push(cpu, (uint8_t)(cpu->p | CPU_FLAG_B | CPU_FLAG_U));
    cpu_flag_set(cpu, CPU_FLAG_I, true);
    cpu->pc = cpu_read16(cpu, 0xFFFEU);
}

static void op_rti(CPU *cpu, uint16_t address)
{
    uint8_t low = 0;
    uint8_t high = 0;

    (void)address;
    cpu->p = (uint8_t)((cpu_pop(cpu) & (uint8_t)~CPU_FLAG_B) | CPU_FLAG_U);
    low = cpu_pop(cpu);
    high = cpu_pop(cpu);
    cpu->pc = (uint16_t)(((uint16_t)high << 8) | (uint16_t)low);
}

static void op_nop(CPU *cpu, uint16_t address)
{
    (void)cpu;
    (void)address;
}

static void op_kil(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu->jammed = true;
    log_warn("Opcode KIL/JAM dieksekusi di PC=$%04X; CPU dihentikan.", cpu->pc);
}

/* ------------------------------------------------------------------------- */
/* Flag                                                                      */
/* ------------------------------------------------------------------------- */

static void op_clc(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_flag_set(cpu, CPU_FLAG_C, false);
}

static void op_sec(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_flag_set(cpu, CPU_FLAG_C, true);
}

static void op_cld(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_flag_set(cpu, CPU_FLAG_D, false); /* Tidak ada efek aritmetika di 2A03. */
}

static void op_sed(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_flag_set(cpu, CPU_FLAG_D, true);
}

static void op_cli(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_flag_set(cpu, CPU_FLAG_I, false);
}

static void op_sei(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_flag_set(cpu, CPU_FLAG_I, true);
}

static void op_clv(CPU *cpu, uint16_t address)
{
    (void)address;
    cpu_flag_set(cpu, CPU_FLAG_V, false);
}

/* ------------------------------------------------------------------------- */
/* Opcode tidak resmi yang umum dipakai                                      */
/* ------------------------------------------------------------------------- */

static void op_lax(CPU *cpu, uint16_t address)
{
    cpu->a = cpu_bus_read8(cpu, address);
    cpu->x = cpu->a;
    cpu_set_zn(cpu, cpu->a);
}

static void op_sax(CPU *cpu, uint16_t address)
{
    cpu_bus_write8(cpu, address, (uint8_t)(cpu->a & cpu->x));
}

static void op_dcp(CPU *cpu, uint16_t address)
{
    uint8_t value = (uint8_t)(cpu_bus_read8(cpu, address) - 1U);

    cpu_bus_write8(cpu, address, value);
    do_compare(cpu, cpu->a, value);
}

static void op_isc(CPU *cpu, uint16_t address)
{
    uint8_t value = (uint8_t)(cpu_bus_read8(cpu, address) + 1U);

    cpu_bus_write8(cpu, address, value);
    do_adc(cpu, (uint8_t)~value);
}

static void op_slo(CPU *cpu, uint16_t address)
{
    uint8_t value = do_asl(cpu, cpu_bus_read8(cpu, address));

    cpu_bus_write8(cpu, address, value);
    cpu->a = (uint8_t)(cpu->a | value);
    cpu_set_zn(cpu, cpu->a);
}

static void op_rla(CPU *cpu, uint16_t address)
{
    uint8_t value = do_rol(cpu, cpu_bus_read8(cpu, address));

    cpu_bus_write8(cpu, address, value);
    cpu->a = (uint8_t)(cpu->a & value);
    cpu_set_zn(cpu, cpu->a);
}

static void op_sre(CPU *cpu, uint16_t address)
{
    uint8_t value = do_lsr(cpu, cpu_bus_read8(cpu, address));

    cpu_bus_write8(cpu, address, value);
    cpu->a = (uint8_t)(cpu->a ^ value);
    cpu_set_zn(cpu, cpu->a);
}

static void op_rra(CPU *cpu, uint16_t address)
{
    uint8_t value = do_ror(cpu, cpu_bus_read8(cpu, address));

    cpu_bus_write8(cpu, address, value);
    do_adc(cpu, value);
}

static void op_anc(CPU *cpu, uint16_t address)
{
    cpu->a = (uint8_t)(cpu->a & cpu_bus_read8(cpu, address));
    cpu_set_zn(cpu, cpu->a);
    cpu_flag_set(cpu, CPU_FLAG_C, cpu_flag_get(cpu, CPU_FLAG_N));
}

static void op_alr(CPU *cpu, uint16_t address)
{
    cpu->a = (uint8_t)(cpu->a & cpu_bus_read8(cpu, address));
    cpu->a = do_lsr(cpu, cpu->a);
}

static void op_arr(CPU *cpu, uint16_t address)
{
    uint8_t value = (uint8_t)(cpu->a & cpu_bus_read8(cpu, address));

    cpu->a = (uint8_t)((value >> 1) | (cpu_flag_get(cpu, CPU_FLAG_C) ? 0x80U : 0x00U));
    cpu_set_zn(cpu, cpu->a);
    cpu_flag_set(cpu, CPU_FLAG_C, (cpu->a & 0x40U) != 0U);
    cpu_flag_set(cpu, CPU_FLAG_V, ((cpu->a ^ (cpu->a << 1)) & 0x40U) != 0U);
}

static void op_sbx(CPU *cpu, uint16_t address)
{
    uint8_t base = (uint8_t)(cpu->a & cpu->x);
    uint8_t value = cpu_bus_read8(cpu, address);

    cpu->x = (uint8_t)(base - value);
    cpu_flag_set(cpu, CPU_FLAG_C, base >= value);
    cpu_set_zn(cpu, cpu->x);
}

static void op_las(CPU *cpu, uint16_t address)
{
    uint8_t value = (uint8_t)(cpu_bus_read8(cpu, address) & cpu->sp);

    cpu->a = value;
    cpu->x = value;
    cpu->sp = value;
    cpu_set_zn(cpu, value);
}

/* ------------------------------------------------------------------------- */
/* Tabel opcode                                                              */
/* ------------------------------------------------------------------------- */

#define OP(code, mnemonic, mode, function, base, penalty) \
    [code] = { mnemonic, mode, function, base, penalty }

static const OpcodeInfo g_opcodes[256] = {
    /* 0x00 - 0x0F */
    OP(0x00, "BRK", MODE_IMP, op_brk, 7, false),
    OP(0x01, "ORA", MODE_IZX, op_ora, 6, false),
    OP(0x02, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x03, "SLO", MODE_IZX, op_slo, 8, false),
    OP(0x04, "NOP", MODE_ZP, op_nop, 3, false),
    OP(0x05, "ORA", MODE_ZP, op_ora, 3, false),
    OP(0x06, "ASL", MODE_ZP, op_asl, 5, false),
    OP(0x07, "SLO", MODE_ZP, op_slo, 5, false),
    OP(0x08, "PHP", MODE_IMP, op_php, 3, false),
    OP(0x09, "ORA", MODE_IMM, op_ora, 2, false),
    OP(0x0A, "ASL", MODE_ACC, op_asl_acc, 2, false),
    OP(0x0B, "ANC", MODE_IMM, op_anc, 2, false),
    OP(0x0C, "NOP", MODE_ABS, op_nop, 4, false),
    OP(0x0D, "ORA", MODE_ABS, op_ora, 4, false),
    OP(0x0E, "ASL", MODE_ABS, op_asl, 6, false),
    OP(0x0F, "SLO", MODE_ABS, op_slo, 6, false),

    /* 0x10 - 0x1F */
    OP(0x10, "BPL", MODE_REL, op_bpl, 2, false),
    OP(0x11, "ORA", MODE_IZY, op_ora, 5, true),
    OP(0x12, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x13, "SLO", MODE_IZY, op_slo, 8, false),
    OP(0x14, "NOP", MODE_ZPX, op_nop, 4, false),
    OP(0x15, "ORA", MODE_ZPX, op_ora, 4, false),
    OP(0x16, "ASL", MODE_ZPX, op_asl, 6, false),
    OP(0x17, "SLO", MODE_ZPX, op_slo, 6, false),
    OP(0x18, "CLC", MODE_IMP, op_clc, 2, false),
    OP(0x19, "ORA", MODE_ABY, op_ora, 4, true),
    OP(0x1A, "NOP", MODE_IMP, op_nop, 2, false),
    OP(0x1B, "SLO", MODE_ABY, op_slo, 7, false),
    OP(0x1C, "NOP", MODE_ABX, op_nop, 4, true),
    OP(0x1D, "ORA", MODE_ABX, op_ora, 4, true),
    OP(0x1E, "ASL", MODE_ABX, op_asl, 7, false),
    OP(0x1F, "SLO", MODE_ABX, op_slo, 7, false),

    /* 0x20 - 0x2F */
    OP(0x20, "JSR", MODE_ABS, op_jsr, 6, false),
    OP(0x21, "AND", MODE_IZX, op_and, 6, false),
    OP(0x22, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x23, "RLA", MODE_IZX, op_rla, 8, false),
    OP(0x24, "BIT", MODE_ZP, op_bit, 3, false),
    OP(0x25, "AND", MODE_ZP, op_and, 3, false),
    OP(0x26, "ROL", MODE_ZP, op_rol, 5, false),
    OP(0x27, "RLA", MODE_ZP, op_rla, 5, false),
    OP(0x28, "PLP", MODE_IMP, op_plp, 4, false),
    OP(0x29, "AND", MODE_IMM, op_and, 2, false),
    OP(0x2A, "ROL", MODE_ACC, op_rol_acc, 2, false),
    OP(0x2B, "ANC", MODE_IMM, op_anc, 2, false),
    OP(0x2C, "BIT", MODE_ABS, op_bit, 4, false),
    OP(0x2D, "AND", MODE_ABS, op_and, 4, false),
    OP(0x2E, "ROL", MODE_ABS, op_rol, 6, false),
    OP(0x2F, "RLA", MODE_ABS, op_rla, 6, false),

    /* 0x30 - 0x3F */
    OP(0x30, "BMI", MODE_REL, op_bmi, 2, false),
    OP(0x31, "AND", MODE_IZY, op_and, 5, true),
    OP(0x32, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x33, "RLA", MODE_IZY, op_rla, 8, false),
    OP(0x34, "NOP", MODE_ZPX, op_nop, 4, false),
    OP(0x35, "AND", MODE_ZPX, op_and, 4, false),
    OP(0x36, "ROL", MODE_ZPX, op_rol, 6, false),
    OP(0x37, "RLA", MODE_ZPX, op_rla, 6, false),
    OP(0x38, "SEC", MODE_IMP, op_sec, 2, false),
    OP(0x39, "AND", MODE_ABY, op_and, 4, true),
    OP(0x3A, "NOP", MODE_IMP, op_nop, 2, false),
    OP(0x3B, "RLA", MODE_ABY, op_rla, 7, false),
    OP(0x3C, "NOP", MODE_ABX, op_nop, 4, true),
    OP(0x3D, "AND", MODE_ABX, op_and, 4, true),
    OP(0x3E, "ROL", MODE_ABX, op_rol, 7, false),
    OP(0x3F, "RLA", MODE_ABX, op_rla, 7, false),

    /* 0x40 - 0x4F */
    OP(0x40, "RTI", MODE_IMP, op_rti, 6, false),
    OP(0x41, "EOR", MODE_IZX, op_eor, 6, false),
    OP(0x42, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x43, "SRE", MODE_IZX, op_sre, 8, false),
    OP(0x44, "NOP", MODE_ZP, op_nop, 3, false),
    OP(0x45, "EOR", MODE_ZP, op_eor, 3, false),
    OP(0x46, "LSR", MODE_ZP, op_lsr, 5, false),
    OP(0x47, "SRE", MODE_ZP, op_sre, 5, false),
    OP(0x48, "PHA", MODE_IMP, op_pha, 3, false),
    OP(0x49, "EOR", MODE_IMM, op_eor, 2, false),
    OP(0x4A, "LSR", MODE_ACC, op_lsr_acc, 2, false),
    OP(0x4B, "ALR", MODE_IMM, op_alr, 2, false),
    OP(0x4C, "JMP", MODE_ABS, op_jmp, 3, false),
    OP(0x4D, "EOR", MODE_ABS, op_eor, 4, false),
    OP(0x4E, "LSR", MODE_ABS, op_lsr, 6, false),
    OP(0x4F, "SRE", MODE_ABS, op_sre, 6, false),

    /* 0x50 - 0x5F */
    OP(0x50, "BVC", MODE_REL, op_bvc, 2, false),
    OP(0x51, "EOR", MODE_IZY, op_eor, 5, true),
    OP(0x52, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x53, "SRE", MODE_IZY, op_sre, 8, false),
    OP(0x54, "NOP", MODE_ZPX, op_nop, 4, false),
    OP(0x55, "EOR", MODE_ZPX, op_eor, 4, false),
    OP(0x56, "LSR", MODE_ZPX, op_lsr, 6, false),
    OP(0x57, "SRE", MODE_ZPX, op_sre, 6, false),
    OP(0x58, "CLI", MODE_IMP, op_cli, 2, false),
    OP(0x59, "EOR", MODE_ABY, op_eor, 4, true),
    OP(0x5A, "NOP", MODE_IMP, op_nop, 2, false),
    OP(0x5B, "SRE", MODE_ABY, op_sre, 7, false),
    OP(0x5C, "NOP", MODE_ABX, op_nop, 4, true),
    OP(0x5D, "EOR", MODE_ABX, op_eor, 4, true),
    OP(0x5E, "LSR", MODE_ABX, op_lsr, 7, false),
    OP(0x5F, "SRE", MODE_ABX, op_sre, 7, false),

    /* 0x60 - 0x6F */
    OP(0x60, "RTS", MODE_IMP, op_rts, 6, false),
    OP(0x61, "ADC", MODE_IZX, op_adc, 6, false),
    OP(0x62, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x63, "RRA", MODE_IZX, op_rra, 8, false),
    OP(0x64, "NOP", MODE_ZP, op_nop, 3, false),
    OP(0x65, "ADC", MODE_ZP, op_adc, 3, false),
    OP(0x66, "ROR", MODE_ZP, op_ror, 5, false),
    OP(0x67, "RRA", MODE_ZP, op_rra, 5, false),
    OP(0x68, "PLA", MODE_IMP, op_pla, 4, false),
    OP(0x69, "ADC", MODE_IMM, op_adc, 2, false),
    OP(0x6A, "ROR", MODE_ACC, op_ror_acc, 2, false),
    OP(0x6B, "ARR", MODE_IMM, op_arr, 2, false),
    OP(0x6C, "JMP", MODE_IND, op_jmp, 5, false),
    OP(0x6D, "ADC", MODE_ABS, op_adc, 4, false),
    OP(0x6E, "ROR", MODE_ABS, op_ror, 6, false),
    OP(0x6F, "RRA", MODE_ABS, op_rra, 6, false),

    /* 0x70 - 0x7F */
    OP(0x70, "BVS", MODE_REL, op_bvs, 2, false),
    OP(0x71, "ADC", MODE_IZY, op_adc, 5, true),
    OP(0x72, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x73, "RRA", MODE_IZY, op_rra, 8, false),
    OP(0x74, "NOP", MODE_ZPX, op_nop, 4, false),
    OP(0x75, "ADC", MODE_ZPX, op_adc, 4, false),
    OP(0x76, "ROR", MODE_ZPX, op_ror, 6, false),
    OP(0x77, "RRA", MODE_ZPX, op_rra, 6, false),
    OP(0x78, "SEI", MODE_IMP, op_sei, 2, false),
    OP(0x79, "ADC", MODE_ABY, op_adc, 4, true),
    OP(0x7A, "NOP", MODE_IMP, op_nop, 2, false),
    OP(0x7B, "RRA", MODE_ABY, op_rra, 7, false),
    OP(0x7C, "NOP", MODE_ABX, op_nop, 4, true),
    OP(0x7D, "ADC", MODE_ABX, op_adc, 4, true),
    OP(0x7E, "ROR", MODE_ABX, op_ror, 7, false),
    OP(0x7F, "RRA", MODE_ABX, op_rra, 7, false),

    /* 0x80 - 0x8F */
    OP(0x80, "NOP", MODE_IMM, op_nop, 2, false),
    OP(0x81, "STA", MODE_IZX, op_sta, 6, false),
    OP(0x82, "NOP", MODE_IMM, op_nop, 2, false),
    OP(0x83, "SAX", MODE_IZX, op_sax, 6, false),
    OP(0x84, "STY", MODE_ZP, op_sty, 3, false),
    OP(0x85, "STA", MODE_ZP, op_sta, 3, false),
    OP(0x86, "STX", MODE_ZP, op_stx, 3, false),
    OP(0x87, "SAX", MODE_ZP, op_sax, 3, false),
    OP(0x88, "DEY", MODE_IMP, op_dey, 2, false),
    OP(0x89, "NOP", MODE_IMM, op_nop, 2, false),
    OP(0x8A, "TXA", MODE_IMP, op_txa, 2, false),
    OP(0x8B, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x8C, "STY", MODE_ABS, op_sty, 4, false),
    OP(0x8D, "STA", MODE_ABS, op_sta, 4, false),
    OP(0x8E, "STX", MODE_ABS, op_stx, 4, false),
    OP(0x8F, "SAX", MODE_ABS, op_sax, 4, false),

    /* 0x90 - 0x9F */
    OP(0x90, "BCC", MODE_REL, op_bcc, 2, false),
    OP(0x91, "STA", MODE_IZY, op_sta, 6, false),
    OP(0x92, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x93, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x94, "STY", MODE_ZPX, op_sty, 4, false),
    OP(0x95, "STA", MODE_ZPX, op_sta, 4, false),
    OP(0x96, "STX", MODE_ZPY, op_stx, 4, false),
    OP(0x97, "SAX", MODE_ZPY, op_sax, 4, false),
    OP(0x98, "TYA", MODE_IMP, op_tya, 2, false),
    OP(0x99, "STA", MODE_ABY, op_sta, 5, false),
    OP(0x9A, "TXS", MODE_IMP, op_txs, 2, false),
    OP(0x9B, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x9C, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x9D, "STA", MODE_ABX, op_sta, 5, false),
    OP(0x9E, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0x9F, "KIL", MODE_IMP, op_kil, 2, false),

    /* 0xA0 - 0xAF */
    OP(0xA0, "LDY", MODE_IMM, op_ldy, 2, false),
    OP(0xA1, "LDA", MODE_IZX, op_lda, 6, false),
    OP(0xA2, "LDX", MODE_IMM, op_ldx, 2, false),
    OP(0xA3, "LAX", MODE_IZX, op_lax, 6, false),
    OP(0xA4, "LDY", MODE_ZP, op_ldy, 3, false),
    OP(0xA5, "LDA", MODE_ZP, op_lda, 3, false),
    OP(0xA6, "LDX", MODE_ZP, op_ldx, 3, false),
    OP(0xA7, "LAX", MODE_ZP, op_lax, 3, false),
    OP(0xA8, "TAY", MODE_IMP, op_tay, 2, false),
    OP(0xA9, "LDA", MODE_IMM, op_lda, 2, false),
    OP(0xAA, "TAX", MODE_IMP, op_tax, 2, false),
    OP(0xAB, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0xAC, "LDY", MODE_ABS, op_ldy, 4, false),
    OP(0xAD, "LDA", MODE_ABS, op_lda, 4, false),
    OP(0xAE, "LDX", MODE_ABS, op_ldx, 4, false),
    OP(0xAF, "LAX", MODE_ABS, op_lax, 4, false),

    /* 0xB0 - 0xBF */
    OP(0xB0, "BCS", MODE_REL, op_bcs, 2, false),
    OP(0xB1, "LDA", MODE_IZY, op_lda, 5, true),
    OP(0xB2, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0xB3, "LAX", MODE_IZY, op_lax, 5, true),
    OP(0xB4, "LDY", MODE_ZPX, op_ldy, 4, false),
    OP(0xB5, "LDA", MODE_ZPX, op_lda, 4, false),
    OP(0xB6, "LDX", MODE_ZPY, op_ldx, 4, false),
    OP(0xB7, "LAX", MODE_ZPY, op_lax, 4, false),
    OP(0xB8, "CLV", MODE_IMP, op_clv, 2, false),
    OP(0xB9, "LDA", MODE_ABY, op_lda, 4, true),
    OP(0xBA, "TSX", MODE_IMP, op_tsx, 2, false),
    OP(0xBB, "LAS", MODE_ABY, op_las, 4, true),
    OP(0xBC, "LDY", MODE_ABX, op_ldy, 4, true),
    OP(0xBD, "LDA", MODE_ABX, op_lda, 4, true),
    OP(0xBE, "LDX", MODE_ABY, op_ldx, 4, true),
    OP(0xBF, "LAX", MODE_ABY, op_lax, 4, true),

    /* 0xC0 - 0xCF */
    OP(0xC0, "CPY", MODE_IMM, op_cpy, 2, false),
    OP(0xC1, "CMP", MODE_IZX, op_cmp, 6, false),
    OP(0xC2, "NOP", MODE_IMM, op_nop, 2, false),
    OP(0xC3, "DCP", MODE_IZX, op_dcp, 8, false),
    OP(0xC4, "CPY", MODE_ZP, op_cpy, 3, false),
    OP(0xC5, "CMP", MODE_ZP, op_cmp, 3, false),
    OP(0xC6, "DEC", MODE_ZP, op_dec, 5, false),
    OP(0xC7, "DCP", MODE_ZP, op_dcp, 5, false),
    OP(0xC8, "INY", MODE_IMP, op_iny, 2, false),
    OP(0xC9, "CMP", MODE_IMM, op_cmp, 2, false),
    OP(0xCA, "DEX", MODE_IMP, op_dex, 2, false),
    OP(0xCB, "SBX", MODE_IMM, op_sbx, 2, false),
    OP(0xCC, "CPY", MODE_ABS, op_cpy, 4, false),
    OP(0xCD, "CMP", MODE_ABS, op_cmp, 4, false),
    OP(0xCE, "DEC", MODE_ABS, op_dec, 6, false),
    OP(0xCF, "DCP", MODE_ABS, op_dcp, 6, false),

    /* 0xD0 - 0xDF */
    OP(0xD0, "BNE", MODE_REL, op_bne, 2, false),
    OP(0xD1, "CMP", MODE_IZY, op_cmp, 5, true),
    OP(0xD2, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0xD3, "DCP", MODE_IZY, op_dcp, 8, false),
    OP(0xD4, "NOP", MODE_ZPX, op_nop, 4, false),
    OP(0xD5, "CMP", MODE_ZPX, op_cmp, 4, false),
    OP(0xD6, "DEC", MODE_ZPX, op_dec, 6, false),
    OP(0xD7, "DCP", MODE_ZPX, op_dcp, 6, false),
    OP(0xD8, "CLD", MODE_IMP, op_cld, 2, false),
    OP(0xD9, "CMP", MODE_ABY, op_cmp, 4, true),
    OP(0xDA, "NOP", MODE_IMP, op_nop, 2, false),
    OP(0xDB, "DCP", MODE_ABY, op_dcp, 7, false),
    OP(0xDC, "NOP", MODE_ABX, op_nop, 4, true),
    OP(0xDD, "CMP", MODE_ABX, op_cmp, 4, true),
    OP(0xDE, "DEC", MODE_ABX, op_dec, 7, false),
    OP(0xDF, "DCP", MODE_ABX, op_dcp, 7, false),

    /* 0xE0 - 0xEF */
    OP(0xE0, "CPX", MODE_IMM, op_cpx, 2, false),
    OP(0xE1, "SBC", MODE_IZX, op_sbc, 6, false),
    OP(0xE2, "NOP", MODE_IMM, op_nop, 2, false),
    OP(0xE3, "ISC", MODE_IZX, op_isc, 8, false),
    OP(0xE4, "CPX", MODE_ZP, op_cpx, 3, false),
    OP(0xE5, "SBC", MODE_ZP, op_sbc, 3, false),
    OP(0xE6, "INC", MODE_ZP, op_inc, 5, false),
    OP(0xE7, "ISC", MODE_ZP, op_isc, 5, false),
    OP(0xE8, "INX", MODE_IMP, op_inx, 2, false),
    OP(0xE9, "SBC", MODE_IMM, op_sbc, 2, false),
    OP(0xEA, "NOP", MODE_IMP, op_nop, 2, false),
    OP(0xEB, "SBC", MODE_IMM, op_sbc, 2, false), /* SBC tidak resmi, banyak dipakai. */
    OP(0xEC, "CPX", MODE_ABS, op_cpx, 4, false),
    OP(0xED, "SBC", MODE_ABS, op_sbc, 4, false),
    OP(0xEE, "INC", MODE_ABS, op_inc, 6, false),
    OP(0xEF, "ISC", MODE_ABS, op_isc, 6, false),

    /* 0xF0 - 0xFF */
    OP(0xF0, "BEQ", MODE_REL, op_beq, 2, false),
    OP(0xF1, "SBC", MODE_IZY, op_sbc, 5, true),
    OP(0xF2, "KIL", MODE_IMP, op_kil, 2, false),
    OP(0xF3, "ISC", MODE_IZY, op_isc, 8, false),
    OP(0xF4, "NOP", MODE_ZPX, op_nop, 4, false),
    OP(0xF5, "SBC", MODE_ZPX, op_sbc, 4, false),
    OP(0xF6, "INC", MODE_ZPX, op_inc, 6, false),
    OP(0xF7, "ISC", MODE_ZPX, op_isc, 6, false),
    OP(0xF8, "SED", MODE_IMP, op_sed, 2, false),
    OP(0xF9, "SBC", MODE_ABY, op_sbc, 4, true),
    OP(0xFA, "NOP", MODE_IMP, op_nop, 2, false),
    OP(0xFB, "ISC", MODE_ABY, op_isc, 7, false),
    OP(0xFC, "NOP", MODE_ABX, op_nop, 4, true),
    OP(0xFD, "SBC", MODE_ABX, op_sbc, 4, true),
    OP(0xFE, "INC", MODE_ABX, op_inc, 7, false),
    OP(0xFF, "ISC", MODE_ABX, op_isc, 7, false)
};

#undef OP

const OpcodeInfo *cpu_opcode_info(uint8_t opcode)
{
    return &g_opcodes[opcode];
}
