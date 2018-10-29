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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "map.h"
#include "mapgen.h"
#include "perlin.h"

/* used for flood fill */
typedef struct fastintstack_s {
    int *values;
    int capacity, size;
} fastintstack_t;

static fastintstack_t *fastintstack_create(int capacity);
static int fastintstack_pop(fastintstack_t *);
static void fastintstack_push(fastintstack_t *, int);
static void fastintstack_destroy(fastintstack_t *);

static int *heightMap = NULL;

static void gen_heightmap(map_t *map);
static void gen_strata(map_t *map);
static void gen_caves(map_t *map);
static void gen_ore(map_t *map, block_e block, float abundance);
static void gen_water(map_t *map);
static void gen_lava(map_t *map);
static void gen_surface(map_t *map);
static void gen_plants(map_t *map);

static void fill_oblate_spherioid(map_t *map, int centreX, int centreY, int centreZ, double radius, block_e block);
static void flood_fill(map_t *map, int startIndex, block_e block);

#define DO_PERCENT(_done, _total, _str) \
    _done++; \
    int _percent = (int)(((float)_done / (float)_total) * 100.0f); \
    if (_percent != lastPercent) { \
        lastPercent = _percent; \
        printf("%s... % 3d%%\n", _str, _percent); \
    }

void mapgen_classic_generate(map_t *map) {
    gen_heightmap(map);
    gen_strata(map);
    gen_caves(map);

    gen_ore(map, gold_ore, 0.5f);
    gen_ore(map, iron_ore, 0.7f);
    gen_ore(map, coal_ore, 0.9f);

    gen_water(map);
    gen_lava(map);
    gen_surface(map);
    gen_plants(map);

    free(heightMap);
}

void gen_heightmap(map_t *map) {
    int blocks = (map->width * map->height);
    int done = 0, lastPercent = 0;

    combinednoise_t *noise1 = combinednoise_create(octavenoise_create(map->rng, 8), octavenoise_create(map->rng, 8));
    combinednoise_t *noise2 = combinednoise_create(octavenoise_create(map->rng, 8), octavenoise_create(map->rng, 8));
    octavenoise_t *noise3 = octavenoise_create(map->rng, 6);

    heightMap = calloc(map->width * map->depth * map->height, sizeof(int));

    for (int x = 0; x < map->width; x++) 
    for (int z = 0; z < map->height; z++) {
        double heightLow = combinednoise_compute(noise1, x * 1.3, z * 1.3) / 6 - 4;
        double heightHigh = combinednoise_compute(noise2, x * 1.3, z * 1.3) / 5 + 6;
        double heightResult;

        if (octavenoise_compute(noise3, x, z) / 8 > 0) {
            heightResult = heightLow;
        } else {
            heightResult = MAX(heightLow, heightHigh);
        }

        heightResult /= 2;

        if (heightResult < 0) {
            heightResult *= 0.8;
        }

        heightMap[x + z * map->width] = (int) (heightResult + (map->depth / 2));

        DO_PERCENT(done, blocks, "Raising");
    }

    combinednoise_destroy(noise1);
    combinednoise_destroy(noise2);
    octavenoise_destroy(noise3);
}

void gen_strata(map_t *map) {
    int blocks = (map->width * map->depth * map->height);
    int done = 0, lastPercent = 0;

    octavenoise_t *noise = octavenoise_create(map->rng, 8);

    for (int x = 0; x < map->width; x++) 
    for (int z = 0; z < map->height; z++) {
        int dirtThickness = (int) octavenoise_compute(noise, x, z) / 24 - 4;
        int dirtTransition = heightMap[x + z * map->width];
        int stoneTransition = dirtTransition + dirtThickness;

        for (int y = 0; y < map->depth; y++) {
            block_e block = air;

            if (y == 0) {
                block = lava;
            } else if (y <= stoneTransition) {
                block = stone;
            } else if (y <= dirtTransition) {
                block = dirt;
            }

            map_set(map, x, y, z, block);

            DO_PERCENT(done, blocks, "Soiling");
        }
    }
}

