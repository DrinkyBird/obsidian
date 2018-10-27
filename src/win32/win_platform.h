#ifndef __WIN_PLATFORM_H__
#define __WIN_PLATFORM_H__

#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET socket_t;

#define SOCKERR_EAGAIN 0
#define SOCKERR_EWOULDBLOCK WSAEWOULDBLOCK

char *platform_strsep(char **stringp, const char *delim);

#endif