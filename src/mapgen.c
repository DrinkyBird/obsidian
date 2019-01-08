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
#include <stdlib.h>
#include "map.h"
#include "mapgen.h"

static inline int qabs(int x) { if (x < 0) return -x; return x; }

bool mapgen_space_for_tree(struct map_s *map, int x, int y, int z, int height) {
    if (!map_pos_valid(map, x, y, z) || !map_pos_valid(map, x, y - 1, z)) {
        return false;
    }

    block_e below = map_get(map, x, y - 1, z);
    if (below != dirt && below != grass) {
        return false;
    }

    block_e here = map_get(map, x, y, z);
    if (here == sapling) {
        map_set(map, x, y, z, air);
    }

    for (int xx = x - 1; xx <= x + 1; xx++)
    for (int yy = y; yy < y + height; yy++)
    for (int zz = z - 1; zz <= z + 1; zz++) {
        if (!map_pos_valid(map, xx, yy, zz)) {
            return false;
        }

        if (map_get(map, xx, yy, zz) != air) {
            return false;
        }
    }

    int canopyY = y + (height - 4);

    for (int xx = x - 2; xx <= x + 2; xx++)
    for (int yy = canopyY; yy < y + height; yy++)
    for (int zz = z - 2; zz <= z + 2; zz++) {
        if (!map_pos_valid(map, xx, yy, zz)) {
            return false;
        }

        if (map_get(map, xx, yy, zz) != air) {
            return false;
        }
    }

    if (here == sapling) {
        map_set(map, x, y, z, sapling);
    }

    return true;
}

void mapgen_grow_tree(struct map_s *map, int x, int y, int z, int height) {
    map_set(map, x, y, z, wood);

    int max0 = y + height;
    int max1 = max0 - 1;
    int max2 = max0 - 2;
    int max3 = max0 - 3;

    /* bottom */
    for (int xx = -2; xx <= 2; xx++)
    for (int zz = -2; zz <= 2; zz++) {
        int ax = x + xx;
        int az = z + zz;

        if (qabs(xx) == 2 && qabs(zz) == 2) {
            if (rng_next_boolean(map->rng)) map_set(map, ax, max3, az, leaves);
            if (rng_next_boolean(map->rng)) map_set(map, ax, max2, az, leaves);
        } else {
            map_set(map, ax, max3, az, leaves);
            map_set(map, ax, max2, az, leaves);
        }
    }

    /* top */
    for (int xx = -1; xx <= 1; xx++)
    for (int zz = -1; zz <= 1; zz++) { 
        int ax = x + xx;
        int az = z + zz;

        if (xx == 0 || zz == 0) {
            map_set(map, ax, max1, az, leaves);
            map_set(map, ax, max0, az, leaves);
        } else {
            if (rng_next_boolean(map->rng)) map_set(map, ax, max1, az, leaves);
        }
    }

    /* trunk */
    for (int yy = y; yy < max0; yy++) {
        map_set(map, x, yy, z, wood);
    }
}