#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "connection.h"
#include "player.h"
#include "commands.h"
#include "namelist.h"
#include "listener.h"

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
            broadcast_op_action(player, "Kicked %s", p->name);
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

    broadcast_op_action(player, "Banned %s", name);
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
        broadcast_op_action(player, "Unbanned %s", name);
    } else {
        connection_msgf(player->conn, "&c%s is not on the banlist", name);
    }
}

void basecmd_op(int argc, char **argv, player_t *player) {
    if (!player->op) {
        connection_msg(player->conn, "&cYou do not have permission to use this command");
        return;
    }
    
    if (argc != 2) {
        connection_msg(player->conn, "Syntax: /op <name>");
        return;
    }

    const char *name = argv[1];

    player_t *p = player_get_by_name(name);

    if (p != NULL) {
        player_set_op(p, true);
        name = p->name;
    }

    namelist_add(adminlist, name);

    broadcast_op_action(player, "Opped %s", name);
}

void basecmd_deop(int argc, char **argv, player_t *player) {
    if (!player->op) {
        connection_msg(player->conn, "&cYou do not have permission to use this command");
        return;
    }
    
    if (argc != 2) {
        connection_msg(player->conn, "Syntax: /deop <name>");
        return;
    }

    const char *name = argv[1];

    player_t *p = player_get_by_name(name);

    if (p != NULL) {
        name = p->name;
    }

    if (!namelist_contains(adminlist, name)) {
        connection_msgf(player->conn, "%s is not an operator", name);
        return;
    }

    namelist_remove(adminlist, name);

    if (p != NULL) {
        player_set_op(p, false);
    }

    broadcast_op_action(player, "De-opped %s", name);
}

void basecmd_whisper(int argc, char **argv, player_t *player) {
    if (argc < 3) {
        connection_msg(player->conn, "Syntax: /whisper <target> <message>");
        return;
    }

    const char *name = argv[1];
    player_t *target = player_get_by_name(name);
    if (target == NULL) { 
        connection_msgf(player->conn, "No such player by the name %s", name);
        return;
    }

    char msg[64];
    memset(msg, 0, sizeof(msg));
    int p = 0;

    for (int i = 2; i < argc; i++) {
        int l = strlen(argv[i]);
        strcpy(msg + p, argv[i]);
        p += l;
        msg[p++] = ' ';
    }

    connection_msgf(player->conn, "&e[-> %s] &f%s", target->name, msg);
    connection_msgf(target->conn, "&e[%s ->] &f%s", player->name, msg);
}

void basecmd_tp(int argc, char **argv, player_t *player) {
    if (!player->op) {
        connection_msg(player->conn, "&cYou do not have permission to use this command");
        return;
    }

    if (argc < 2 || argc > 5) {
        connection_msg(player->conn, "Syntax: /tp [player] <target>");
        connection_msg(player->conn, "     or /tp [player] <x> <y> <z>");
        return;
    }

    /* teleport to position */
    if (argc >= 4) {
        int i = 0;
        player_t *target = player;

        if (argc == 5) {
            target = player_get_by_name(argv[i + 1]);
            i++;

            if (target == NULL) {
                connection_msgf(player->conn, "No player named %s", argv[1]);
                return;
            }
        }

        float x = atof(argv[i + 1]);
        float y = atof(argv[i + 2]) + 1.59375f;
        float z = atof(argv[i + 3]);

        player_teleport(target, x, y, z);
        broadcast_op_action(player, "Teleported %s to [%f, %f, %f]", target->name, x, y, z);
    } else {
        player_t *who = player;
        int i = 0;
        if (argc == 3) {
            who = player_get_by_name(argv[1]);
            i++;

            if (who == NULL) {
                connection_msgf(player->conn, "No player named %s", argv[1]);
                return;
            }
        }

        player_t *dest = player_get_by_name(argv[i + 1]);
        if (dest == NULL) {
            connection_msgf(player->conn, "No player named %s", argv[1]);
            return;
        }

        player_teleport(who, dest->x, dest->y, dest->z);
        broadcast_op_action(player, "Teleported %s to %s", who->name, dest->name);
    }
}

void basecmd_whois(int argc, char **argv, player_t *player) {
    if (argc != 2) {
        connection_msg(player->conn, "Syntax: /whois <name>");
        return;
    }

    const char *name = argv[1];

    player_t *p = player_get_by_name(name);

    if (p == NULL) {
        connection_msgf(player->conn, "No player named %s", name);
        return;
    }

    name = p->name;

    connection_msgf(player->conn, "&eInformation on &f%s&e (ID &f%d&e):", name, p->id);
    connection_msgf(player->conn, "* &eX: &f%f &eY: &f%f &eZ: &f%f", name, p->x, p->y, p->z);
    connection_msgf(player->conn, "* &eYaw: &f%f &ePitch: &f%f", name, p->yaw, p->pitch);
    connection_msgf(player->conn, "* &f%s is %san operator", name, p->op ? "" : "not ");

    if (player->conn->software != NULL)
        connection_msgf(player->conn, "* &f%s is using &f%s", name, p->conn->software);
}

void basecmds_init() {
    command_register("kick", basecmd_kick);
    command_register("ban", basecmd_ban);
    command_register("unban", basecmd_unban);
    command_register("op", basecmd_op);
    command_register("deop", basecmd_deop);
    command_register("whisper", basecmd_whisper);
    command_register("tp", basecmd_tp);
    command_register("whois", basecmd_whois);
}