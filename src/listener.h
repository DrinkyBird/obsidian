#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <stdint.h>

typedef struct listener_s {
    uint16_t port;
    int socket_fd;
} listener_t;

listener_t *listener_create(uint16_t port, int connections);
void listener_destroy(listener_t *listener);
void listener_tick(listener_t *listener);

#endif // __LISTENER_H