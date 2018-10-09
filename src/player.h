#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <stdbool.h>

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

player_t *player_create(struct connection_s *conn);
void player_destroy(player_t *player);
void player_spawn(player_t *player);
bool player_is_block_admin_only(block_e);

#endif