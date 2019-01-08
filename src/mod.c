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
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fts.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#include <lua5.3/lualib.h>
#include "mod.h"
#include "ini.h"

#define COPYSTR(d, s) d = calloc(strlen(s), sizeof(char)); strcpy((char *)d, s);

static int modman_find_free_index();

static mod_t *mod_create(const char *id, const char *dirpath);
static void mod_destroy(mod_t *mod);

static int mod_handle_ini(void* user, const char* section, const char* name, const char* value, int line);

mod_t **mods = NULL;
int num_mods = 0;

void modman_init() {
    /* create mods array */
    num_mods = 32;
    mods = calloc(num_mods, sizeof(*mods));

    /* check if the mods dir exists - if not, create it */
    DIR *dir = opendir("mods");
    if (dir) {
        closedir(dir);
    } else if (errno == ENOENT) {
        mode_t mask = umask(0);
        umask(mask);
        if (mkdir("mods", 0777 - mask) == -1) {
            perror("Failed to create mods directory.");
            return;
        }
    } else {
        perror("Failed to open the mods directory.");
        return;
    }

    /* Look for all mod.ini files */
    FTS *fts = NULL;
    FTSENT *p = NULL, *chp = NULL;

    char *paths[] = {
        "mods/",
        NULL
    };

    if ((fts = fts_open(paths, FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR, NULL)) == NULL) {
        fputs("Failed to fts_open on the mods directory.\n", stderr);
        return;
    }

    if ((chp = fts_children(fts, 0)) == NULL) {
        fputs("Failed to fts_children on the mods directory.\n", stderr);
        return;
    }

    while ((p = fts_read(fts)) != NULL) {
        /* ignore self */
        if (p->fts_name[0] == '\0') {
            continue;
        }

        switch (p->fts_info) {
            case FTS_D: {
                char *full = malloc(PATH_MAX);
                char *ini = malloc(PATH_MAX);

                realpath(p->fts_path, full);
                realpath(p->fts_path, ini);

                /* check for mod.ini */
                strncat(ini, "/mod.ini", PATH_MAX);

                if (access(ini, R_OK) == -1) {
                    char buf[512];
                    snprintf(buf, sizeof(buf), "Cannot access %s for reading", ini);
                    perror(buf);
                    free(full);
                    continue;
                }

                free(ini);

                mod_t *mod = mod_create(p->fts_name, full);

                break;
            }

            default: {
                break;
            }
        }
    }
}

void modman_shutdown() {
    for (int i = 0; i < num_mods; i++) {
        mod_t *mod = mods[i];
        if (mod == NULL) {
            continue;
        }

        mod_destroy(mod);
        mods[i] = NULL;
    }
}

int modman_find_free_index() {
    for (int i = 0; i < num_mods; i++) {
        if (mods[i] == NULL) {
            return i;
        }
    }

    /* none free, realloc and try again */
    num_mods *= 2;
    mods = realloc(mods, num_mods);
    return modman_find_free_index();
}

mod_t *mod_create(const char *id, const char *dirpath) {
    mod_t *mod = malloc(sizeof(mod_t));

    mod->id = id;
    mod->dirpath = dirpath;

    char *ini = malloc(PATH_MAX);
    strncpy(ini, dirpath, PATH_MAX);
    strcat(ini, "/mod.ini");

    if (ini_parse(ini, &mod_handle_ini, mod)) {
        fprintf(stderr, "Failed to parse %s\n", ini);
        free(ini);
        free(mod);
        return NULL;
    }

    free(ini);

    mod->L = luaL_newstate();
    luaL_openlibs(mod->L);

    /* load the lua file */
    if (luaL_dofile(mod->L, mod->main_file)) {
        fprintf(stderr, "%s\n", lua_tostring(mod->L, -1));
        
    }

    mod_register_globals(mod);

    /* call onload function */
    lua_getglobal(mod->L, mod->load_callback);
    
    if (lua_pcall(mod->L, 0, 0, 0) != 0) {
        fprintf(stderr, "Failed to call load callback %s: %s\n", mod->load_callback, lua_tostring(mod->L, -1));
    }

    mods[modman_find_free_index()] = mod;

    return mod;
}

int mod_handle_ini(void* user, const char* section, const char* name, const char* value, int line) {
    mod_t *mod = (mod_t *)user;

    if (strcasecmp(section, "mod") == 0) {
        if (strcasecmp(name, "main") == 0) {
            char *main = malloc(PATH_MAX);
            strncpy(main, mod->dirpath, PATH_MAX);
            strcat(main, "/");
            strcat(main, value);
            mod->main_file = main;
        }
    }

    else if (strcasecmp(section, "callbacks") == 0) {
        if (strcasecmp(name, "load") == 0) {
            COPYSTR(mod->load_callback, value);
        }
    }

    return 1;
}

void mod_destroy(mod_t *mod) {
    lua_close(mod->L);

    free((void *)mod->dirpath);
    free((void *)mod->id);
    free(mod);
}