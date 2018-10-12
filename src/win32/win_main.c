#include <stdlib.h>
#include <stdio.h>
#include "sane_windows.h"
#include "../platform.h"
#include "../player.h"

#define WINVER_SIZE 16

static char *win_name;
static char *win_ver;

void platform_init() {
    SetConsoleTitleA("miniclassic");

    WORD versionReq = MAKEWORD(2, 2);
    WSADATA data;

    int err = WSAStartup(versionReq, &data);
    if (err != 0) {
        fprintf(stderr, "WSAStartup failed with %d\n", err);
        exit(0);
    }

    OSVERSIONINFOA ver;
    memset(&ver, 0, sizeof(ver));
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&ver);

    win_name = "Windows NT";
    win_ver = malloc(WINVER_SIZE);
    snprintf(win_ver, WINVER_SIZE, "%ld.%ld", ver.dwMajorVersion, ver.dwMinorVersion);
}

void platform_tick() {
    int online = playerman_get_num_online();

    const char *plural = (online == 1 ? "" : "s");
    char buf[128];
    snprintf(buf, sizeof(buf), "miniclassic - %d player%s online", online, plural);

    SetConsoleTitleA(buf);
}

void platform_shutdown() {
    WSACleanup();
}

const char *platform_get_name() {
    return win_name;
}

const char *platform_get_version() {
    return win_ver;
}