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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "namelist.h"

#define NUM_NAMES 128
#define R_NAMEBUF_SIZE 64

namelist_t *namelist_create(int num) {
    namelist_t *nl = malloc(sizeof(namelist_t));

    nl->num_names = num;
    nl->names = calloc(nl->num_names, sizeof(*nl->names));
    nl->filename = NULL;

    return nl;
}

namelist_t *namelist_read_file(const char *fn) {
    FILE *f = fopen(fn, "r");
    if (f == NULL) {
        fprintf(stderr, "namelist: failed to open %s\n", fn);
        return NULL;
    }

    char c;
    int num_lines = 0;

    fseek(f, 0, SEEK_SET);
    while ((fread(&c, sizeof(c), 1, f)) == sizeof(c)) {
        if (c == '\n') {
            num_lines++;
        }
    }

    namelist_t *nl = namelist_create(num_lines + 128);
    nl->filename = calloc(strlen(fn) + 1, sizeof(char));
    strcpy(nl->filename, fn);

    int i = 0;
    bool comment = false;

    char *namebuf = malloc(R_NAMEBUF_SIZE);
    memset(namebuf, 0, R_NAMEBUF_SIZE);
    fseek(f, 0, SEEK_SET);
    while ((fread(&c, sizeof(c), 1, f)) == sizeof(c)) {
        if (c == '\r') {
            continue;
        }

        else if (c == '\n') {
            if (comment) {
                comment = false;
            } else if (i > 0) {
                char *n = malloc(i + 1);
                memset(n, 0, i + 1);
                strcpy(n, namebuf);

                namelist_add(nl, n);
            }

            i = 0;
            memset(namebuf, 0, R_NAMEBUF_SIZE);
        }

        else {
            if (comment) {
                continue;
            }

            if (i == 0 && c == '#') {
                comment = true;
                continue;
            }

            namebuf[i] = c;
            i++;
        }
    }

    free(namebuf);

    return nl;
}

void namelist_destroy(namelist_t *nl) {
    if (nl->filename != NULL) {
        namelist_write_file(nl, nl->filename);
        free(nl->filename);
        nl->filename = NULL;
    }

    free(nl->names);

    free(nl);
}

bool namelist_add(namelist_t *nl, const char *name) {
    if (namelist_contains(nl, name)) {
        return false;
    }
    
    for (int i = 0; i < nl->num_names; i++) {
        if (nl->names[i] == NULL) {
            nl->names[i] = (char *)name;

            if (nl->filename) {
                namelist_write_file(nl, nl->filename);
            }

            return true;
        }
    }

    fprintf(stderr, "namelist: no room for any more names (max %d)\n", nl->num_names);
    return false;
}

bool namelist_remove(namelist_t *nl, const char *name) {
    for (int i = 0; i < nl->num_names; i++) {
        if (nl->names[i] == NULL) {
            continue;
        }

        if (strcasecmp(nl->names[i], name) == 0) {
            nl->names[i] = NULL;

            if (nl->filename) {
                namelist_write_file(nl, nl->filename);
            }

            return true;
        }
    }

    return false;
}

bool namelist_contains(namelist_t *nl, const char *name) {
    for (int i = 0; i < nl->num_names; i++) {
        const char *n = nl->names[i];
        if (n == NULL) {
            continue;
        }

        if (strcasecmp(n, name) == 0) {
            return true;
        }
    }

    return false;
}

bool namelist_write_file(namelist_t *nl, const char *fn) {
    if (fn == NULL && nl->filename != NULL) {
        fn = nl->filename;
    }

    FILE *f = fopen(fn, "w");
    if (f == NULL) {
        fprintf(stderr, "namelist: failed to open %s for writing\n", fn);
        return false;
    }

#ifdef _WIN32
    const char *lf = "\r\n";
#else
    const char *lf = "\n";
#endif

    for (int i = 0; i < nl->num_names; i++) {
        const char *n = nl->names[i];
        if (n == NULL) {
            continue;
        }

        fwrite(n, strlen(n), 1, f);
        fwrite(lf, strlen(lf), 1, f);
    }

    fclose(f);

    return true;
}