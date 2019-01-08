/*
    Obsidian, a lightweight Classicube server
    Copyright (C) 2018-2019 Sean Baggaley.

    Obsidian is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Obsidian is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with Obsidian.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef __POSIX_PLATFORM_H__
#define __POSIX_PLATFORM_H__

#include <errno.h>

/* so we can use winsock and posix sockets together */
typedef int socket_t;
#define ioctlsocket ioctl

#define SOCKERR_EAGAIN EAGAIN
#define SOCKERR_EWOULDBLOCK EWOULDBLOCK
#define SOCKERR_ECONNRESET ECONNRESET
#define SOCKERR_EBADF EBADF

#define platform_strsep strsep

#endif