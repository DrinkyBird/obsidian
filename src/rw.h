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
/* common memory read/write functions */
#ifndef __RW_H__
#define __RW_H__

#include <stdbool.h>
#include "defs.h"

enum {
    rw_set,
    rw_cur,
    rw_end
};

typedef struct rw_s {
    void *buf;
    int len;

    int offset;
    bool read_only;
} rw_t;

rw_t *rw_create(void *buf, int len);
rw_t *rw_create_empty(int len);
rw_t *rw_create_ro(void *buf, int len);
void rw_destroy(rw_t *rw);
void rw_destroy_and_buffer(rw_t *rw);

int rw_size(rw_t *rw);
int rw_tell(rw_t *rw);
int rw_seek(rw_t *rw, int pos, int from);

/***************************** reading functions *****************************/
int rw_read(rw_t *rw, void *buf, int len);
int rw_read_mc(rw_t *rw, void *buf, int len);

char rw_read_char(rw_t *rw);
byte rw_read_byte(rw_t *rw);
short rw_read_int16le(rw_t *rw);
short rw_read_int16be(rw_t *rw);
unsigned short rw_read_uint16le(rw_t *rw);
unsigned short rw_read_uint16be(rw_t *rw);
int rw_read_int32le(rw_t *rw);
int rw_read_int32be(rw_t *rw);
unsigned int rw_read_uint32le(rw_t *rw);
unsigned int rw_read_uint32be(rw_t *rw);
long long rw_read_int64le(rw_t *rw);
long long rw_read_int64be(rw_t *rw);
unsigned long long rw_read_uint64le(rw_t *rw);
unsigned long long rw_read_uint64be(rw_t *rw);

float rw_read_floatle(rw_t *rw);
float rw_read_floatbe(rw_t *rw);
double rw_read_doublele(rw_t *rw);
double rw_read_doublebe(rw_t *rw);

const char *rw_read_mc_str(rw_t *rw);

/***************************** writing functions *****************************/
int rw_write(rw_t *rw, void *buf, int len);
int rw_write_char(rw_t *rw, char c);
int rw_write_byte(rw_t *rw, byte c);
int rw_write_int16le(rw_t *rw, short c);
int rw_write_int16be(rw_t *rw, short c);
int rw_write_uint16le(rw_t *rw, unsigned short c);
int rw_write_uint16be(rw_t *rw, unsigned short c);
int rw_write_int32le(rw_t *rw, int c);
int rw_write_int32be(rw_t *rw, int c);
int rw_write_uint32le(rw_t *rw, unsigned int c);
int rw_write_uint32be(rw_t *rw, unsigned int c);
int rw_write_int64le(rw_t *rw, long long c);
int rw_write_int64be(rw_t *rw, long long c);
int rw_write_uint64le(rw_t *rw, unsigned long long c);
int rw_write_uint64be(rw_t *rw, unsigned long long c);

int rw_write_floatle(rw_t *rw, float c);
int rw_write_floatbe(rw_t *rw, float c);
int rw_write_doublele(rw_t *rw, double c);
int rw_write_doublebe(rw_t *rw, double c);

int rw_write_mc_str(rw_t *rw, const char *str);

#endif