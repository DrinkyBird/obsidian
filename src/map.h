#ifndef __MAP_H__
#define __MAP_H__

#include <stdbool.h>
#include "defs.h"

typedef struct map_s {
    int width, depth, height;
    byte *blocks;
} map_t;

map_t *map_create(int width, int depth, int height);

block_e map_get(map_t *map, int x, int y, int z);
bool map_set(map_t *map, int x, int y, int z, block_e block);

void map_generate(map_t *map);

#endif