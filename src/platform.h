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
void platform_tick();
void platform_shutdown();

const char *platform_get_name();
const char *platform_get_version();

#endif