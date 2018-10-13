#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

#include <stdint.h>

#if defined(__hppa__) || \
defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
(defined(__MIPS__) && defined(__MISPEB__)) || \
defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
defined(__sparc__)
#define ENDIAN_BIG 1
#else
#define ENDIAN_LITTLE 1
#endif

static inline uint16_t endian_swap16(uint16_t x) {
    return (uint16_t)((x << 8) | (x >> 8));
}

static inline uint32_t endian_swap32(uint32_t x) {
    return (uint32_t)((x << 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x >> 24));
}

static inline uint64_t endian_swap64(uint64_t x) {
    uint32_t hi, lo;

    lo = (uint32_t)(x & 0xFFFFFFFF);
    x >>= 32;
    hi = (uint32_t)(x & 0xFFFFFFFF);
    
    x = (uint32_t) endian_swap32(lo);
    x <<= 32;
    x |= (uint32_t) endian_swap32(hi);
    
    return x;
}

static inline double endian_swapd(double x) {
    double r;
    char *ftc = (char*)&x;
    char *rf = (char*)&r;

    rf[0] = ftc[7];
    rf[1] = ftc[6];
    rf[2] = ftc[5];
    rf[3] = ftc[4];
    rf[4] = ftc[3];
    rf[5] = ftc[2];
    rf[6] = ftc[1];
    rf[7] = ftc[0];

    return r;
}

static inline float endian_swapf(float x) {
    float r;
    char *ftc = (char*)&x;
    char *rf = (char*)&r;

    rf[0] = ftc[3];
    rf[1] = ftc[2];
    rf[2] = ftc[1];
    rf[0] = ftc[0];

    return r;
}

static inline uint16_t endian_tolittle16(uint16_t x) {
#ifdef ENDIAN_BIG
    return endian_swap16(x);
#else
    return x;
#endif
}

static inline uint32_t endian_tolittle32(uint32_t x){
#ifdef ENDIAN_BIG
    return endian_swap32(x);
#else
    return x;
#endif
}

static inline uint64_t endian_tolittle64(uint64_t x){
#ifdef ENDIAN_BIG
    return endian_swap64(x);
#else
    return x;
#endif
}

static inline float endian_tolittlef(float x){
#ifdef ENDIAN_BIG
    return endian_swapf(x);
#else
    return x;
#endif
}

static inline double endian_tolittled(double x){
#ifdef ENDIAN_BIG
    return endian_swapd(x);
#else
    return x;
#endif
}

static inline uint16_t endian_tobig16(uint16_t x) {
#ifdef ENDIAN_LITTLE
    return endian_swap16(x);
#else
    return x;
#endif
}

static inline uint32_t endian_tobig32(uint32_t x) {
#ifdef ENDIAN_LITTLE
    return endian_swap32(x);
#else
    return x;
#endif
}

static inline uint64_t endian_tobig64(uint64_t x) {
#ifdef ENDIAN_LITTLE
    return endian_swap64(x);
#else
    return x;
#endif
}

static inline float endian_tobigf(float x) {
#ifdef ENDIAN_LITTLE
    return endian_swapf(x);
#else
    return x;
#endif
}

static inline double endian_tobigd(double x) {
#ifdef ENDIAN_LITTLE
    return endian_swapd(x);
#else
    return x;
#endif
}



#endif // __BYTEORDER_H__