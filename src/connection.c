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
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include "listener.h"
#include "connection.h"
#include "defs.h"
#include "rw.h"
#include "map.h"
#include "cpe.h"
#include "md5.h"
#include "heartbeat.h"
#include "config.h"
#include "platform.h"
#include "commands.h"
#include "namelist.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif

#define IN_BUF_SIZE 8192

static bool connection_handle_packet(connection_t *conn, unsigned char id, rw_t* rw);
static void connection_flush_out(connection_t *conn);
static void connection_start_mapgz(connection_t *conn);
static void connection_send_mapgz(connection_t *conn);
static void connection_ping(connection_t *conn);
static void connection_perror(connection_t *conn, const char *s);
static bool connection_verify_key(connection_t *conn);

extern map_t *map;
extern int current_tick;
extern int num_connections;
extern connection_t **connections;
extern namelist_t *adminlist;
extern namelist_t *banlist;

connection_t *connection_create(int fd) {
    connection_t *conn = malloc(sizeof(*conn));
    conn->id = -1;
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
    conn->last_ping = 0;
    conn->is_connected = true;
    conn->fd_open = true;
    conn->num_extensions = 0;
    conn->extensions = NULL;
    conn->ext_index = 0;

    memset(conn->out_buf, 0, CONN_OUT_BUFFER_SIZE);

    return conn;
}

void connection_destroy(connection_t *conn) {
    if (conn->id != -1) {
        connections[conn->id] = NULL;
    }

    if (conn->player != NULL) {
        player_destroy(conn->player);
    }

    if (conn->extensions != NULL) {
        for (int i = 0; i < conn->num_extensions; i++) {
            free(conn->extensions[i]);
        }

        free(conn->extensions);
    }

    conn->is_connected = false;

    close(conn->fd);
    rw_destroy(conn->out_rw);
    free(conn->out_buf);
    free(conn);

    /* save the map on empty server */
    if (listener_get_active_connections() == 0) {
        map_save(map);
    }
}

