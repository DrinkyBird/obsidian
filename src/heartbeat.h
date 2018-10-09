#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#define SALT_LENGTH 32

void *heartbeat_run(void *);

void heartbeat_init();
void heartbeat_tick();
void heartbeat_shutdown();

char *heartbeat_get_salt();

#endif