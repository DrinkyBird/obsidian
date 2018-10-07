#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL_endian.h>
#include "rw.h"

static void rw_clamp_offset(rw_t *rw);

rw_t *rw_create(void *buf, int len) {
    rw_t *rw = malloc(sizeof(rw_t));
    rw->buf = buf;
    rw->len = len;

    rw->offset = 0;
    rw->read_only = false;

    return rw;
}

rw_t *rw_create_ro(void *buf, int len) {
    rw_t *rw = rw_create_ro(buf, len);
    rw->read_only = true;

    return rw;
}

rw_t *rw_create_empty(int len) {
    byte *b = malloc(len);
    rw_t *rw = rw_create(b, len);

    return rw;
}

void rw_destroy(rw_t *rw) {
    free(rw);
}

void rw_destroy_and_buffer(rw_t *rw) {
    free(rw->buf);
    rw_destroy(rw);
}

int rw_tell(rw_t *rw) {
    return rw->offset;
}

int rw_size(rw_t *rw) {
    return rw->len;
}

int rw_seek(rw_t *rw, int pos, int from) {
    if (from == rw_cur) {
        rw->offset += pos;
    } else if (from == rw_set) {
        rw->offset = pos;
    } else if (from == rw_end) {
        rw->offset = rw->len - pos;
    }

    rw_clamp_offset(rw);

    return rw->offset;
}

void rw_clamp_offset(rw_t *rw) {
    if (rw->offset < 0) {
        rw->offset = 0;
    } else if (rw->offset > rw->len) {
        rw->offset = rw->len;
    }
}

int rw_read(rw_t *rw, void *buf, int len) {
    int read = MIN(len, rw->len - rw->offset);
    memcpy(buf, rw->buf + rw->offset, read);
    rw_seek(rw, read, rw_cur);

    // printf("DBG: want %d bytes, got %d, currently at %d, now at %d\n", len, read, rw->offset - read, rw->offset);

    return read;
}

int rw_write(rw_t *rw, void *buf, int len) {
    int written = MIN(len, rw->len - rw->offset);
    memcpy(rw->buf + rw->offset, buf, written);
    rw_seek(rw, written, rw_cur);
    return written;
}

int rw_read_mc(rw_t *rw, void *buf, int len) {
    byte *data = malloc(1024);
    int r = rw_read(rw, data, 1024);
    int w = MIN(r, len);
    memcpy(buf, data, w);
    return w;
}

const char *rw_read_mc_str(rw_t *rw) {
    char s[64];
    rw_read(rw, s, 64);
    printf("newoff=%d\n", rw->offset);

    // trim the string
    int start = 0;
    int end = 63;

    for (int i = 0; i < 64; i++) {
        if (s[i] != 0x20) {
            break;
        }

        start++;
    }

    for (int i = 63; i > 0; i--) {
        if (s[i] != 0x20) {
            break;
        }

        end--;
    }

    /* + 1 for null terminator */
    int newlen = (end - start) + 1;

    char *newstr = malloc(newlen);
    memcpy(newstr, s + start, newlen);

    return newstr;
}

int rw_write_mc_str(rw_t *rw, const char *str) {
    int slen = strlen(str);
    char s[64];
    for (int i = 0; i < 64; i++) {
        s[i] = 0x20;
    }
    memcpy(s, str, MIN(64, slen));
    rw_write(rw, s, 64);
    printf("OFF=%d\n", rw->offset);
    return 64;
}

/******************************************************************************/
#define CHECK_SIZE() \
if (rw->offset + sizeof(c) > rw->len) { \
    fprintf(stderr, "RW ERROR! %d + %lu >= %d\n", rw->offset, sizeof(c), rw->len); \
    abort(); \
}

char rw_read_char(rw_t *rw) {
    char c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return c;
}

byte rw_read_byte(rw_t *rw) {
    byte c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return c;
}

short rw_read_int16le(rw_t *rw) {
    short c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return SDL_SwapLE16(c);
}

short rw_read_int16be(rw_t *rw) {
    short c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return SDL_SwapBE16(c);
}

unsigned short rw_read_uint16le(rw_t *rw) {
    unsigned short c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return SDL_SwapLE16(c);
}

unsigned short rw_read_uint16be(rw_t *rw) {
    unsigned short c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return SDL_SwapBE16(c);
}

int rw_read_int32le(rw_t *rw) {
    int c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return SDL_SwapLE32(c);
}

int rw_read_int32be(rw_t *rw) {
    int c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return SDL_SwapBE32(c);
}

unsigned int rw_read_uint32le(rw_t *rw) {
    unsigned int c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return SDL_SwapLE32(c);
}

unsigned int rw_read_uint32be(rw_t *rw) {
    unsigned int c; CHECK_SIZE();
    rw_read(rw, &c, sizeof(c));
    return SDL_SwapBE32(c);
}


int rw_write_char(rw_t *rw, char c) {
    return rw_write(rw, &c, sizeof(c));
}

int rw_write_byte(rw_t *rw, byte c) {
    return rw_write(rw, &c, sizeof(c));
}

int rw_write_int16le(rw_t *rw, short c) {
    short a = SDL_SwapLE16(c);
    return rw_write(rw, &a, sizeof(a));
}

int rw_write_int16be(rw_t *rw, short c) {
    short a = SDL_SwapBE16(c);
    return rw_write(rw, &a, sizeof(a));
}

int rw_write_uint16le(rw_t *rw, unsigned short c) {
    unsigned short a = SDL_SwapLE16(c);
    return rw_write(rw, &a, sizeof(a));
}

int rw_write_uint16be(rw_t *rw, unsigned short c) {
    unsigned short a = SDL_SwapBE16(c);
    return rw_write(rw, &a, sizeof(a));
}

int rw_write_int32le(rw_t *rw, int c) {
    short a = SDL_SwapLE32(c);
    return rw_write(rw, &a, sizeof(a));
}

int rw_write_int32be(rw_t *rw, int c) {
    short a = SDL_SwapBE32(c);
    return rw_write(rw, &a, sizeof(a));
}

int rw_write_uint32le(rw_t *rw, unsigned int c) {
    unsigned short a = SDL_SwapLE32(c);
    return rw_write(rw, &a, sizeof(a));
}

int rw_write_uint32be(rw_t *rw, unsigned int c) {
    unsigned short a = SDL_SwapBE32(c);
    return rw_write(rw, &a, sizeof(a));
}