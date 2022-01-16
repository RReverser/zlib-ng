/* crc32.c -- compute the CRC-32 of a data stream
 * Copyright (C) 1995-2006, 2010, 2011, 2012, 2016, 2018 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * Thanks to Rodney Brown <rbrown64@csc.com.au> for his contribution of faster
 * CRC methods: exclusive-oring 32 bits of data at a time, and pre-computing
 * tables for updating the shift register in one step with three exclusive-ors
 * instead of four steps with four exclusive-ors.  This results in about a
 * factor of two increase in speed on a Power PC G4 (PPC7455) using gcc -O3.
 */

#include "zbuild.h"
#include "zendian.h"
#include <stdint.h>
#include "deflate.h"
#include "functable.h"
#include "crc32_tbl.h"

/* ========================================================================= */
const uint32_t * Z_EXPORT PREFIX(get_crc_table)(void) {
    return (const uint32_t *)crc_table;
}

#ifdef ZLIB_COMPAT
unsigned long Z_EXPORT PREFIX(crc32_z)(unsigned long crc, const unsigned char *buf, size_t len) {
    if (buf == NULL) return 0;

    return (unsigned long)functable.crc32((uint32_t)crc, buf, len);
}
#else
uint32_t Z_EXPORT PREFIX(crc32_z)(uint32_t crc, const unsigned char *buf, size_t len) {
    if (buf == NULL) return 0;

    return functable.crc32(crc, buf, len);
}
#endif

#ifdef ZLIB_COMPAT
unsigned long Z_EXPORT PREFIX(crc32)(unsigned long crc, const unsigned char *buf, unsigned int len) {
    return (unsigned long)PREFIX(crc32_z)((uint32_t)crc, buf, len);
}
#else
uint32_t Z_EXPORT PREFIX(crc32)(uint32_t crc, const unsigned char *buf, uint32_t len) {
    return PREFIX(crc32_z)(crc, buf, len);
}
#endif

/* ========================================================================= */

/*
   This BYEIGHT code accesses the passed unsigned char * buffer with a 64-bit
   integer pointer type. This violates the strict aliasing rule, where a
   compiler can assume, for optimization purposes, that two pointers to
   fundamentally different types won't ever point to the same memory. This can
   manifest as a problem only if one of the pointers is written to. This code
   only reads from those pointers. So long as this code remains isolated in
   this compilation unit, there won't be a problem. For this reason, this code
   should not be copied and pasted into a compilation unit in which other code
   writes to the buffer that is passed to these routines.
 */

/* ========================================================================= */

#if BYTE_ORDER == LITTLE_ENDIAN
#define DOSWAP(crc) (crc)
#define DO1(b) \
    c = crc_table[0][(c ^ b) & 0xff] ^ (c >> 8)
#define DO8(b) c ^= b; \
    c = crc_table[7][c & 0xFF] ^ \
        crc_table[6][((c >> 8) & 0xFF)] ^ \
        crc_table[5][((c >> 16) & 0xFF)] ^ \
        crc_table[4][((c >> 24) & 0xFF)] ^ \
        crc_table[3][((c >> 32) & 0xFF)] ^ \
        crc_table[2][((c >> 40) & 0xFF)] ^ \
        crc_table[1][((c >> 48) & 0xFF)] ^ \
        crc_table[0][c >> 56]
#elif BYTE_ORDER == BIG_ENDIAN
#define DOSWAP(crc) ZSWAP32(crc)
#define DO1(b) \
    c = crc_table[4][(c >> 24) ^ b] ^ (c << 8)
#define DO8(b) c ^= b; \
    c = crc_table[7][c >> 56] ^ \
        crc_table[6][((c >> 48) & 0xFF)] ^ \
        crc_table[5][((c >> 40) & 0xFF)] ^ \
        crc_table[4][((c >> 32) & 0xFF)] ^ \
        crc_table[3][((c >> 24) & 0xFF)] ^ \
        crc_table[2][((c >> 16) & 0xFF)] ^ \
        crc_table[1][((c >> 8) & 0xFF)] ^ \
        crc_table[0][c & 0xFF]
#else
#  error "No endian defined"
#endif

Z_INTERNAL uint32_t crc32_byeight(uint32_t crc, const unsigned char *buf, uint64_t len) {
    Z_REGISTER uint64_t c;
    Z_REGISTER const uint64_t *buf8;

    c = DOSWAP(crc);
    c = ~c;
    while (len && ((ptrdiff_t)buf & 7)) {
        DO1(*buf++);
        len--;
    }

    buf8 = (const uint64_t *)(const void *)buf;

    while (len >= 8) {
        uint64_t b = *buf8++;
        DO8(b);
        len -= 8;
    }

    buf = (const unsigned char *)buf8;

    if (len) do {
        DO1(*buf++);
    } while (--len);

    c = ~c;
    return (uint32_t)DOSWAP(c);
}
