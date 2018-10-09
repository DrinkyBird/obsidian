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

#endif // __BYTEORDER_H__