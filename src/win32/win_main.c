#include <stdlib.h>
#include <stdio.h>
#include "../platform.h"
#include "../player.h"

void platform_init() {
    SetConsoleTitleA("miniclassic");

    WORD versionReq = MAKEWORD(2, 2);
    WSADATA data;

    int err = WSAStartup(versionReq, &data);
    if (err != 0) {
        fprintf(stderr, "WSAStartup failed with %d\n", err);
        exit(0);
    }

    puts("Successfully initialised Winsock.\n");
}

void platform_tick() {
    char buf[128];
    snprintf(buf, sizeof(buf), "miniclassic - %d players online", playerman_get_num_online());

    SetConsoleTitleA(buf);
}

void platform_shutdown() {
    WSACleanup();
}

const char *platform_get_name() {
    return "Windows NT";
}

const char *platform_get_version() {
    return "";
}