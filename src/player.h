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
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <stdbool.h>
#include "defs.h"

struct connection_s;

typedef struct player_s {
    int id;
    const char *name;

    float x, y, z, yaw, pitch;
    bool op;
    bool spawned;

    struct connection_s *conn;
} player_t;

void playerman_init(int players);
void playerman_deinit();
int playerman_get_num_online();

player_t *player_get_by_name(const char *name);

player_t *player_create(struct connection_s *conn);
void player_destroy(player_t *player);
void player_spawn(player_t *player);
void player_set_op(player_t *player, bool op);
void player_teleport(player_t *player, float x, float y, float z);

bool player_is_block_admin_only(block_e b);

void player_broadcast_movement(player_t *player);

#endif