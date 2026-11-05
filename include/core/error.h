/*
 * error.h - Kode hasil (result code) yang dipakai seluruh TinyNES.
 *
 * C tidak memiliki exception. Agar kegagalan tidak hilang diam-diam, setiap
 * operasi yang dapat gagal mengembalikan ErrorCode. Nilai ERR_OK berarti
 * berhasil; nilai lain menjelaskan penyebab kegagalan.
 */
#ifndef TINYNES_CORE_ERROR_H
#define TINYNES_CORE_ERROR_H

#include "util/types.h"

typedef enum ErrorCode {
    ERR_OK = 0,
    ERR_ALLOCATION,         /* malloc/calloc gagal. */
    ERR_IO,                 /* File tidak dapat dibuka atau dibaca. */
    ERR_INVALID_ARGUMENT,   /* Argumen fungsi tidak valid. */
    ERR_INES_MAGIC,         /* Empat byte pertama bukan "NES<0x1A>". */
    ERR_INES_FORMAT,        /* Header iNES tidak valid. */
    ERR_UNSUPPORTED_MAPPER, /* Mapper belum diimplementasikan. */
    ERR_UNSUPPORTED_CONSOLE, /* Console NES 2.0 bukan NES/Famicom standar. */
    ERR_UNSUPPORTED_REGION,  /* Timing Dendy belum didukung. */
    ERR_INVALID_ROM_SIZE,   /* Ukuran file lebih kecil dari isi header. */
    ERR_CONFIG,             /* Argumen command line salah. */
    ERR_SDL_INIT,           /* SDL_Init gagal. */
    ERR_SDL_WINDOW,         /* Window SDL gagal dibuat. */
    ERR_SDL_RENDERER,       /* Renderer SDL gagal dibuat. */
    ERR_SDL_TEXTURE,        /* Texture streaming gagal dibuat. */
    ERR_SDL_AUDIO,          /* Audio device gagal dibuka. */
    ERR_SDL_CONTROLLER      /* Game controller gagal dibuka. */
} ErrorCode;

/* Mengembalikan pesan Bahasa Indonesia yang dapat dicetak ke pengguna. */
const char *error_string(ErrorCode code);

/* Mencetak "konteks: pesan" ke stderr bila code bukan ERR_OK. */
void error_report(ErrorCode code, const char *context);

#endif /* TINYNES_CORE_ERROR_H */
