#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdbool.h>

typedef struct config_s {
    unsigned short port;
    int maxplayers;
    bool verifynames;
    bool should_heartbeat;

    const char *name, *motd;

    const char *map_name;
    const char *map_generator;
    int width, depth, height; 
} config_t;

extern config_t *configuration;

bool config_parse();

#endif