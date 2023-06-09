/*
    Obsidian, a lightweight Classicube server
    Copyright (C) 2018-2019 Sean Baggaley.

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
#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "connection.h"
#include "platform.h"

#define NUM_COMMANDS 64
#define NUM_ARGS 64
static command_t **commands;

static void basecmd_help(int argc, char **argv, player_t *player);

void commands_init() {
    commands = calloc(NUM_COMMANDS, sizeof(*commands));

    command_register("help", basecmd_help);
    basecmds_init();
}

void commands_execute(const char *msg, player_t *player) {
    if (msg[0] != '/') {
        return;
    }

    msg++;

    char *sep1 = (char *)msg;
    char **argv;
    char *token;
    char *command;
    int argc = 0, i = 0;

    argv = calloc(NUM_ARGS, sizeof(*argv));

    while ((token = platform_strsep(&sep1, " "))) {
        argv[argc] = token;
        argc++;
    }

    command = argv[0];

    bool found = false;
    for (i = 0; i < NUM_COMMANDS; i++) {
        if (commands[i] == NULL) {
            continue;
        }

        if (strcasecmp(command, commands[i]->name) == 0) {
            commands[i]->callback(argc, argv, player);
            found = true;
            break;
        }
    }
    
    if (!found) {
        connection_msg(player->conn, "&cUnknown command.");
    }

    free(argv);
}

void commands_shutdown() {
    for (int i = 0; i < NUM_COMMANDS; i++) {
        command_t *c = commands[i];
        if (c == NULL) continue;
        free(c);
    }

    free(commands);
}

command_t *command_register(const char *name, command_callback_t callback) {
    int i;
    bool found = false;
    for (i = 0; i < NUM_COMMANDS; i++) {
        if (commands[i] == NULL) {
            found = true;
            break;
        }
    }

    if (!found) {
        fprintf(stderr, "ran out of room to register commands (max %d)\n", NUM_COMMANDS);
        return NULL;
    }

    command_t *cmd = malloc(sizeof(*cmd));
    cmd->name = name;
    cmd->callback = callback;

    commands[i] = cmd;

    return cmd;
}

void basecmd_help(int argc, char **argv, player_t *player){
    for (int i = 0; i < NUM_COMMANDS; i++) {
        command_t *cmd = commands[i];

        if (cmd == NULL) continue;

        connection_msgf(player->conn, "&e* &f/%s", cmd->name);
    } 
}