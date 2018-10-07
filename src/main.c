#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "listener.h"
#include "defs.h"
#include "map.h"

listener_t *listener;
map_t *map;

static void tick();
static void handle_sigint(int);

static bool running = true;

int main(int argc, char *argv[]) {
    int width = 256;
    int depth = 256;
    int height = 256;
    printf("Creating map with size [%d, %d, %d]\n", width, depth, height);
    map = map_create(width, depth, height);
    map_generate(map);

    unsigned short port = 25565;
    int players = 16;
    printf("Starting listener on %hu\n", port);

    listener = listener_create(port, players);
    if (listener == NULL) {
        fprintf(stderr, "Failed to create listener on port %hu\n", port);
        return 1;
    }

    printf("Listening...\n\n");

    signal(SIGINT, handle_sigint);

    while (running) {
        tick();
        usleep((1000 / TICKRATE) * 1000);
    }

    listener_destroy(listener);

    return 0;
}

void tick() {
    listener_tick(listener);
}

void handle_sigint(int d) {
    printf("Caught SIGINT.");
    running = false;
}