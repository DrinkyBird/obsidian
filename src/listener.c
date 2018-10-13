#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include "listener.h"
#include "connection.h"
#include "platform.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif

#define BACKLOG 10

static void listener_accept(listener_t *listener);
static void listener_tick_connections();
static int find_free_connection_index();

int num_connections;
connection_t **connections;

listener_t *listener_create(uint16_t port, int num_conns) {
    listener_t *listener = malloc(sizeof(*listener));
    listener->port = port;

    int err;
    err = (listener->socket_fd = socket(AF_INET, SOCK_STREAM, 0));

    if (err == -1) {
        sock_perror("socket");
        return NULL;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);

    int yes = 1;
    err = setsockopt(listener->socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (err == -1) {
        sock_perror("setsockopt(SO_REUSEADDR)");
        return NULL;
    }

    err = bind(listener->socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
        sock_perror("bind");
        return NULL;
    }

    err = listen(listener->socket_fd, BACKLOG);
    if (err == -1) {
        sock_perror("listen");
        return NULL;
    }

    ioctlsocket(listener->socket_fd, FIONBIO, &yes);

    connections = calloc(num_conns, sizeof(*connections));
    num_connections = num_conns;

    return listener;
}

void listener_tick(listener_t *listener) {
    listener_accept(listener);
    listener_tick_connections();
}

void listener_tick_connections() {
    for (int i = 0; i < num_connections; i++) {
        if (connections[i] == NULL) {
            continue;
        }

        connection_tick(connections[i]);
    }
}

void listener_accept(listener_t *listener) {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);

    int acceptfd = accept(listener->socket_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (acceptfd == -1) {
        int e = sock_error();
        if (e == SOCKERR_EAGAIN || e == SOCKERR_EWOULDBLOCK) {
            /* no incoming connections */
            return;
        }

        sock_perror("accept");
        return;
    }

    int yes = 1;
    ioctlsocket(acceptfd, FIONBIO, &yes);
    struct sockaddr_in *sin = (struct sockaddr_in*)&client_addr;

    unsigned char *ip = (unsigned char *)&sin->sin_addr.s_addr;
    printf("Incoming connection from: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

    connection_t *conn = connection_create(acceptfd);
    int i = find_free_connection_index();

    if (i == -1) {
        connection_disconnect(conn, "Server is full");
        return;
    }

    conn->id = i;

    connections[i] = conn;
}

int find_free_connection_index() {
    for (int i = 0; i < num_connections; i++) {
        if (connections[i] == NULL) {
            return i;
        }
    }

    return -1;
}

void listener_destroy(listener_t *listener) {
    for (int i = 0; i < num_connections; i++) {
        if (connections[i] == NULL) {
            continue;
        }

        connection_disconnect(connections[i], "Server shutting down");
    }

    close(listener->socket_fd);
}

void broadcast_rw(rw_t *rw) {
    for (int i = 0; i < num_connections; i++) {
        if (connections[i] == NULL) {
            continue;
        }

        connection_write_rw(connections[i], rw);
    }
}

void broadcast_msg(const char *msg) {
    rw_t *packet = packet_create();
    rw_write_byte(packet, PACKET_MESSAGE);
    rw_write_byte(packet, 0);
    rw_write_mc_str(packet, msg);
    broadcast_rw(packet);

    rw_destroy_and_buffer(packet);
}

void broadcast_op_action(player_t *source, const char *f, ...) {
    char act[64];
    char buf[64];

    va_list args;
    va_start(args, f);
    vsnprintf(act, sizeof(act), f, args);
    va_end(args);

    snprintf(buf, sizeof(buf), "&7[%s: %s]", source->name, act);

    for (int i = 0; i < num_connections; i++) {
        connection_t *c = connections[i];
        if (c == NULL || c->player == NULL) continue;
        if (!c->player->op) continue;

        if (c->player == source) {
            connection_msg(c, act);
        } else {
            connection_msg(c, buf);
        }
    }
}

int listener_get_active_connections() {
    int n = 0;

    for (int i = 0; i < num_connections; i++) {
        if (connections[i] != NULL) {
            n++;
        }
    }

    return n;
}