/*
    Obsidian, a lightweight Classicube server
    Copyright (C) 2018-2019 Sean Baggaley.

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
    bool modified_since_last_save;
} map_t;

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