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
#ifndef __NBT_H__
#define __NBT_H__

#include <stdbool.h>
#include "rw.h"
#include "defs.h"

typedef enum tag_e {
    tag_end,
    tag_byte,
    tag_short,
    tag_int,
    tag_long,
    tag_float,
    tag_double,
    tag_byte_array,
    tag_string,
    tag_list,
    tag_compound,

    num_tags
} tag_e;

typedef struct tag_s {
    char *name;
    tag_e type;

    union {
        char b;
        short s;
        int i;
        long long l;
        float f;
        double d;
        byte *pb;
        char *str;
        struct tag_s **list;
    };

    /* for entries in lists */
    bool no_header;

    /* for byte_array, list, compound tags */
    int array_size;

    /* for tag_list */
    tag_e array_type;
} tag_t;

tag_t *nbt_create(const char *name);
tag_t *nbt_create_compound(const char *name);
tag_t *nbt_create_string(const char *name, const char *val);
tag_t *nbt_create_bytearray(const char *name, byte *val, int len);
tag_t *nbt_copy_bytearray(const char *name, byte *val, int len);

void nbt_destroy(tag_t *tag, bool destroy_values);

void nbt_set_char(tag_t *tag, char b);
void nbt_set_short(tag_t *tag, short b);
void nbt_set_int(tag_t *tag, int b);
void nbt_set_long(tag_t *tag, long long b);
void nbt_add_tag(tag_t *tag, tag_t *new);

tag_t *nbt_get_tag(tag_t *tag, const char *n);

void nbt_write(tag_t *tag, rw_t *rw);
tag_t *nbt_read(rw_t *rw, bool named);

void nbt_dump(tag_t *tag, int indent);

#endif