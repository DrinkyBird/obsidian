/*
    Obsidian, a lightweight Classicube server
    Copyright (C) 2018 Sean Baggaley.

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
#ifndef __WIN_PLATFORM_H__
#define __WIN_PLATFORM_H__

#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET socket_t;

#define SOCKERR_EAGAIN 0
#define SOCKERR_EWOULDBLOCK WSAEWOULDBLOCK
#define SOCKERR_ECONNRESET WSAECONNRESET
#define SOCKERR_EBADF WSAEBADF

char *platform_strsep(char **stringp, const char *delim);

#endif