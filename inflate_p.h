/* inflate_p.h -- Private inline functions and macros shared with more than one deflate method
 *
 */

#ifndef INFLATE_P_H
#define INFLATE_P_H

/* Architecture-specific hooks. */
#ifdef S390_DFLTCC_INFLATE
#  include "arch/s390/dfltcc_inflate.h"
#else
/* Memory management for the inflate state. Useful for allocating arch-specific extension blocks. */
#  define ZALLOC_STATE(strm, items, size) ZALLOC(strm, items, size)
#  define ZFREE_STATE(strm, addr) ZFREE(strm, addr)
#  define ZCOPY_STATE(dst, src, size) memcpy(dst, src, size)
/* Memory management for the window. Useful for allocation the aligned window. */
#  define ZALLOC_WINDOW(strm, items, size) ZALLOC(strm, items, size)
#  define ZFREE_WINDOW(strm, addr) ZFREE(strm, addr)
/* Invoked at the end of inflateResetKeep(). Useful for initializing arch-specific extension blocks. */
#  define INFLATE_RESET_KEEP_HOOK(strm) do {} while (0)
/* Invoked at the beginning of inflatePrime(). Useful for updating arch-specific buffers. */
#  define INFLATE_PRIME_HOOK(strm, bits, value) do {} while (0)
/* Invoked at the beginning of each block. Useful for plugging arch-specific inflation code. */
#  define INFLATE_TYPEDO_HOOK(strm, flush) do {} while (0)
/* Returns whether zlib-ng should compute a checksum. Set to 0 if arch-specific inflation code already does that. */
#  define INFLATE_NEED_CHECKSUM(strm) 1
/* Returns whether zlib-ng should update a window. Set to 0 if arch-specific inflation code already does that. */
#  define INFLATE_NEED_UPDATEWINDOW(strm) 1
/* Returns whether zlib-ng should flush the window to the output buffer.
   Set to 0 if arch-specific inflation code already does that. */
#  define INFLATE_NEED_WINDOW_OUTPUT_FLUSH(strm) 1
/* Invoked at the beginning of inflateMark(). Useful for updating arch-specific pointers and offsets. */
#  define INFLATE_MARK_HOOK(strm) do {} while (0)
/* Invoked at the beginning of inflateSyncPoint(). Useful for performing arch-specific state checks. */
#  define INFLATE_SYNC_POINT_HOOK(strm) do {} while (0)
/* Invoked at the beginning of inflateSetDictionary(). Useful for checking arch-specific window data. */
#  define INFLATE_SET_DICTIONARY_HOOK(strm, dict, dict_len) do {} while (0)
/* Invoked at the beginning of inflateGetDictionary(). Useful for adjusting arch-specific window data. */
#  define INFLATE_GET_DICTIONARY_HOOK(strm, dict, dict_len) do {} while (0)
#endif

/*
 *   Macros shared by inflate() and inflateBack()
 */

/* check function to use adler32() for zlib or crc32() for gzip */
#ifdef GUNZIP
#  define UPDATE(check, buf, len) \
    (state->flags ? PREFIX(crc32)(check, buf, len) : functable.adler32(check, buf, len))
#else
#  define UPDATE(check, buf, len) functable.adler32(check, buf, len)
#endif

/* check macros for header crc */
#ifdef GUNZIP
#  define CRC2(check, word) \
    do { \
        hbuf[0] = (unsigned char)(word); \
        hbuf[1] = (unsigned char)((word) >> 8); \
        check = PREFIX(crc32)(check, hbuf, 2); \
    } while (0)

#  define CRC4(check, word) \
    do { \
        hbuf[0] = (unsigned char)(word); \
        hbuf[1] = (unsigned char)((word) >> 8); \
        hbuf[2] = (unsigned char)((word) >> 16); \
        hbuf[3] = (unsigned char)((word) >> 24); \
        check = PREFIX(crc32)(check, hbuf, 4); \
    } while (0)
#endif

/* Load registers with state in inflate() for speed */
#define LOAD() \
    do { \
        put = strm->next_out; \
        left = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } while (0)

/* Restore state from registers in inflate() */
#define RESTORE() \
    do { \
        strm->next_out = put; \
        strm->avail_out = left; \
        strm->next_in = (z_const unsigned char *)next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } while (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    do { \
        hold = 0; \
        bits = 0; \
    } while (0)

/* Ensure that there is at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate()/inflateBack(). */
#define NEEDBITS(n) \
    do { \
        while (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } while (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    (hold & ((1U << (unsigned)(n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } while (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    do { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } while (0)

#endif

/* Set mode=BAD and prepare error message */
#define SET_BAD(errmsg) \
    do { \
        state->mode = BAD; \
        strm->msg = (char *)errmsg; \
    } while (0)
