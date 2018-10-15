#include "defs.h"
#include "map.h"
#include "mapgen.h"

void mapgen_flat_generate(map_t *map) {
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

void mapgen_debug_generate(map_t *map) {
    for (int x = 0; x < map->width; x++)
    for (int z = 0; z < map->height; z++) {
        map_set(map, x, 0, z, stone);

        if (x < num_blocks) {
            map_set(map, x, 1, z, (block_e) x);
        }
    }
}