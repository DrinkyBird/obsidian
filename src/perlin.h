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
#ifndef __PERLIN_H__
#define __PERLIN_H__

#include "defs.h"
#include "rng.h"

typedef struct improvednoise_s {
    byte *p;
    int num_p;

    rng_t *rng;
} improvednoise_t;

typedef struct octavenoise_s {
    improvednoise_t **octaves;
    int num_octaves;

    rng_t *rng;
} octavenoise_t;

typedef struct combinednoise_s {
    octavenoise_t *n1, *n2;
} combinednoise_t;

improvednoise_t *improvednoise_create(rng_t *rng);
double improvednoise_compute(improvednoise_t *noise, double x, double y);
void improvednoise_destroy(improvednoise_t *noise);

octavenoise_t *octavenoise_create(rng_t *rng, int numoctaves);
double octavenoise_compute(octavenoise_t *noise, double x, double y);
void octavenoise_destroy(octavenoise_t *noise);

combinednoise_t *combinednoise_create(octavenoise_t *n1, octavenoise_t *n2);
double combinednoise_compute(combinednoise_t *noise, double x, double y);
void combinednoise_destroy(combinednoise_t *noise);


#endif