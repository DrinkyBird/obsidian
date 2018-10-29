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
#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <stdbool.h>
#include "player.h"

typedef void (*command_callback_t)(int /*argc*/, char ** /*argv*/, player_t * /*player*/);

typedef struct command_s {
    const char *name;
    command_callback_t callback;
} command_t;

void commands_init();
void commands_execute(const char *msg, player_t *player);
command_t *command_register(const char *name, command_callback_t callback);
void commands_shutdown();

void basecmds_init();

#endif