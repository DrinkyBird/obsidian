#ifndef __POSIX_PLATFORM_H__
#define __POSIX_PLATFORM_H__

#include <errno.h>

/* so we can use winsock and posix sockets together */
typedef int socket_t;
#define ioctlsocket ioctl

#define SOCKERR_EAGAIN EAGAIN
#define SOCKERR_EWOULDBLOCK EWOULDBLOCK

#define platform_strsep strsep

#endif