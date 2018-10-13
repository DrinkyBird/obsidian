#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
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
    map->time_created = (unsigned int)time(NULL);
    map->last_modify = 0;
    map->last_access = 0;
    map->uuid = calloc(16, sizeof(byte));

    for (int i = 0; i < 16; i++) {
        map->uuid[i] = (byte)rrand(0, 0xFF);
    }

    map->uuid[6] &= 0x0F;
    map->uuid[6] |= 0x40;
    map->uuid[8] &= 0x3F;
    map->uuid[8] |= 0x80;

    return map;
}

block_e map_get(map_t *map, int x, int y, int z) {
    if (!map_pos_valid(map, x, y, z)) {
        fprintf(stderr, "NOTE: attempt to get invalid block [%d, %d, %d]\n", x, y, z);
        return air;
    }

    map->last_access = (unsigned int)time(NULL);
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

    map->last_modify = (unsigned int)time(NULL);
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