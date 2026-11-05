/*
 * test.h - Framework unit test minimal untuk TinyNES.
 *
 * Tidak memakai library eksternal. Setiap pemeriksaan mencetak lokasi dan
 * nilai bila gagal, lalu test_main.c merangkum hasilnya.
 */
#ifndef TINYNES_TESTS_TEST_H
#define TINYNES_TESTS_TEST_H

#include <stdio.h>

extern int g_test_checks;
extern int g_test_failures;
extern int g_test_suites;

#define CHECK_TRUE(condition)                                                          \
    do {                                                                               \
        ++g_test_checks;                                                               \
        if (!(condition)) {                                                            \
            ++g_test_failures;                                                         \
            fprintf(stderr, "  GAGAL %s:%d: %s\n", __FILE__, __LINE__, #condition);    \
        }                                                                              \
    } while (0)

#define CHECK_FALSE(condition) CHECK_TRUE(!(condition))

#define CHECK_EQ_U8(actual, expected)                                                  \
    do {                                                                               \
        unsigned int check_a = (unsigned int)(actual);                                 \
        unsigned int check_e = (unsigned int)(expected);                               \
        ++g_test_checks;                                                               \
        if (check_a != check_e) {                                                      \
            ++g_test_failures;                                                         \
            fprintf(stderr, "  GAGAL %s:%d: %s = %u, seharusnya %u\n", __FILE__,        \
                    __LINE__, #actual, check_a, check_e);                              \
        }                                                                              \
    } while (0)

#define CHECK_EQ_U16(actual, expected)                                                 \
    do {                                                                               \
        unsigned int check_a = (unsigned int)(actual);                                 \
        unsigned int check_e = (unsigned int)(expected);                               \
        ++g_test_checks;                                                               \
        if (check_a != check_e) {                                                      \
            ++g_test_failures;                                                         \
            fprintf(stderr, "  GAGAL %s:%d: %s = 0x%04X, seharusnya 0x%04X\n",         \
                    __FILE__, __LINE__, #actual, check_a, check_e);                    \
        }                                                                              \
    } while (0)

#define CHECK_EQ_INT(actual, expected) CHECK_EQ_U16((unsigned int)(actual), (unsigned int)(expected))

#define RUN_SUITE(function)                                                            \
    do {                                                                               \
        int before = g_test_failures;                                                  \
        ++g_test_suites;                                                               \
        printf("[TEST] %s\n", #function);                                              \
        function();                                                                    \
        if (g_test_failures == before) {                                               \
            printf("  OK\n");                                                          \
        }                                                                              \
    } while (0)

#endif /* TINYNES_TESTS_TEST_H */
