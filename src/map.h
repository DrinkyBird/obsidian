#ifndef __MAP_H__
#define __MAP_H__

#include <stdbool.h>
#include "defs.h"
#include "mapgen.h"
#include "rng.h"

#define MAPFILE_MAGIC "HARM"
#define MAPFILE_VERSION 0

typedef struct map_s {
    const char *name;
    int width, depth, height;
    byte *blocks;
    rng_t *rng;

    byte *uuid;
    unsigned int time_created, last_modify, last_access;
} map_t;

typedef struct mapsave_s {
    char _magic[4];
    byte _file_version;

    const char *name;
    int width, depth, height;
} mapsave_t;

map_t *map_create(const char *name, int width, int depth, int height);
void map_destroy(map_t *map);

block_e map_get(map_t *map, int x, int y, int z);
bool map_set(map_t *map, int x, int y, int z, block_e block);
bool map_pos_valid(map_t *, int, int, int);

int map_get_block_index(map_t *, int, int, int);

map_t *map_load(const char *name);
void map_save(map_t *map);

void map_generate(map_t *map);

#endif