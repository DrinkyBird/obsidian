/*
    Obsidian, a lightweight Classicube server
    Copyright (C) 2018 Sean Baggaley.

    Obsidian is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Obsidian is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with Obsidian.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "map.h"
#include "config.h"
#include "listener.h"

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
    map->rng = rng_create((int)time(NULL));
    map->modified_since_last_save = true;

    for (int i = 0; i < 16; i++) {
        map->uuid[i] = (byte)rng_next2(map->rng, 0, 256);
    }

    map->uuid[6] &= 0x0F;
    map->uuid[6] |= 0x40;
    map->uuid[8] &= 0x3F;
    map->uuid[8] |= 0x80;

    return map;
}

void map_destroy(map_t *map){
    free(map->blocks);
    free(map->uuid);

    rng_destroy(map->rng);

    free(map);
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
        fprintf(stderr, "NOTE: attempt to set invalid block [%d, %d, %d] to %d\n", x, y, z, block);
        return false;
    }

    int i = map_get_block_index(map, x, y, z);

    map->last_modify = (unsigned int)time(NULL);
    map->modified_since_last_save = true;
    map->blocks[i] = block;

    broadcast_block_change(x, y, z, block);

    return true;
}

void map_generate(map_t *map) {
         if (strcasecmp(configuration->map_generator, "flat") == 0) mapgen_flat_generate(map); 
    else if (strcasecmp(configuration->map_generator, "classic") == 0) mapgen_classic_generate(map); 
    else if (strcasecmp(configuration->map_generator, "debug") == 0) mapgen_debug_generate(map); 
}