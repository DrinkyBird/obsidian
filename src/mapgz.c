#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <zlib.h>
#include "defs.h"
#include "connection.h"

void *mapgz_run(void *data) {
    mapgz_data_t *mgz = (mapgz_data_t *)data;
    connection_t *conn = mgz->conn;

    int outsize = ((mgz->num_blocks+4) * 1.1) + 12;
    byte *outbuf = malloc(outsize);
    
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = mgz->num_blocks + 4;
    stream.next_in = (Bytef*)mgz->blocks;
    stream.avail_out = outsize;
    stream.next_out = (Bytef*)outbuf;

    int err = deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, (15 | 16), 8, Z_DEFAULT_STRATEGY);
    if (err != Z_OK) {
        fprintf(stderr, "zlib error.");
    }

    while ((err = deflate(&stream, Z_FINISH)) != Z_STREAM_END);
    deflateEnd(&stream);

    outsize = stream.total_out;

    conn->mapgz_data = outbuf;
    conn->mapgz_size = outsize;
    conn->mapgz_rw = rw_create(conn->mapgz_data, conn->mapgz_size);
    rw_seek(conn->mapgz_rw, 0, rw_set);

    conn->thread_successful = true;
    conn->thread_running = false;

    free(mgz->blocks);
    free(data);

    pthread_exit(0);
}