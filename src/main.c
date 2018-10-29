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
#include "namelist.h"
#include "nbt.h"

#define IS_OPT(n) (strcmp(long_options[option_index].name, n) == 0) 

listener_t *listener;
map_t *map;

int current_tick = 0;

static void tick();
static void handle_sigint(int);
static namelist_t *read_or_create_namelist(const char *fn);

static void util_nbtdump(const char *filename);

namelist_t *banlist = NULL;
namelist_t *adminlist = NULL;

rng_t *global_rng = NULL;

bool running = true;

static char *full_name = NULL;

static const struct option long_options[] = {
    { "port", required_argument, NULL, 0 },
    { "max-players", required_argument, NULL, 0 },
    { "version", no_argument, NULL, 0 },
    { "dump-nbt", required_argument, NULL, 0 },
    {NULL, no_argument, NULL, 0}
};

int main(int argc, char *argv[]) {
    int c, option_index;
    
    platform_init();

    global_rng = rng_create((int)time(NULL));

    full_name = malloc(64);
    snprintf(full_name, 64, "obsidian v%s on %s %s", VERSION_STR, platform_get_name(), platform_get_version());

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

                if (IS_OPT("dump-nbt")) {
                    util_nbtdump(optarg);
                    return 0;
                }

                break;
            }
        }
    }

    banlist = read_or_create_namelist("banlist.txt");
    adminlist = read_or_create_namelist("adminlist.txt");

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
    playerman_deinit();
    
#ifdef ENABLE_HEARTBEAT
    heartbeat_shutdown();
#endif

    map_save(map);
    map_destroy(map);

    commands_shutdown();

    namelist_destroy(adminlist);
    namelist_destroy(banlist);

    rng_destroy(global_rng);

    free(full_name);
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
    if (!running) {
        return;
    }
    
    printf("Caught SIGINT.\n");
    running = false;
}

const char *app_get_full_name() {
    return full_name;
}

namelist_t *read_or_create_namelist(const char *fn) {
    namelist_t *nl;
    nl = namelist_read_file(fn);

    if (nl == NULL) {
        printf("%s does not exist, creating it.\n", fn);
        nl = namelist_create(128);
        nl->filename = (char *)fn;
    }

    return nl;
}

void util_nbtdump(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "%s: failed to open file: ", filename);
        perror("");
        return;
    }

    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);

    byte *buf = calloc(len, sizeof(byte));
    fread(buf, len, 1, f);
    fclose(f);

    rw_t *rw = rw_create(buf, len);

    byte first = rw_read_byte(rw);
    if (first < 0 || first >= num_tags) {
        fprintf(stderr, "%s: not a valid nbt file: expected a tag type, got 0x%02x\n", filename, first);
        return;
    }

    rw_seek(rw, 0, rw_set);

    tag_t *tag = nbt_read(rw, true);
    nbt_dump(tag, 0);
    rw_destroy(rw);

    free(buf);
}