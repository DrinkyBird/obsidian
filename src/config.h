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
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdbool.h>

typedef struct config_s {
    unsigned short port;
    int maxplayers;
    bool verifynames;
    bool should_heartbeat;

    const char *name, *motd;

    const char *map_name;
    const char *map_generator;
    int width, depth, height; 
} config_t;

extern config_t *configuration;

bool config_parse();
bool config_reload();

#endif