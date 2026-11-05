/*
 * file.c - Implementasi pembacaan file biner.
 */
#include "util/file.h"

#include "util/log.h"

#include <stdio.h>
#include <stdlib.h>

ErrorCode file_read_all(const char *path, FileData *out)
{
    FILE *fp = NULL;
    long length = 0;
    uint8_t *buffer = NULL;
    size_t bytes_read = 0;

    if (path == NULL || out == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    out->data = NULL;
    out->size = 0;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        log_error("Tidak dapat membuka file '%s'.", path);
        return ERR_IO;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return ERR_IO;
    }

    length = ftell(fp);
    if (length < 0) {
        fclose(fp);
        return ERR_IO;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return ERR_IO;
    }

    /* File kosong tetap valid secara I/O; validasi isi dilakukan pemanggil. */
    buffer = (uint8_t *)malloc((size_t)length + 1U);
    if (buffer == NULL) {
        fclose(fp);
        return ERR_ALLOCATION;
    }

    bytes_read = fread(buffer, 1, (size_t)length, fp);
    if (bytes_read != (size_t)length) {
        free(buffer);
        fclose(fp);
        return ERR_IO;
    }

    fclose(fp);
    out->data = buffer;
    out->size = bytes_read;
    return ERR_OK;
}

void file_data_free(FileData *file)
{
    if (file == NULL) {
        return;
    }

    free(file->data);
    file->data = NULL;
    file->size = 0;
}
