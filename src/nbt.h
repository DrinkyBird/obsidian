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
    tag_compound
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

#endif