#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "listener.h"
#include "connection.h"
#include "defs.h"
#include "rw.h"
#include "map.h"
#include <SDL/SDL_endian.h>

#define IN_BUF_SIZE 2048

static bool connection_handle_packet(connection_t *conn, unsigned char id, rw_t* rw);
static void connection_flush_out(connection_t *conn);
static void connection_start_mapgz(connection_t *conn);
static void connection_send_mapgz(connection_t *conn);

extern map_t *map;

static int packetId = 0;

connection_t *connection_create(int fd) {
    connection_t *conn = malloc(sizeof(*conn));
    conn->fd = fd;
    conn->out_buf = malloc(CONN_OUT_BUFFER_SIZE);
    conn->out_rw = rw_create(conn->out_buf, CONN_OUT_BUFFER_SIZE);
    conn->mapgz_data = NULL;
    conn->mapgz_rw = NULL;
    conn->thread_running = false;
    conn->thread_successful = false;
    conn->thread_joined = false;
    conn->mapgz_sent = false;
    conn->player = NULL;

    return conn;
}

void connection_destroy(connection_t *conn) {
    close(conn->fd);
    rw_destroy(conn->out_rw);
    free(conn->out_buf);
    free(conn);
}

void connection_tick(connection_t *conn) {
    unsigned char *buf = malloc(IN_BUF_SIZE);
    int len;

    if (!conn->mapgz_sent && conn->thread_successful) {
        connection_send_mapgz(conn);
    }

    connection_flush_out(conn);

    len = recv(conn->fd, buf, IN_BUF_SIZE, 0);
    if (len == -1) {
        free(buf);

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            /* no data */
            return;
        }

        perror("recv");
        return;
    }

    if (len == 0) {
        /* empty buffer? */
        free(buf);
        return;
    }

    rw_t *rw = rw_create(buf, len);
    int id;

    rw_seek(rw, 0, rw_set);
    while ((int)rw_tell(rw) < len - 1) {
        id = rw_read_byte(rw);
        if (!connection_handle_packet(conn, id, rw))
            break;
    }

    rw_destroy(rw);

    free(buf);
}

bool connection_handle_packet(connection_t *conn, unsigned char id, rw_t* rw) {
    switch (id) {
        case PACKET_IDENT: {
            byte version = rw_read_byte(rw);
            conn->name = rw_read_mc_str(rw);
            conn->key = rw_read_mc_str(rw);
            byte unused = rw_read_byte(rw);

            bool supportsCpe = unused == 0x42;
            const char *v = supportsCpe ? "supports CPE" : "doesn't support CPE";

            rw_t *packet = packet_create();
            rw_write_byte(packet, PACKET_IDENT);
            rw_write_byte(packet, 7);
            rw_write_mc_str(packet, "test");
            rw_write_mc_str(packet, "testmotd");
            rw_write_byte(packet, 0x64);
            packet_send(packet, conn);

            connection_flush_out(conn);

            packet = packet_create();
            rw_write_byte(packet, PACKET_LEVEL_INIT);
            packet_send(packet, conn);

            connection_start_mapgz(conn);

            break;
        }

        case PACKET_MESSAGE: {
            char buf[64];
            byte unused = rw_read_byte(rw);

            const char *msg = rw_read_mc_str(rw);

            snprintf(buf, 64, "<%s> %s", conn->name, msg);
            printf("%s\n", buf);

            broadcast_msg(buf);

            break;
        }

        case PACKET_PLAYER_POS_AND_ANGLE: {
            if (!conn->player) {
                printf("Client %s sent movement packed without a player\n", conn->name);
                return false;
            }

            rw_read_byte(rw);
            conn->player->x = TOFLOAT(rw_read_int16be(rw));
            conn->player->y = TOFLOAT(rw_read_int16be(rw));
            conn->player->z = TOFLOAT(rw_read_int16be(rw));
            conn->player->yaw = FLOATANGLE(rw_read_byte(rw));
            conn->player->pitch = FLOATANGLE(rw_read_byte(rw));

            break;
        }

        case PACKET_SET_BLOCK_CLIENT: {
            if (!conn->player) {
                printf("Client %s sent setblock packed without a player\n", conn->name);
                return false;
            }

            int x = rw_read_int16be(rw);
            int y = rw_read_int16be(rw);
            int z = rw_read_int16be(rw);
            int destroyed = rw_read_byte(rw) == 0;
            block_e block = (block_e) rw_read_byte(rw);
            block_e oldBlock = (block_e) map_get(map, x, y, z);

            if (!conn->player->op) {
                if (player_is_block_admin_only(block) && !destroyed) {
                    connection_msg(conn, "&cYou are not allowed to place that block");
                } else if (player_is_block_admin_only(oldBlock)) {
                    connection_msg(conn, "&cYou are not allowed to modify that block");
                } else {
                    goto nofix;
                }

                rw_t *packet = packet_create();
                rw_write_byte(packet, PACKET_SET_BLOCK_SERVER);
                rw_write_int16be(packet, x);
                rw_write_int16be(packet, y);
                rw_write_int16be(packet, z);
                rw_write_byte(packet, oldBlock);
                packet_send(packet, conn);

                break;
            }

nofix:

            if (destroyed) {
                block = air;
            }

            map_set(map, x, y, z, block);

            break;
        }

        default: {
            printf("Client %s sent invalid packet ID %d\n", conn->name, id);
            return false;
        }
    }

    return true;
}

