#ifndef __NAMELIST_H__
#define __NAMELIST_H__

#include <stdbool.h>

typedef struct namelist_s {
    char **names;
    int num_names;

    const char *filename;
} namelist_t;

namelist_t *namelist_create(int num);
namelist_t *namelist_read_file(const char *fn);
void namelist_destroy(namelist_t *nl);

bool namelist_add(namelist_t *nl, const char *name);
bool namelist_remove(namelist_t *nl, const char *name);
bool namelist_contains(namelist_t *nl, const char *name);

bool namelist_write_file(namelist_t *nl, const char *fn);

#endif