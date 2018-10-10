#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef _WIN32
#include "win32/win_platform.h"
#elif defined(__unix__)
#include "posix/posix_platform.h"
#endif

int sock_error();
void sock_perror(const char *s);

void platform_init();
void platform_shutdown();

#endif