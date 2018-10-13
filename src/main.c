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
#include "version.h"
#include "commands.h"

#define IS_OPT(n) (strcmp(long_options[option_index].name, n) == 0) 

listener_t *listener;
map_t *map;

int current_tick = 0;

static void tick();
static void handle_sigint(int);

static bool running = true;

static char *full_name = NULL;

static const struct option long_options[] = {
    { "port", required_argument, NULL, 0 },
    { "max-players", required_argument, NULL, 0 },
    { "version", no_argument, NULL, 0 },
    {NULL, no_argument, NULL, 0}
};

int main(int argc, char *argv[]) {
    int c, option_index;

    srand(time(NULL));
    
    platform_init();

    full_name = malloc(64);
    snprintf(full_name, 64, "miniclassic v%s on %s %s", VERSION_STR, platform_get_name(), platform_get_version());

    if (!config_parse()) {
        return 1;
    }

    while ((c = getopt_long(argc, argv, "", (const struct option *)&long_options, &option_index)) != -1) {
        switch (c) {
            case 0: {
                if (IS_OPT("version")) {
                    printf("%s\n", VERSION_STR);
                    printf("%s\n", app_get_full_name());
                    printf("compiled with %s %s for %s-endian %s %s\n", VERSION_COMPILER_ID, VERSION_COMPILER_VER, VERSION_HOST_ENDIAN, VERSION_HOST_FAMILY, VERSION_HOST_OS);
                    return 0;
                }

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

    commands_init();

    printf("%s\n", full_name);

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

    commands_shutdown();
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

const char *app_get_full_name() {
    return full_name;
}