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

void playerman_init(int numplayers) {
    num_players = num_players;
    players = calloc(numplayers, sizeof(*players));
}

player_t *player_create(connection_t *conn) {
    player_t *player = malloc(sizeof(*player));

    player->conn = conn;
    player->name = conn->name;
    player->x = 0.0f;
    player->y = 0.0f;
    player->z = 0.0f;
    player->op = false;

    return player;
}

void player_destroy(player_t *player) {
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
    rw_write_byte(packet, 0);
    rw_write_byte(packet, 0);
    packet_send(packet, player->conn);

    packet = packet_create();
    rw_write_byte(packet, PACKET_PLAYER_POS_AND_ANGLE);
    rw_write_char(packet, -1);
    rw_write_int16be(packet, TOFIXED(player->x));
    rw_write_int16be(packet, TOFIXED(player->y));
    rw_write_int16be(packet, TOFIXED(player->z));
    rw_write_byte(packet, 0);
    rw_write_byte(packet, 0);
    packet_send(packet, player->conn);

    char buf[64];
    snprintf(buf, 64, "&e%s joined the game.", player->name);
    printf("%s joined the game\n", player->name);
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