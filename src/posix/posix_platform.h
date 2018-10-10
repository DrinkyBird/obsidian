#ifndef __POSIX_PLATFORM_H__
#define __POSIX_PLATFORM_H__

/* so we can use winsock and posix sockets together */
typedef int socket_t;
#define ioctlsocket ioctl

#endif