void gen_caves(map_t *map) {
    int numCaves = (map->width * map->depth * map->height) / 8192;
    int lastPercent = 0;

    for (int i = 0; i < numCaves; i++) {
        double caveX = rng_next2(map->rng, 0, map->width);
        double caveY = rng_next2(map->rng, 0, map->width);
        double caveZ = rng_next2(map->rng, 0, map->width);

        int caveLength = (int) (rng_next_float(map->rng) * rng_next_float(map->rng) * 200.0f);

        float theta = rng_next_float(map->rng) * M_PI * 2;
        float deltaTheta = 0.0f;

        float phi = rng_next_float(map->rng) * M_PI * 2;
        float deltaPhi = 0.0f;

        float caveRadius = rng_next_float(map->rng) * rng_next_float(map->rng);

        for (float len = 0; len < caveLength; len++) {
            caveX += sin(theta) * cos(phi);
            caveY += cos(theta) * cos(phi);
            caveZ += sin(phi);

            theta = theta + deltaTheta * 0.2f;
            deltaTheta = deltaTheta * 0.9f + rng_next_float(map->rng) - rng_next_float(map->rng);
            phi = phi / 2.0f + deltaPhi / 4.0f;
            deltaPhi = deltaPhi * 0.75f + rng_next_float(map->rng) - rng_next_float(map->rng);

            if (rng_next_float(map->rng) >= 0.25f) {
                int centreX = (int) (caveX + (rng_next(map->rng, 4) - 2) * 0.2f);
                int centreY = (int) (caveY + (rng_next(map->rng, 4) - 2) * 0.2f);
                int centreZ = (int) (caveZ + (rng_next(map->rng, 4) - 2) * 0.2f);

                float radius = (map->height - centreY) / (float)map->height;
                radius = 1.2 + (radius * 3.5 + 1) * caveRadius;
                radius = radius * sin(len * M_PI / caveLength);

                fill_oblate_spherioid(map, centreX, centreY, centreZ, radius, air);
            }
        } 

        DO_PERCENT(i, numCaves, "Carving");
    } 
}

void gen_ore(map_t *map, block_e block, float abundance) {
    int numVeins = (int)(((float)(map->width * map->depth * map->height) * abundance) / 16384.0f);

    for (int i = 0; i < numVeins; i++) {
        float veinX = rng_next2(map->rng, 0, map->width);
        float veinY = rng_next2(map->rng, 0, map->depth);
        float veinZ = rng_next2(map->rng, 0, map->height);

        float veinLength = rng_next_float(map->rng) * rng_next_float(map->rng) * 75.0f * abundance;

        float theta = rng_next_float(map->rng) * M_PI * 2;
        float deltaTheta = 0;
        float phi = rng_next_float(map->rng) * M_PI * 2;
        float deltaPhi = 0;

        for (float len = 0; len < veinLength; len++) {
            veinX = veinX + sin(theta) * cos(phi);
            veinY = veinY + cos(theta) * cos(phi);
            veinZ = veinZ + cos(theta);

            theta = deltaTheta * 0.2f;
            deltaTheta = (deltaTheta * 0.9f) + rng_next_float(map->rng) - rng_next_float(map->rng);
            phi = phi / 2.0f + deltaPhi / 4.0f;
            deltaPhi = (deltaPhi * 0.9f) + rng_next_float(map->rng) - rng_next_float(map->rng);

            float radius = abundance * sin(len * M_PI / veinLength) + 1;

            fill_oblate_spherioid(map, (int)veinX, (int)veinY, (int)veinZ, radius, block);
        }
    }
}

void gen_water(map_t *map) {
    int waterLevel = (map->depth / 2) - 1;
    int numSources = (map->width * map->height) / 800;

    int total = map->width + map->height + numSources;
    int done = 0, lastPercent = 0;
    
    for (int x = 0; x < map->width; x++) {
        flood_fill(map, map_get_block_index(map, x, waterLevel, 0), water);
        flood_fill(map, map_get_block_index(map, x, waterLevel, map->height - 1), water);

        DO_PERCENT(done, total, "Watering");
    }

    for (int z = 0; z < map->height; z++) {
        flood_fill(map, map_get_block_index(map, 0, waterLevel, z), water);
        flood_fill(map, map_get_block_index(map, map->width - 1, waterLevel, z), water);

        DO_PERCENT(done, total, "Watering");
    }

    for (int i = 0; i < numSources; i++) {
        int x = rng_next2(map->rng, 0, map->width);
        int z = rng_next2(map->rng, 0, map->height);
        int y = waterLevel - rng_next2(map->rng, 0, 2);

        flood_fill(map, map_get_block_index(map, x, y, z), water);

        DO_PERCENT(done, total, "Watering");
    }
}

