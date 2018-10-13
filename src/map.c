#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "map.h"
#include "rw.h"
#include "nbt.h"

int map_get_block_index(map_t *, int, int, int);
bool map_pos_valid(map_t *, int, int, int);

map_t *map_create(const char *name, int width, int depth, int height) {
    map_t *map = malloc(sizeof(*map));

    map->name = name;
    map->width = width;
    map->depth = depth;
    map->height = height;
    map->blocks = calloc((width * depth * height), sizeof(*map->blocks));

    return map;
}

block_e map_get(map_t *map, int x, int y, int z) {
    if (!map_pos_valid(map, x, y, z)) {
        fprintf(stderr, "NOTE: attempt to get invalid block [%d, %d, %d]\n", x, y, z);
        return air;
    }

    return map->blocks[map_get_block_index(map, x, y, z)];
}

int map_get_block_index(map_t *map, int x, int y, int z) {
    return (y * map->height + z) * map->width + x;
}

bool map_pos_valid(map_t *m, int x, int y, int z) {
    return !(x < 0 || y < 0 || z < 0 || x >= m->width || y >= m->depth || z >= m->height);
}

bool map_set(map_t *map, int x, int y, int z, block_e block) {
    if (!map_pos_valid(map, x, y, z)) {
        fprintf(stderr, "NOTE: attempt to set invalid block [%d, %d, %d]\n", x, y, z);
        return false;
    }

    int i = map_get_block_index(map, x, y, z);

    map->blocks[i] = block;
    return true;
}

void map_generate(map_t *map) {
    int land = map->depth / 2;

    for (int x = 0; x < map->width; x++)
    for (int z = 0; z < map->height; z++)
    for (int y = 0; y < land; y++) 
    {
        block_e b = air;

        if (y == land - 1) {
            b = grass;
        } else if (y >= land - 5) {
            b = dirt;
        } else {
            b = stone;
        }

        map_set(map, x, y, z, b);
    }
}