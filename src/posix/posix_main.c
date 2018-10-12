#include <sys/utsname.h>
#include "../platform.h"

static struct utsname uts;

void platform_init() {
    uname(&uts);
}

void platform_tick() {
    /* We don't need to do anything here */
}

void platform_shutdown() {
    /* We don't need to do anything here */
}

const char *platform_get_name() {
    return uts.sysname;
}

const char *platform_get_version() {
    return uts.release;
}