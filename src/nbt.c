#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "nbt.h"
#include "rw.h"

static void write_len_str(rw_t *rw, const char *s);

static tag_t *tag_create_empty();

tag_t *tag_create_empty() {
    tag_t *t = malloc(sizeof(tag_t));

    t->name = NULL;
    t->no_header = true;
    t->array_size = 0;

    return t;
}

tag_t *nbt_create(const char *name) {
    tag_t *t = tag_create_empty();
    t->name = (char *)name;
    t->no_header = false;
    return t;
}

tag_t *nbt_create_compound(const char *name) {
    tag_t * t = nbt_create(name);
    t->type = tag_compound;
    t->array_size = 32;
    t->list = calloc(t->array_size, sizeof(*t->list));

    return t;
}

tag_t *nbt_create_string(const char *name, const char *val){
    tag_t *t = nbt_create(name);
    t->type = tag_string;
    
    t->array_size = strlen(val);
    t->str = malloc(t->array_size);
    strcpy(t->str, val);

    return t;
}

tag_t *nbt_create_bytearray(const char *name, byte *val, int len) {
    tag_t *t = nbt_create(name);
    t->type = tag_byte_array;
    t->array_size = len;
    t->pb = val;

    return t;
}

tag_t *nbt_copy_bytearray(const char *name, byte *val, int len) {
    byte *a = malloc(len);
    memcpy(a, val, len);

    return nbt_create_bytearray(name, a, len);
}

void nbt_destroy(tag_t *tag, bool destroy_values) {
    if (tag->type == tag_list || tag->type == tag_compound) {
        for (int i = 0; i < tag->array_size; i++) {
            tag_t *t = tag->list[i];
            nbt_destroy(t, destroy_values);
        }
    }

    if (!tag->no_header) {
        if (tag->name != NULL) {
            free(tag->name);
        }
    }

    if (tag->type == tag_string && destroy_values) {
        free(tag->str);
    }

    if (tag->type == tag_byte_array && destroy_values) {
        free(tag->pb);
    }

    free(tag);
}

void nbt_write(tag_t *tag, rw_t *rw) {
    if (!tag->no_header) {
        rw_write_byte(rw, tag->type);

        if (tag->name != NULL) {
            write_len_str(rw, tag->name);
        }
    }

    switch (tag->type) {
        case tag_end:
            break;
        
        case tag_byte:
            rw_write_byte(rw, tag->b);
            break;

        case tag_short:
            rw_write_int16be(rw, tag->s);
            break;

        case tag_int:
            rw_write_int32be(rw, tag->i);
            break;

        case tag_long:
            rw_write_int16be(rw, tag->l);
            break;

        case tag_float:
            rw_write_floatbe(rw, tag->f);
            break;

        case tag_double:
            rw_write_doublebe(rw, tag->f);
            break;

        case tag_byte_array:
            rw_write_int32be(rw, tag->array_size);
            rw_write(rw, tag->pb, tag->array_size);
            break;

        case tag_string:
            write_len_str(rw, tag->str);
            break;

        case tag_list:
            rw_write_byte(rw, (byte)tag->array_type);
            rw_write_int32le(rw, tag->array_size);

            for (int i = 0; i < tag->array_size; i++) {
                if (tag->list[i] == NULL) continue;
                
                nbt_write(tag->list[i], rw);
            }

            break;

        case tag_compound:
            for (int i = 0; i < tag->array_size; i++) {
                if (tag->list[i] == NULL) continue;

                nbt_write(tag->list[i], rw);
            }

            rw_write_byte(rw, tag_end);

            break;
    }
}

void write_len_str(rw_t *rw, const char *s) {
    unsigned short len = (unsigned short) strlen(s);

    rw_write_int16be(rw, len);
    rw_write(rw, (void *)s, len);
}

void nbt_set_char(tag_t *tag, char b) {
    tag->type = tag_byte;
    tag->b = b;
}

void nbt_set_short(tag_t *tag, short b) {
    tag->type = tag_short;
    tag->s = b;
}

void nbt_set_int(tag_t *tag, int b) {
    tag->type = tag_int;
    tag->i = b;
}

void nbt_set_long(tag_t *tag, long long b) {
    tag->type = tag_long;
    tag->l = b;
}

void nbt_add_tag(tag_t *tag, tag_t *new) {
    if ((tag->type != tag_list && tag->type != tag_compound) || tag->list == NULL) {
        return;
    }

    int i;
    bool found = false;
    for (i = 0; i < tag->array_size; i++) {
        if (tag->list[i] == NULL) {
            found = true;
            break;
        }
    }

    if (!found) {
        printf("no room for new element in %p\n", tag);
        return;
    }

    tag->list[i] = new;
}

tag_t *nbt_read(rw_t *rw, bool named) {
    tag_t *t = tag_create_empty();
    t->type = rw_read_byte(rw);

    if (named) {
        int namelen = rw_read_int16be(rw);
        t->name = malloc(namelen + 1);
        rw_read(rw, t->name, namelen);
        t->name[namelen] = 0;
    }

    switch (t->type) {
        case tag_end:
            break;

        case tag_byte:
            t->b = rw_read_char(rw);
            break;

        case tag_short:
            t->s = rw_read_int16be(rw);
            break;

        case tag_int:
            t->i = rw_read_int32be(rw);
            break;

        case tag_float:
            t->f = rw_read_floatbe(rw);
            break;

        case tag_double:
            t->f = rw_read_doublebe(rw);
            break;

        case tag_byte_array:
            t->array_size = rw_read_int32be(rw);
            t->pb = malloc(t->array_size);
            rw_read(rw, t->pb, t->array_size);

            break;

        case tag_string:
            t->array_size = rw_read_int16be(rw);
            t->str = malloc(t->array_size);
            rw_read(rw, t->str, t->array_size);

            break;

        case tag_list:
            t->array_type = (tag_e) rw_read_byte(rw);
            t->array_size = rw_read_int32be(rw);

            break;

        case tag_compound:
            t->array_size = 0;
            tag_t *subtag;
            int i = 0;
            int off = rw_tell(rw);

            /* count how many sub-tags we have */
            while ((subtag = nbt_read(rw, true))->type != tag_end) {
                t->array_size++;
            }            

            t->list = calloc(t->array_size, sizeof(*t->list));

            rw_seek(rw, off, rw_set);

            while ((subtag = nbt_read(rw, true))->type != tag_end) {
                t->list[i] = subtag;
                i++;
            }

            break;

        default:
            printf("attempt to read unknown tag type %d\n", t->type);
            
            nbt_destroy(t, true);

            return NULL;
    }

    return t;
}

tag_t *nbt_get_tag(tag_t *tag, const char *n) {
    if (tag->type != tag_compound) {
        return NULL;
    }

    for (int i = 0; i < tag->array_size; i++) {
        tag_t *t = tag->list[i];
        if (t == NULL) {
            continue;
        }

        if (strcmp(t->name, n) == 0) {
            return t;
        }
    }

    fprintf(stderr, "tag %s has no sub-tag %s\n", tag->name, n);

    return NULL;
}