#include <stdlib.h>
#include <stdio.h>
#include "../platform.h"

void platform_init() {
    WORD versionReq = MAKEWORD(2, 2);
    WSADATA data;

    int err = WSAStartup(versionReq, &data);
    if (err != 0) {
        fprintf(stderr, "WSAStartup failed with %d\n", err);
        exit(0);
    }

    puts("Successfully initialised Winsock.\n");
}

void platform_shutdown() {
    WSACleanup();
}