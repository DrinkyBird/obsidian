#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <pthread.h>
#include "defs.h"
#include "rw.h"
#include "player.h"
#include "cpe.h"
#include "platform.h"

#define CONN_OUT_BUFFER_SIZE 2048

typedef struct connection_s {
    socket_t id;
    int fd;
    const char *name;
    const char *key;

    const char *software;

    player_t *player;

    byte *out_buf;
    rw_t *out_rw;

    pthread_t *mapgz_thread;
    bool thread_running, thread_successful, thread_joined, mapgz_sent;

    byte *mapgz_data;
    rw_t *mapgz_rw;
    int mapgz_size;

    int last_ping;
    bool is_connected;
    bool fd_open;

    int num_extensions;
    cpeext_t **extensions;
    int ext_index;
} connection_t;

connection_t *connection_create(int fd);
void connection_destroy(connection_t *conn);
void connection_disconnect(connection_t *conn, const char *reason);

void connection_tick(connection_t *conn);
void connection_write(connection_t *conn, void *buf, int len);
void connection_write_rw(connection_t *conn, rw_t *rw);
void connection_msg(connection_t *conn, const char *msg);
void connection_msgf(connection_t *conn, const char *f, ...);

rw_t *packet_create();
void packet_send(rw_t *packet, connection_t *conn);

typedef struct mapgz_data_s {
    connection_t *conn;
    byte *blocks;
    int width, depth, height, num_blocks;
} mapgz_data_t;

void *mapgz_run(void *data);

#endif