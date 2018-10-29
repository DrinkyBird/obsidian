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
#include <string.h>
#include "ini.h"
#include "config.h"

#define INI_MATCH(s, n) ((strcasecmp(section, s) == 0) && (strcasecmp(name, n) == 0))
#define COPYSTR(d, s) d = calloc(strlen(s), sizeof(char)); strcpy((char *)d, s);

config_t *configuration;

static int config_handle_ini(void* user, const char* section, const char* name, const char* value, int line);
static bool boolify(const char *c, int line);

bool config_parse() {
    configuration = malloc(sizeof(config_t));

    if (ini_parse("settings.ini", config_handle_ini, NULL) < 0) {
        printf("Failed to parse settings.ini\n");
        return false;
    }

    return true;
}

bool config_reload() {
    config_t *oldconf = configuration;

    if (!config_parse()) {
        free(configuration);
        configuration = oldconf;

        return false;
    }

    free(oldconf);
    
    return true;
}

int config_handle_ini(void* user, const char* section, const char* name, const char* value, int line) {
    if (strcasecmp(section, "server") == 0) {
        if (strcasecmp(name, "port") == 0) {
            configuration->port = (unsigned short) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "players") == 0) {
            configuration->maxplayers = (int) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "name") == 0) {
            COPYSTR(configuration->name, value);
        } else if (strcasecmp(name, "motd") == 0) {
            COPYSTR(configuration->motd, value);
        } else if (strcasecmp(name, "verifynames") == 0) {
            configuration->verifynames = boolify(value, line);
        } else if (strcasecmp(name, "heartbeat") == 0) {
            configuration->should_heartbeat = boolify(value, line);
        }
    }

    else if (strcasecmp(section, "world") == 0) {
        if (strcasecmp(name, "name") == 0) {
            COPYSTR(configuration->map_name, value);
        } else if (strcasecmp(name, "width") == 0) {
            configuration->width = (int) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "depth") == 0) {
            configuration->depth = (int) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "height") == 0) {
            configuration->height = (int) strtoul(value, NULL, 10);
        } else if (strcasecmp(name, "generator") == 0) {
            COPYSTR(configuration->map_generator, value);
        }
    }

    return 1;
}

bool boolify(const char *c, int line) {
    if (strcasecmp(c, "true") == 0 || strcasecmp(c, "yes") == 0 || strcasecmp(c, "on") == 0) {
        return true;
    } else if (strcasecmp(c, "false") == 0 || strcasecmp(c, "no") == 0 || strcasecmp(c, "off") == 0) {
        return false;
    }

    fprintf(stderr, "settings.ini: \"%s\" on line %d is not a valid boolean value, assuming false\n", c, line);
    return false;
}