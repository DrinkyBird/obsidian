#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <getopt.h>
#include <string.h>
#include "listener.h"
#include "defs.h"
#include "player.h"
#include "map.h"
#include "heartbeat.h"
#include "config.h"
#include "platform.h"

#define IS_OPT(n) (strcmp(long_options[option_index].name, n) == 0) 

listener_t *listener;
map_t *map;

int current_tick = 0;

static void tick();
static void handle_sigint(int);

static bool running = true;

static const struct option long_options[] = {
    { "port", required_argument, NULL, 0 },
    { "max-players", required_argument, NULL, 0 },
    {NULL, no_argument, NULL, 0}
};

int main(int argc, char *argv[]) {
    int c, option_index;

    if (!config_parse()) {
        return 1;
    }

    while ((c = getopt_long(argc, argv, "", &long_options, &option_index)) != -1) {
        switch (c) {
            case 0: {
                if (IS_OPT("port")) {
                    configuration->port = (unsigned short) strtoul(optarg, NULL, 10);
                }

                if (IS_OPT("max-players")) {
                    configuration->maxplayers = (int) strtoul(optarg, NULL, 10);
                }

                break;
            }
        }
    }

    srand(time(NULL));
    
    platform_init();

    int width = configuration->width;
    int depth = configuration->depth;
    int height = configuration->height;

    printf("Creating map with size [%d, %d, %d]\n", width, depth, height);
    map = map_load(configuration->map_name);
    if (map == NULL) {
        map = map_create(configuration->map_name, width, depth, height);
        map_generate(map);
        map_save(map);
    }

#ifdef ENABLE_HEARTBEAT
    heartbeat_init();
#endif
    printf("Starting server on port %hu\n", configuration->port);

    playerman_init(configuration->maxplayers);

    listener = listener_create(configuration->port, configuration->maxplayers);
    if (listener == NULL) {
        fprintf(stderr, "Failed to create listener on port %hu\n", configuration->port);
        return 1;
    }

    printf("Server is now ready to accept connections\n\n");

    signal(SIGINT, handle_sigint);

    while (running) {
        tick();
        usleep((1000 / TICKRATE) * 1000);
    }

    listener_destroy(listener);
    
#ifdef ENABLE_HEARTBEAT
    heartbeat_shutdown();
#endif

    map_save(map);

    platform_shutdown();

    return 0;
}

void tick() {
    listener_tick(listener);

#ifdef ENABLE_HEARTBEAT
    heartbeat_tick();
#endif

    platform_tick();

    current_tick++;
}

void handle_sigint(int d) {
    printf("Caught SIGINT.\n");
    running = false;
}

int rrand(int min, int max) {
    return (rand()%(max-min))+min;
}