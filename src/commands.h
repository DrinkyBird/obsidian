#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <stdbool.h>
#include "player.h"

typedef void (*command_callback_t)(int /*argc*/, char ** /*argv*/, player_t * /*player*/);

typedef struct command_s {
    const char *name;
    command_callback_t callback;
} command_t;

void commands_init();
void commands_execute(const char *msg, player_t *player);
command_t *command_register(const char *name, command_callback_t callback);
void commands_shutdown();

void basecmds_init();

#endif