void gen_lava(map_t *map) {
    int waterLevel = (map->depth / 2) - 1;
    int numSources = (map->width * map->height) / 20000;

    int done = 0, lastPercent = 0;

    for (int i = 0; i < numSources; i++) {
        int x = rng_next2(map->rng, 0, map->width);
        int z = rng_next2(map->rng, 0, map->height);
        int y = (int) ((waterLevel - 3) * rng_next_float(map->rng) * rng_next_float(map->rng));

        flood_fill(map, map_get_block_index(map, x, y, z), lava);

        DO_PERCENT(done, numSources, "Melting");
    }
}

void gen_surface(map_t *map) {
    octavenoise_t *noise1 = octavenoise_create(map->rng, 8);
    octavenoise_t *noise2 = octavenoise_create(map->rng, 8);

    int total = map->width * map->height;
    int done = 0, lastPercent = 0;

    for (int x = 0; x < map->width; x++) 
    for (int z = 0; z < map->height; z++) {
        bool sandChance = octavenoise_compute(noise1, x, z) > 8;
        bool gravelChance = octavenoise_compute(noise2, x, z) > 12;

        int y = heightMap[x + z * map->width];
        block_e above = map_get(map, x, y + 1, z);

        if (above == water && gravelChance) {
            map_set(map, x, y, z, gravel);
        }

        else if (above == air) {
            if (y <= (map->depth / 2) && sandChance) {
                map_set(map, x, y, z, sand);
            } else {
                map_set(map, x, y, z, grass);
            }
        }

        DO_PERCENT(done, total, "Growing");
    }

    octavenoise_destroy(noise1);
    octavenoise_destroy(noise2);
}

void gen_plants(map_t *map) {
    int numFlowers = (map->width * map->height) / 3000;
    int numShrooms = (map->width * map->depth * map->height) / 2000;
    int numTrees = (map->width * map->height) / 4000;

    int total = numFlowers + numShrooms + numTrees;
    int done = 0, lastPercent = 0;

    for (int i = 0; i < numFlowers; i++) {
        block_e flowerType = rng_next_boolean(map->rng) ? dandelion : rose;

        int patchX = rng_next2(map->rng, 0, map->width);
        int patchZ = rng_next2(map->rng, 0, map->height);

        for (int j = 0; j < 10; j++) {
            int flowerX = patchX;
            int flowerZ = patchZ;

            for (int k = 0; k < 5; k++) {
                flowerX += rng_next(map->rng, 6) - rng_next(map->rng, 6);
                flowerZ += rng_next(map->rng, 6) - rng_next(map->rng, 6);

                if (map_pos_valid(map, flowerX, 0, flowerZ)) {
                    int flowerY = heightMap[flowerX + flowerZ * map->width] + 1;
                    block_e below = map_get(map, flowerX, flowerY - 1, flowerZ);
                    
                    if (map_get(map, flowerX, flowerY, flowerZ) == air && below == grass) {
                        map_set(map, flowerX, flowerY, flowerZ, flowerType);
                    } 
                }
            }
        }

        DO_PERCENT(done, total, "Planting");
    }

    for (int i = 0; i < numShrooms; i++) {
        block_e shroomType = rng_next_boolean(map->rng) ? brown_mushroom : red_mushroom;

        int patchX = rng_next2(map->rng, 0, map->width);
        int patchY = rng_next2(map->rng, 1, map->depth);
        int patchZ = rng_next2(map->rng, 0, map->height);

        for (int j = 0; j < 20; j++) {
            int shroomX = patchX;
            int shroomY = patchY;
            int shroomZ = patchZ;

            for (int k = 0; k < 5; k++) {
                shroomX += rng_next(map->rng, 6) - rng_next(map->rng, 6);
                shroomZ += rng_next(map->rng, 6) - rng_next(map->rng, 6);

                if (map_pos_valid(map, shroomX, 0, shroomZ) && shroomY < heightMap[shroomX + shroomZ * map->width] - 1) {
                    block_e below = map_get(map, shroomX, shroomY - 1, shroomZ);
                    
                    if (map_get(map, shroomX, shroomY, shroomZ) == air && below == stone) {
                        map_set(map, shroomX, shroomY, shroomZ, shroomType);
                    } 
                }
            }
        }

        DO_PERCENT(done, total, "Planting");
    }

    for (int i = 0; i < numTrees; i++) {
        int patchX = rng_next2(map->rng, 0, map->width);
        int patchZ = rng_next2(map->rng, 0, map->height);

        for (int j = 0; j < 20; j++) {
            int treeX = patchX;
            int treeZ = patchZ;

            for (int k = 0; k < 20; k++) {
                treeX += rng_next(map->rng, 6) - rng_next(map->rng, 6);
                treeZ += rng_next(map->rng, 6) - rng_next(map->rng, 6);

                if (map_pos_valid(map, treeX, 0, treeZ) && rng_next_float(map->rng) <= 0.25f) {
                    int treeY = heightMap[treeX + treeZ * map->width] + 1;
                    int treeHeight = rng_next2(map->rng, 1, 3) + 4;

                    if (mapgen_space_for_tree(map, treeX, treeY, treeZ, treeHeight)) {
                        mapgen_grow_tree(map, treeX, treeY, treeZ, treeHeight);
                    }
                }
            }
        }

        DO_PERCENT(done, total, "Planting");
    }
}

