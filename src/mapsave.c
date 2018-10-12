#include <zlib.h>
#include <stdio.h>
#include "map.h"
#include "rw.h"
#include "nbt.h"

void map_save(map_t *map) {
    char buf[128];
    snprintf(buf, sizeof(buf), "%s.cw", map->name);

    printf("Saving level to %s\n", buf);

    int num_blocks = (map->width * map->depth * map->height);

    byte *uuid = malloc(16);
    memset(uuid, 0, 16);

    tag_t *root = nbt_create_compound("ClassicWorld");
    tag_t *ver = nbt_create("FormatVersion"); nbt_set_char(ver, 1);
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

    int err = deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, (15 | 16), 8, Z_DEFAULT_STRATEGY);
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