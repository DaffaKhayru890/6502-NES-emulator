/*
 * mapper.c - Implementasi pembungkus Mapper.
 */
#include "cartridge/mapper.h"

#include <stdlib.h>

Mapper *mapper_create(const MapperOps *ops, void *context)
{
    Mapper *mapper = NULL;

    if (ops == NULL) {
        return NULL;
    }

    mapper = (Mapper *)malloc(sizeof(*mapper));
    if (mapper == NULL) {
        return NULL;
    }

    mapper->ops = ops;
    mapper->context = context;
    return mapper;
}

void mapper_destroy(Mapper *mapper)
{
    if (mapper == NULL) {
        return;
    }

    if (mapper->ops != NULL && mapper->ops->destroy != NULL) {
        mapper->ops->destroy(mapper->context);
    }

    free(mapper);
}

uint8_t mapper_cpu_read(Mapper *mapper, uint16_t address)
{
    if (mapper == NULL || mapper->ops == NULL || mapper->ops->cpu_read == NULL) {
        return 0;
    }
    return mapper->ops->cpu_read(mapper->context, address);
}

void mapper_cpu_write(Mapper *mapper, uint16_t address, uint8_t value)
{
    if (mapper == NULL || mapper->ops == NULL || mapper->ops->cpu_write == NULL) {
        return;
    }
    mapper->ops->cpu_write(mapper->context, address, value);
}

uint8_t mapper_ppu_read(Mapper *mapper, uint16_t address)
{
    if (mapper == NULL || mapper->ops == NULL || mapper->ops->ppu_read == NULL) {
        return 0;
    }
    return mapper->ops->ppu_read(mapper->context, address);
}

void mapper_ppu_write(Mapper *mapper, uint16_t address, uint8_t value)
{
    if (mapper == NULL || mapper->ops == NULL || mapper->ops->ppu_write == NULL) {
        return;
    }
    mapper->ops->ppu_write(mapper->context, address, value);
}
