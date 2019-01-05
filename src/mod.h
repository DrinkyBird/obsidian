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
#ifndef __SCRIPT_H__
#define __SCRIPT_H__

typedef struct lua_State lua_State;

typedef struct mod_s {
    const char *id;
    const char *dirpath;

    const char *main_file;

    lua_State *L;

    const char *load_callback;
} mod_t;

void modman_init();
void modman_shutdown();

void mod_register_globals(mod_t *mod);

#endif