void connection_tick(connection_t *conn) {
    if (!conn->is_connected || !conn->fd_open) {
        return;
    }

    if (!conn->mapgz_sent && conn->thread_successful) {
        connection_send_mapgz(conn);
    }

    connection_ping(conn);
    connection_flush_out(conn);

    unsigned char *buf = malloc(IN_BUF_SIZE);
    
    int len;

    len = recv(conn->fd, buf, IN_BUF_SIZE, 0);
    if (len == -1) {
        free(buf);

        int e = sock_error();

        if (e == SOCKERR_EAGAIN || e == SOCKERR_EWOULDBLOCK) {
            /* no data */
            return;
        }

        conn->fd_open = false;

        if (e == ECONNRESET || e == EPIPE) {
            connection_disconnect(conn, "Client quit");
            return;
        }

        connection_perror(conn, "recv");
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

            if (version != PROTOCOL_VERSION) {
                connection_disconnect(conn, "Invalid protocol version");
                return false;
            }

            conn->name = rw_read_mc_str(rw);
            conn->key = rw_read_mc_str(rw);

            if (!connection_verify_key(conn)) {
                connection_disconnect(conn, "Authentication failure");
                return false;
            }

            if (namelist_contains(banlist, conn->name)) {
                connection_disconnect(conn, "You are banned from this server!");
                return false;
            }

            byte unused = rw_read_byte(rw);

            /* check for name conflicts */
            for (int i = 0; i < num_connections; i++) {
                if (connections[i] == NULL || connections[i] == conn) {
                    continue;
                }

                if (strcasecmp(conn->name, connections[i]->name) == 0) {
                    connection_disconnect(conn, "Name already taken");
                    return false;
                }
            }

            bool supportsCpe = unused == 0x42;
            bool isOp = namelist_contains(adminlist, conn->name);

            rw_t *packet = packet_create();
            rw_write_byte(packet, PACKET_IDENT);
            rw_write_byte(packet, 7);
            rw_write_mc_str(packet, configuration->name);
            rw_write_mc_str(packet, configuration->motd);
            rw_write_byte(packet, isOp ? 0x64 : 0x00);
            packet_send(packet, conn);

            connection_flush_out(conn);

            packet = packet_create();
            rw_write_byte(packet, PACKET_LEVEL_INIT);
            packet_send(packet, conn);

            if (supportsCpe) {
                int numexts;
                cpeext_t *exts = cpe_get_supported_exts(&numexts);

                packet = packet_create();
                rw_write_byte(packet, PACKET_EXTINFO);
                rw_write_mc_str(packet, app_get_full_name());
                rw_write_int16be(packet, numexts);
                packet_send(packet, conn);

                for (int i = 0; i < numexts; i++) {
                    cpeext_t ext = exts[i];

                    packet = packet_create();
                    rw_write_byte(packet, PACKET_EXTENTRY);
                    rw_write_mc_str(packet, ext.name);
                    rw_write_int32be(packet, ext.version);
                    packet_send(packet, conn);
                }
            }

            connection_start_mapgz(conn);

            break;
        }

        case PACKET_MESSAGE: {
            char buf[64];
            rw_read_byte(rw);

            const char *msg = rw_read_mc_str(rw);

            if (conn->player == NULL) {
                break;
            }

            if (msg[0] == '/') {
                commands_execute(msg, conn->player);
                break;
            }

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

            player_broadcast_movement(conn->player);

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

        case PACKET_EXTINFO: {
            conn->software = rw_read_mc_str(rw);
            conn->num_extensions = rw_read_int16be(rw);
            conn->extensions = calloc(conn->num_extensions, sizeof(cpeext_t));

            break;
        }

        case PACKET_EXTENTRY: {
            cpeext_t *ext = malloc(sizeof(cpeext_t));
            ext->name = rw_read_mc_str(rw);
            ext->version = rw_read_int32be(rw);

            conn->extensions[conn->ext_index++] = ext;

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
    if (!conn->is_connected || !conn->fd_open) {
        return;
    }

    int len = rw_tell(conn->out_rw);
    if (len < 1) {
        return;
    }

    int sendflags = 0;
#ifdef __linux__
    sendflags |= MSG_NOSIGNAL;
#endif

    int l;
    l = send(conn->fd, conn->out_buf, len, sendflags);
    rw_seek(conn->out_rw, 0, rw_set);

    if (l == -1) {
        int e = sock_error();

        if (e == SOCKERR_EAGAIN || e == SOCKERR_EWOULDBLOCK) {
            return;
        }

        conn->fd_open = false;

        if (e == ECONNRESET || e == EPIPE) {
            connection_disconnect(conn, "Client quit");
            return;
        }

        connection_perror(conn, "send");
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
    connection_write(conn, buf, originaloffset);

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
    for (int i = 0; i < 4; i++) {
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

        if (rw_tell(conn->mapgz_rw) == rw_size(conn->mapgz_rw)) {
            free(conn->mapgz_data);
            rw_destroy(conn->mapgz_rw);

            conn->mapgz_data = NULL;

            conn->mapgz_sent = true;

            packet = packet_create();
            rw_write_byte(packet, PACKET_LEVEL_FINISH);
            rw_write_int16be(packet, map->width);
            rw_write_int16be(packet, map->depth);
            rw_write_int16be(packet, map->height);
            packet_send(packet, conn);

            conn->player = player_create(conn);
            if (conn->player == NULL) {
                connection_disconnect(conn, "Failed to create player instance");
                return;
            }

            conn->player->op = namelist_contains(adminlist, conn->name);

            player_spawn(conn->player);

            break;
        }
    }
}

void connection_msg(connection_t *conn, const char *msg) {
    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_MESSAGE);
    rw_write_byte(packet, 0);
    rw_write_mc_str(packet, msg);
    packet_send(packet, conn);
}

void connection_msgf(connection_t *conn, const char *f, ...) {
    char buf[64];

    va_list args;
    va_start(args, f);
    vsnprintf(buf, sizeof(buf), f, args);
    connection_msg(conn, buf);
    va_end(args);
}

void connection_disconnect(connection_t *conn, const char *reason) {
    if (!conn->is_connected) {
        return;
    }

    /* we don't need to send other packets if we're disconnecting */
    rw_seek(conn->out_rw, 0, rw_set);

    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_PLAYER_DISCONNECT);
    rw_write_mc_str(packet, reason);
    packet_send(packet, conn);
    connection_flush_out(conn);

    printf("%s left the game. (%s)\n", conn->name, reason);

    if (conn->player != NULL) {
        char buf[64];
        snprintf(buf, 64, "&e%s left the game. (%s)", conn->name, reason);
        broadcast_msg(buf);
    }

    connection_destroy(conn);
}

void connection_ping(connection_t *conn) {
    int delta = current_tick - conn->last_ping;
    if (delta < 5 * TICKRATE) {
        return;
    }

    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_PING);
    packet_send(packet, conn);
}

void connection_perror(connection_t *conn, const char *s) {
    char *err = strerror(errno);

    char buf[256];
    snprintf(buf, sizeof(buf), "%s: %s", s, err);

    conn->fd_open = false;

    connection_disconnect(conn, buf);
}

bool connection_verify_key(connection_t *conn) {
#ifndef ENABLE_HEARTBEAT
    return true;
#else
    if (!configuration->verifynames) {
        return true;
    }

    struct MD5Context md;
    unsigned char output[16];
    char digest[33];

    MD5Init(&md);
    MD5Update(&md, (const byte *)heartbeat_get_salt(), SALT_LENGTH);
    MD5Update(&md, (const byte *)conn->name, strlen(conn->name));
    MD5Final(output, &md);

    for (int i = 0; i < 16; i++) {
        int i2 = i * 2;
        snprintf(digest + i2, sizeof(digest) - i2, "%02x", output[i]);
    }

    bool valid = (strcasecmp(conn->key, digest) == 0);

    if (!valid) {
        fprintf(stderr, "auth failure: %s != %s\n", digest, conn->key);
    }

    return valid;
#endif
}