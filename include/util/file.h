/*
 * file.h - Pembacaan file biner dengan alokasi eksplisit.
 *
 * FileData memiliki buffer hasil malloc. Pemanggil WAJIB memanggil
 * file_data_free() setelah selesai. Kepemilikan ini sengaja dibuat eksplisit
 * agar tidak ada kebocoran memori pada jalur error.
 */
#ifndef TINYNES_UTIL_FILE_H
#define TINYNES_UTIL_FILE_H

#include "core/error.h"
#include "util/types.h"

typedef struct FileData {
    uint8_t *data;
    size_t size;
} FileData;

/* Membaca seluruh isi file. Mengembalikan ERR_OK bila berhasil. */
ErrorCode file_read_all(const char *path, FileData *out);

/* Membebaskan buffer dan mengosongkan struktur. Aman dipanggil berkali-kali. */
void file_data_free(FileData *file);

#endif /* TINYNES_UTIL_FILE_H */
