#include <stdlib.h>
#include <stdio.h>
#include "listener.h"
#include "connection.h"
#include "player.h"
#include "map.h"
#include "rw.h"
#include "defs.h"

int num_players;
player_t **players;

extern map_t *map;

static int find_player_index();

void playerman_init(int numplayers) {
    num_players = numplayers;
    players = calloc(numplayers, sizeof(player_t));

    for (int i = 0; i < num_players; i++) {
        players[i] = NULL;
    }
}

player_t *player_create(connection_t *conn) {
    int id = find_player_index();
    if (id == -1) {
        return NULL;
    }

    player_t *player = malloc(sizeof(*player));

    player->id = id;
    player->conn = conn;
    player->name = conn->name;
    player->x = 0.0f;
    player->y = 0.0f;
    player->z = 0.0f;
    player->op = false;
    player->spawned = false;

    players[id] = player;

    return player;
}

void player_destroy(player_t *player) {
    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_PLAYER_DESPAWN);
    rw_write_char(packet, player->id);

    for (int i = 0; i < num_players; i++) {
        player_t *p = players[i];

        if (p == NULL || p == player) {
            continue;
        }

        connection_write_rw(p->conn, packet);
    }

    rw_destroy_and_buffer(packet);

    players[player->id] = NULL;
    free(player);
}

void player_spawn(player_t *player) {
    player->x = rrand(0, map->width);
    player->y = map->depth;
    player->z = rrand(0, map->height);

    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_PLAYER_SPAWN);
    rw_write_char(packet, -1);
    rw_write_mc_str(packet, player->name);
    rw_write_int16be(packet, TOFIXED(player->x));
    rw_write_int16be(packet, TOFIXED(player->y));
    rw_write_int16be(packet, TOFIXED(player->z));
    rw_write_byte(packet, FIXEDANGLE(player->yaw));
    rw_write_byte(packet, FIXEDANGLE(player->pitch));
    packet_send(packet, player->conn);

    /* will be broadcasted to already existing players */
    rw_t *broadcastpacket = packet_create();
    rw_write_byte(broadcastpacket, PACKET_PLAYER_SPAWN);
    rw_write_char(broadcastpacket, player->id);
    rw_write_mc_str(broadcastpacket, player->name);
    rw_write_int16be(broadcastpacket, TOFIXED(player->x));
    rw_write_int16be(broadcastpacket, TOFIXED(player->y));
    rw_write_int16be(broadcastpacket, TOFIXED(player->z));
    rw_write_byte(broadcastpacket, FIXEDANGLE(player->yaw));
    rw_write_byte(broadcastpacket, FIXEDANGLE(player->pitch));

    for (int i = 0; i < num_players; i++) {
        if (players[i] == NULL || players[i] == player) {
            continue;
        }

        player_t *p = players[i];

        /* tell the new player about this player */
        packet = packet_create();
        rw_write_byte(packet, PACKET_PLAYER_SPAWN);
        rw_write_char(packet, p->id);
        rw_write_mc_str(packet, p->name);
        rw_write_int16be(packet, TOFIXED(p->x));
        rw_write_int16be(packet, TOFIXED(p->y));
        rw_write_int16be(packet, TOFIXED(p->z));
        rw_write_byte(packet, FIXEDANGLE(p->yaw));
        rw_write_byte(packet, FIXEDANGLE(p->pitch));
        packet_send(packet, player->conn);

        connection_write_rw(p->conn, broadcastpacket);
    }

    rw_destroy_and_buffer(broadcastpacket);

    player->spawned = true;

    char buf[64];
    snprintf(buf, 64, "&e%s joined the game.", player->name);
    printf("%s joined the game.\n", player->name);
    broadcast_msg(buf);
}

bool player_is_block_admin_only(block_e b) {
    return (
           b == water
        || b == water_still
        || b == lava
        || b == lava_still
        || b == bedrock
    );
}

int find_player_index() {
    for (int i = 0; i < num_players; i++) {
        if (players[i] == NULL) {
            return i;
        }
    }

    return -1;
}

int playerman_get_num_online() {
    int n = 0;

    for (int i = 0; i < num_players; i++) {
        if (players[i] != NULL) {
            n++;
        }
    }

    return n;
}

void player_broadcast_movement(player_t *player) {
    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_PLAYER_POS_AND_ANGLE);
    rw_write_char(packet, player->id);
    rw_write_int16be(packet, TOFIXED(player->x));
    rw_write_int16be(packet, TOFIXED(player->y));
    rw_write_int16be(packet, TOFIXED(player->z));
    rw_write_byte(packet, FIXEDANGLE(player->yaw));
    rw_write_byte(packet, FIXEDANGLE(player->pitch));

    for (int i = 0; i < num_players; i++) {
        player_t *p = players[i];

        if (p == NULL || p == player) {
            continue;
        }

        connection_write_rw(p->conn, packet);
    }

    rw_destroy_and_buffer(packet);
}