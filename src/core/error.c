/*
 * error.c - Pemetaan kode error ke pesan yang ramah pengguna.
 */
#include "core/error.h"

#include "util/log.h"

const char *error_string(ErrorCode code)
{
    switch (code) {
    case ERR_OK:
        return "Berhasil.";
    case ERR_ALLOCATION:
        return "Alokasi memori gagal.";
    case ERR_IO:
        return "Terjadi kesalahan saat membaca atau menulis file.";
    case ERR_INVALID_ARGUMENT:
        return "Argumen tidak valid.";
    case ERR_INES_MAGIC:
        return "File bukan cartridge iNES yang valid (magic bytes salah).";
    case ERR_INES_FORMAT:
        return "Header iNES/NES 2.0 tidak valid atau tidak lengkap.";
    case ERR_UNSUPPORTED_MAPPER:
        return "Mapper cartridge belum didukung. Versi ini hanya mendukung NROM (mapper 0).";
    case ERR_UNSUPPORTED_CONSOLE:
        return "Console type NES 2.0 belum didukung. Hanya NES/Famicom standar.";
    case ERR_UNSUPPORTED_REGION:
        return "Timing region tidak didukung. TinyNES mendukung NTSC dan PAL, tetapi belum Dendy.";
    case ERR_INVALID_ROM_SIZE:
        return "Ukuran file lebih kecil daripada ukuran yang dideklarasikan header iNES.";
    case ERR_CONFIG:
        return "Konfigurasi command line tidak valid.";
    case ERR_SDL_INIT:
        return "Inisialisasi SDL2 gagal.";
    case ERR_SDL_WINDOW:
        return "Pembuatan window SDL2 gagal.";
    case ERR_SDL_RENDERER:
        return "Pembuatan renderer SDL2 gagal.";
    case ERR_SDL_TEXTURE:
        return "Pembuatan texture streaming SDL2 gagal.";
    case ERR_SDL_AUDIO:
        return "Pembukaan audio device SDL2 gagal.";
    case ERR_SDL_CONTROLLER:
        return "Pembukaan game controller SDL2 gagal.";
    default:
        return "Kesalahan tidak dikenal.";
    }
}

void error_report(ErrorCode code, const char *context)
{
    if (code == ERR_OK) {
        return;
    }

    if (context != NULL) {
        log_error("%s: %s", context, error_string(code));
    } else {
        log_error("%s", error_string(code));
    }
}
