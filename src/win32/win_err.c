#include <stdio.h>
#include "../platform.h"

int sock_error() {
    return WSAGetLastError();
}

void sock_perror(const char *s) {
    int err = sock_error();

    fprintf(stderr, "%s: %d\n", s, err);
}