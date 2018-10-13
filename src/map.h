#ifndef __MAP_H__
#define __MAP_H__

#include <stdbool.h>
#include "defs.h"

#define MAPFILE_MAGIC "HARM"
#define MAPFILE_VERSION 0

typedef struct map_s {
    const char *name;
    int width, depth, height;
    byte *blocks;

    unsigned int time_created, last_modify, last_access;
} map_t;

typedef struct mapsave_s {
    char _magic[4];
    byte _file_version;

    const char *name;
    int width, depth, height;
} mapsave_t;

map_t *map_create(const char *name, int width, int depth, int height);

block_e map_get(map_t *map, int x, int y, int z);
bool map_set(map_t *map, int x, int y, int z, block_e block);

map_t *map_load(const char *name);
void map_save(map_t *map);

void map_generate(map_t *map);

#endif