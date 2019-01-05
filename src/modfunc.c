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
#include <lua5.3/lua.h>
#include <lua5.3/lauxlib.h>
#include <lua5.3/lualib.h>
#include "defs.h"
#include "mod.h"
#include "map.h"

static int obsidian_lua_get_version(lua_State *L);

static int obsidian_lua_map_get_width(lua_State *L);
static int obsidian_lua_map_get_depth(lua_State *L);
static int obsidian_lua_map_get_height(lua_State *L);
static int obsidian_lua_map_get_block(lua_State *L);
static int obsidian_lua_map_set_block(lua_State *L);

static const struct luaL_Reg obsidian_global[] = {
    { "get_version", obsidian_lua_get_version },
    { NULL, NULL }
};

static const struct luaL_Reg obsidian_map_funcs[] = {
    { "get_width", obsidian_lua_map_get_width },
    { "get_depth", obsidian_lua_map_get_depth },
    { "get_height", obsidian_lua_map_get_height },
    { "get", obsidian_lua_map_get_block },
    { "set", obsidian_lua_map_set_block },
    { NULL, NULL }
};

extern map_t *map;

void mod_register_globals(mod_t *mod) {
    lua_newtable(mod->L);
    luaL_setfuncs(mod->L, obsidian_global, 0);

    lua_pushstring(mod->L, "map");
    lua_newtable(mod->L);
    luaL_setfuncs(mod->L, obsidian_map_funcs, 0);
    lua_settable(mod->L, -3);

    lua_setglobal(mod->L, "obsidian");
}

/*********************************************************************************************************************/
/* Map object type */

int obsidian_lua_get_version(lua_State *L) {
    lua_pushstring(L, app_get_full_name());
    return 1;
}


int obsidian_lua_map_get_width(lua_State *L) {
    lua_pushnumber(L, map->width);
    return 1;
}

int obsidian_lua_map_get_depth(lua_State *L) {
    lua_pushnumber(L, map->depth);
    return 1;
}

int obsidian_lua_map_get_height(lua_State *L) {
    lua_pushnumber(L, map->height);
    return 1;
}

int obsidian_lua_map_get_block(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int z = luaL_checkinteger(L, 3);

    luaL_argcheck(L, x >= 1 && x < map->width, 1, "x out of range");
    luaL_argcheck(L, y >= 1 && y < map->depth, 2, "y out of range");
    luaL_argcheck(L, z >= 1 && z < map->height, 3, "z out of range");

    block_e block = map_get(map, x - 1, y - 1, z - 1);

    lua_pushnumber(L, block);

    return 1;
}

int obsidian_lua_map_set_block(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int z = luaL_checkinteger(L, 3);
    block_e block = luaL_checkinteger(L, 4);

    luaL_argcheck(L, x >= 1 && x < map->width, 1, "x out of range");
    luaL_argcheck(L, y >= 1 && y < map->depth, 2, "y out of range");
    luaL_argcheck(L, z >= 1 && z < map->height, 3, "z out of range");
    luaL_argcheck(L, block >= 0 && block < num_blocks, 4, "invalid block id");

    map_set(map, x - 1, y - 1, z - 1, block);

    return 0;
}