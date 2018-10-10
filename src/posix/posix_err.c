#include <stdio.h>
#include <errno.h>
#include "../platform.h"

int sock_error() {
    return errno;
}

void sock_perror(const char *s) {
    perror(s);
}