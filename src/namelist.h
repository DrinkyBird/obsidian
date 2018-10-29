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
#ifndef __NAMELIST_H__
#define __NAMELIST_H__

#include <stdbool.h>

typedef struct namelist_s {
    char **names;
    int num_names;

    char *filename;
} namelist_t;

namelist_t *namelist_create(int num);
namelist_t *namelist_read_file(const char *fn);
void namelist_destroy(namelist_t *nl);

bool namelist_add(namelist_t *nl, const char *name);
bool namelist_remove(namelist_t *nl, const char *name);
bool namelist_contains(namelist_t *nl, const char *name);

bool namelist_write_file(namelist_t *nl, const char *fn);

#endif