void fill_oblate_spherioid(map_t *map, int centreX, int centreY, int centreZ, double radius, block_e block) {
    int xStart = (int)floor(MAX(centreX - radius, 0));
    int xEnd = (int)floor(MIN(centreX + radius, map->width - 1));
    int yStart = (int)floor(MAX(centreY - radius, 0));
    int yEnd = (int)floor(MIN(centreY + radius, map->depth - 1));
    int zStart = (int)floor(MAX(centreZ - radius, 0));
    int zEnd = (int)floor(MIN(centreZ + radius, map->height - 1));

    for (double x = xStart; x < xEnd; x++)
    for (double y = yStart; y < yEnd; y++)
    for (double z = zStart; z < zEnd; z++) {
        double dx = x - centreX;
        double dy = y - centreY;
        double dz = z - centreZ;

        int ix = (int)x;
        int iy = (int)y;
        int iz = (int)z;

        if ((dx * dx + 2 * dy * dy + dz * dz) < (radius * radius) && map_pos_valid(map, ix, iy, iz)) {
            if (map_get(map, ix, iy, iz) == stone) {
                map_set(map, ix, iy, iz, block);
            }
        }
    }
}

void flood_fill(map_t *map, int startIndex, block_e block) {
    int oneY = map->width * map->height;
    int blocklen = map->width * map->depth * map->height;

    if (startIndex < 0) return;

    fastintstack_t *stack = fastintstack_create(4);
    fastintstack_push(stack, startIndex);

    while (stack->size > 0) {
        int index = fastintstack_pop(stack);

        if (index < 0 || index >= blocklen) continue;

        if (map->blocks[index] != air) continue;
        map->blocks[index] = block;

        int x = index % map->width;
        int y = index / oneY;
        int z = (index / map->width) % map->height;

        if (x > 0) fastintstack_push(stack, index - 1);
        if (x < map->width - 1) fastintstack_push(stack, index + 1);
        if (z > 0) fastintstack_push(stack, index - map->width);
        if (z < map->height - 1) fastintstack_push(stack, index + map->width);
        if (y > 0) fastintstack_push(stack, index - oneY);
    }

    fastintstack_destroy(stack);
}

fastintstack_t *fastintstack_create(int capacity) {
    fastintstack_t *s = malloc(sizeof(fastintstack_t));
    s->size = 0;
    s->capacity = capacity;
    s->values = calloc(capacity, sizeof(int));
    return s;
}

int fastintstack_pop(fastintstack_t *s) {
    return s->values[--s->size];
}

void fastintstack_push(fastintstack_t *s, int v) {
    if (s->size == s->capacity) {
        int newcap = s->capacity * 2;
        int *arr = calloc(newcap, sizeof(int));
        memcpy(arr, s->values, s->size * sizeof(int));
        free(s->values);
        s->values = arr;
        s->capacity = newcap;
    }

    s->values[s->size++] = v;
}

void fastintstack_destroy(fastintstack_t *s) {
    free(s->values);
    free(s);
}