#include <stdio.h>
#include <string.h>
#include "connection.h"
#include "player.h"
#include "commands.h"
#include "namelist.h"

extern namelist_t *adminlist;
extern namelist_t *banlist;

extern int num_players;
extern player_t **players;

void basecmd_kick(int argc, char **argv, player_t *player) {
    if (!player->op) {
        connection_msg(player->conn, "&cYou do not have permission to use this command");
        return;
    }

    if (argc != 2) {
        connection_msg(player->conn, "Syntax: /kick <name>");
        return;
    }

    const char *name = argv[1];

    for (int i = 0; i < num_players; i++) {
        player_t *p = players[i];
        if (p == NULL) continue;

        if (strcasecmp(p->name, name) == 0) {
            connection_disconnect(p->conn, "You have been kicked from the server!");
            return;
        }
    }

    connection_msgf(player->conn, "There is no such player by the name %s, name");
}

void basecmd_ban(int argc, char **argv, player_t *player) {
    if (!player->op) {
        connection_msg(player->conn, "&cYou do not have permission to use this command");
        return;
    }
    
    if (argc != 2) {
        connection_msg(player->conn, "Syntax: /ban <name>");
        return;
    }

    const char *name = argv[1];

    if (namelist_contains(banlist, name)) {
        connection_msgf(player->conn, "%s is already on the banlist", name);
        return;
    }

    namelist_add(banlist, name);

    for (int i = 0; i < num_players; i++) {
        player_t *p = players[i];
        if (p == NULL) continue;

        if (strcasecmp(p->name, name) == 0) {
            connection_disconnect(p->conn, "You have been banned from this server!");
            break;
        }
    }

    connection_msgf(player->conn, "%s has been added to the banlist", name);
}

void basecmd_unban(int argc, char **argv, player_t *player) {
    if (!player->op) {
        connection_msg(player->conn, "&cYou do not have permission to use this command");
        return;
    }
    
    if (argc != 2) {
        connection_msg(player->conn, "Syntax: /unban <name>");
        return;
    }

    const char *name = argv[1];

    if (namelist_remove(banlist, name)) {
        connection_msgf(player->conn, "%s has been removed from the banlist", name);
    } else {
        connection_msgf(player->conn, "&c%s is not on the banlist", name);
    }
}

void basecmds_init() {
    command_register("kick", basecmd_kick);
    command_register("ban", basecmd_ban);
    command_register("unban", basecmd_unban);
}