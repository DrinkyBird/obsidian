#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <stdint.h>
#include "rw.h"
#include "platform.h"
#include "player.h"

typedef struct listener_s {
    uint16_t port;
    socket_t socket_fd;
} listener_t;

listener_t *listener_create(uint16_t port, int connections);
void listener_destroy(listener_t *listener);
void listener_tick(listener_t *listener);

void broadcast_rw(rw_t *rw);
void broadcast_msg(const char *msg);
void broadcast_op_action(player_t *source, const char *f, ...);

/* for map_set */
void broadcast_block_change(int x, int y, int z, block_e b);

int listener_get_active_connections();

#endif // __LISTENER_H