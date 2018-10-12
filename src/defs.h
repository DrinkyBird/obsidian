#ifndef __DEFS_H__
#define __DEFS_H__

/* how many times per second the game updates */
#define TICKRATE 20

#define HEARTBEAT_RATE (45 * TICKRATE)

#define PROTOCOL_VERSION 7

typedef unsigned char byte;

enum {
    PACKET_IDENT = 0x00,
    PACKET_PING = 0x01,
    PACKET_LEVEL_INIT = 0x02,
    PACKET_LEVEL_CHUNK = 0x03,
    PACKET_LEVEL_FINISH = 0x04,
    PACKET_SET_BLOCK_CLIENT = 0x05,
    PACKET_SET_BLOCK_SERVER = 0x06,
    PACKET_PLAYER_SPAWN = 0x07,
    PACKET_PLAYER_POS_AND_ANGLE = 0x08,
    PACKET_PLAYER_POS_AND_ANGLE_UPDATE = 0x09,
    PACKET_PLAYER_POS_UPDATE = 0x0a,
    PACKET_PLAYER_ANGLE_UPDATE = 0x0b,
    PACKET_PLAYER_DESPAWN = 0x0c,
    PACKET_MESSAGE = 0x0d,
    PACKET_PLAYER_DISCONNECT = 0x0e,
    PACKET_PLAYER_SET_TYPE = 0x0f,

    PACKET_EXTINFO = 0x10,
    PACKET_EXTENTRY = 0x11
};

typedef enum block_e {
    air,
    stone,
    grass,
    dirt,
    cobblestone,
    wood_planks,
    sapling,
    bedrock,
    water,
    water_still,
    lava,
    lava_still,
    sand,
    gravel,
    gold_ore,
    iron_ore,
    coal_ore,
    wood,
    leaves,
    sponge,
    glass,
    red_wool,
    orange_wool,
    yellow_wool,
    lime_wool,
    green_wool,
    aquagreen_wool,
    cyan_wool,
    blue_wool,
    purple_wool,
    indigo_wool,
    violet_wool,
    magenta_wool,
    pink_wool,
    black_wool,
    grey_wool,
    white_wool,
    dandelion,
    rose,
    brown_mushroom,
    red_mushroom,
    gold_block,
    iron_block,
    double_slab,
    slab,
    bricks,
    tnt,
    bookshelf,
    mossy_cobblestoe,
    obsidian
} block_e;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define TOFLOAT(x) (float)(x / 32.0f)
#define TOFIXED(x) (short)(x * 32.0f)

#define FLOATANGLE(x) (float)(x * 360.0f / 256.0f)
#define FIXEDANGLE(x) (byte)(x * 256.0f / 360.0f)

int rrand(int min, int max);

const char *app_get_full_name();

#endif