#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "rw.h"
#include "nbt.h"

#define CLASSICWORLD_VERSION 1

static int attempt_inflate(byte *in, int in_size, byte *out, int out_size, int *buf_size);

void map_save(map_t *map) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s.cw", map->name);

    printf("Saving level to %s\n", buf);

    int num_blocks = (map->width * map->depth * map->height);

    byte *uuid = malloc(16);
    memset(uuid, 0, 16);

    tag_t *root = nbt_create_compound("ClassicWorld");
    tag_t *ver = nbt_create("FormatVersion"); nbt_set_char(ver, CLASSICWORLD_VERSION);
    tag_t *xtag = nbt_create("X"); nbt_set_short(xtag, map->width);
    tag_t *ytag = nbt_create("Y"); nbt_set_short(ytag, map->depth);
    tag_t *ztag = nbt_create("Z"); nbt_set_short(ztag, map->height);
    tag_t *blocktag = nbt_copy_bytearray("BlockArray", map->blocks, num_blocks);
    tag_t *uuidtag = nbt_copy_bytearray("UUID", uuid, 16);

    nbt_add_tag(root, xtag);
    nbt_add_tag(root, ytag);
    nbt_add_tag(root, ztag);
    nbt_add_tag(root, ver);
    nbt_add_tag(root, blocktag);
    nbt_add_tag(root, uuidtag);

    tag_t *spawntag = nbt_create_compound("Spawn");
    tag_t *spawnxtag = nbt_create("X"); nbt_set_short(spawnxtag, map->width / 2);
    tag_t *spawnytag = nbt_create("Y"); nbt_set_short(spawnytag, map->depth);
    tag_t *spawnztag = nbt_create("Z"); nbt_set_short(spawnztag, map->height / 2);
    nbt_add_tag(spawntag, spawnxtag);
    nbt_add_tag(spawntag, spawnytag);
    nbt_add_tag(spawntag, spawnztag);

    nbt_add_tag(root, spawntag);

    rw_t *rw = rw_create_empty(num_blocks + 1024);
    nbt_write(root, rw);

    int outsize = (rw_size(rw) * 1.1) + 12;
    byte *outbuf = malloc(outsize);

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = rw_size(rw);
    stream.next_in = rw->buf;
    stream.avail_out = outsize;
    stream.next_out = (Bytef*)outbuf;

    int err = deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, (MAX_WBITS | 16), 8, Z_DEFAULT_STRATEGY);
    if (err != Z_OK) {
        fprintf(stderr, "zlib error.");
    }

    while ((err = deflate(&stream, Z_FINISH)) != Z_STREAM_END);
    deflateEnd(&stream);

    FILE *f = fopen(buf, "wb");
    if (f == NULL) {
        fprintf(stderr, "Failed to open file for writing, map will not be saved: %s\n", buf);
        return;
    }

    fwrite(outbuf, outsize, 1, f);
    fclose(f);

    rw_destroy_and_buffer(rw);
}

map_t *map_load(const char *name) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s.cw", name);

    printf("Loading file %s\n", buf);

    FILE *f = fopen(buf, "rb");
    if (f == NULL) {
        printf("Failed to open %s - generating new world\n", buf);
        return NULL;
    }

    /* determine file length */
    fseek(f, 0, SEEK_END);
    int flen = ftell(f);
    fseek(f, 0, SEEK_SET);

    byte *inf = malloc(flen);

    byte *in = NULL;
    int insize;

    fread(inf, flen, 1, f);
    fclose(f);

    /* probably gzip'd */
    if (inf[0] == 0x1F) {
        int buf_size = 8192;
        int res_size;
        int r;

        byte *defbuf = malloc(buf_size);

        while ((r = attempt_inflate(inf, flen, defbuf, buf_size, &res_size)) != Z_STREAM_END) {
            if (r == Z_BUF_ERROR) {
                buf_size += 512 * 1024;
                free(defbuf);
                defbuf = malloc(buf_size);
            }
        }

        in = defbuf;
        insize = res_size;
    } else {
        in = inf;
        insize = flen;
    }
    
    f = fopen("test.nbt", "wb");
    fwrite(in, insize, 1, f);
    fclose(f);

    rw_t *rw = rw_create(in, insize);
    tag_t *root = nbt_read(rw, true);

    if (root->type != tag_compound) {
        fprintf(stderr, "%s invalid: root tag is not TAG_Compound\n", buf);
        return NULL;
    } else if (strcmp(root->name, "ClassicWorld") != 0) {
        fprintf(stderr, "%s invalid: root tag is not named ClassicWorld\n", buf);
        return NULL;
    }

    tag_t *versiontag = nbt_get_tag(root, "FormatVersion");
    if (versiontag->b > CLASSICWORLD_VERSION) {
        fprintf(stderr, "%s invalid: classicworld format version (%d) is newer than supported (%d)\n", buf, versiontag->b, CLASSICWORLD_VERSION);
        return NULL;
    }

    tag_t *xtag = nbt_get_tag(root, "X");
    tag_t *ytag = nbt_get_tag(root, "Y");
    tag_t *ztag = nbt_get_tag(root, "Z");
    tag_t *blockstag = nbt_get_tag(root, "BlockArray");

    int w = xtag->i;
    int d = ytag->i;
    int h = ztag->i;
    byte *blocks = blockstag->pb;

    map_t *map = map_create(name, w, d, h);
    memcpy(map->blocks, blocks, (w * d * h));

    nbt_destroy(root, true);
    rw_destroy_and_buffer(rw);

    return map;
}

int attempt_inflate(byte *in, int in_size, byte *out, int out_size, int *buf_size) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = (uInt)in_size;
    stream.next_in = (Bytef *)in;
    stream.avail_out = (uInt)out_size;
    stream.next_out = (Bytef *)out;

    int r;
    if ((r = inflateInit2(&stream, (MAX_WBITS | 16))) != Z_OK) {
        return r;
    }

    if ((r = inflate(&stream, Z_FINISH)) != Z_STREAM_END) {
        return r;
    }

    inflateEnd(&stream);

    *buf_size = stream.avail_in;

    return r;
}