void connection_flush_out(connection_t *conn) {
    int len = rw_tell(conn->out_rw);
    if (len < 1) {
        return;
    }

    int l;
    l = send(conn->fd, conn->out_buf, len, 0);
    rw_seek(conn->out_rw, 0, rw_set);

    printf("DBG flushed %d (%d)\n", packetId, l);
    packetId++;

    if (l == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }

        perror("send");
        return;
    }
}

void connection_write(connection_t *conn, void *buf, int len) {
    if (rw_tell(conn->out_rw) + len >= CONN_OUT_BUFFER_SIZE) {
        connection_flush_out(conn);
    }

    rw_write(conn->out_rw, buf, len);
}

void connection_write_rw(connection_t *conn, rw_t *rw) {
    int originaloffset = rw_tell(rw);
    rw_seek(rw, 0, rw_set);

    byte *buf = malloc(originaloffset);
    rw_read(rw, buf, originaloffset);
    rw_write(conn->out_rw, buf, originaloffset);

    free(buf);
    rw_seek(rw, originaloffset, rw_set);
}

void connection_start_mapgz(connection_t *conn) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    mapgz_data_t *data = malloc(sizeof(*data));
    data->conn = conn;
    data->width = map->width;
    data->depth = map->depth;
    data->height = map->height;
    data->num_blocks = (map->width * map->depth * map->height);
    data->blocks = malloc(data->num_blocks + 4);

    rw_t *rw = rw_create(data->blocks, data->num_blocks + 4);

    printf("size=%d\n", rw_size(rw));
    rw_write_int32be(rw, data->num_blocks);

    for (int i = 0; i < data->num_blocks; i++) {
        rw_write_byte(rw, (byte)map->blocks[i]);
    }

    rw_destroy(rw);

    pthread_t thread;
    pthread_create(&thread, &attr, mapgz_run, data);
    conn->thread_running = true;

    conn->mapgz_thread = &thread;
}

rw_t *packet_create() {
    rw_t *rw = rw_create_empty(2048);
    return rw;
}

void packet_send(rw_t *packet, connection_t *conn) {
    connection_write_rw(conn, packet);
    rw_destroy_and_buffer(packet);
}

void connection_send_mapgz(connection_t *conn) {
    byte *buf = malloc(1024);
    memset(buf, 0, 1024);

    int r = rw_read(conn->mapgz_rw, buf, 1024);

    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_LEVEL_CHUNK);
    rw_write_int16be(packet, r);
    rw_write(packet, buf, 1024);
    rw_write_byte(packet, 0);
    packet_send(packet, conn);

    connection_flush_out(conn);

    free(buf);

    printf("%d/%d\n", rw_tell(conn->mapgz_rw), rw_size(conn->mapgz_rw));

    if (rw_tell(conn->mapgz_rw) == rw_size(conn->mapgz_rw)) {
        free(conn->mapgz_data);
        rw_destroy(conn->mapgz_rw);

        conn->mapgz_sent = true;

        printf("Sent level to %s\n", conn->name);
        packet = packet_create();
        rw_write_byte(packet, PACKET_LEVEL_FINISH);
        rw_write_int16be(packet, map->width);
        rw_write_int16be(packet, map->depth);
        rw_write_int16be(packet, map->height);
        packet_send(packet, conn);

        conn->player = player_create(conn);
        player_spawn(conn->player);
    }
}

void connection_msg(connection_t *conn, const char *msg) {
    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_MESSAGE);
    rw_write_byte(packet, 0);
    rw_write_mc_str(packet, msg);
    packet_send(packet, conn);
}