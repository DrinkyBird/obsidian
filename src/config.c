#include <stdlib.h>
#include <string.h>
#include "ini.h"
#include "config.h"

#define INI_MATCH(s, n) ((strcasecmp(section, s) == 0) && (strcasecmp(name, n) == 0))

config_t *configuration;

static int config_handle_ini(void* user, const char* section, const char* name, const char* value);

bool config_parse() {
    configuration = malloc(sizeof(config_t));

    if (ini_parse("settings.ini", config_handle_ini, NULL) < 0) {
        printf("Failed to parse settings.ini\n");
        return false;
    }

    return true;
}

int config_handle_ini(void* user, const char* section, const char* name, const char* value) {
    if (strcasecmp(section, "server") == 0) {
        if (strcasecmp(name, "port") == 0) {
            configuration->port = (unsigned short) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "players") == 0) {
            configuration->maxplayers = (int) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "name") == 0) {
            configuration->name = value;
        } else if (strcasecmp(name, "motd") == 0) {
            configuration->motd = value;
        } else if (strcasecmp(name, "verifynames") == 0) {
            configuration->verifynames = (bool) strtoul(value, NULL, 10);
        }
    }

    else if (strcasecmp(section, "world") == 0) {
        if (strcasecmp(name, "width") == 0) {
            configuration->width = (int) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "depth") == 0) {
            configuration->depth = (int) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "height") == 0) {
            configuration->height = (int) strtoul(value, NULL, 10);
        }
    }

    return 1;
}