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
#ifndef __CPE_H__
#define __CPE_H__

typedef struct cpeext_s {
    const char *name;
    int version;
} cpeext_t;

cpeext_t *cpe_get_supported_exts(int *size);

#endif // __CPE_H__