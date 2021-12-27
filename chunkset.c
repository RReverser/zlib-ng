/* chunkset.c -- inline functions to copy small data chunks.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "zbuild.h"
#include "zutil.h"

/* Define 8 byte chunks differently depending on unaligned support */
#if defined(UNALIGNED64_OK)
typedef uint64_t chunk_t;
#elif defined(UNALIGNED_OK)
typedef struct chunk_t { uint32_t u32[2]; } chunk_t;
#else
typedef struct chunk_t { uint8_t u8[8]; } chunk_t;
#endif

#define CHUNK_SIZE 8

#define HAVE_CHUNKMEMSET_1
#define HAVE_CHUNKMEMSET_4
#define HAVE_CHUNKMEMSET_8

static inline void chunkmemset_1(uint8_t *from, chunk_t *chunk) {
#if defined(UNALIGNED64_OK)
    *chunk = 0x0101010101010101 * (uint8_t)*from;
#elif defined(UNALIGNED_OK)
    chunk->u32[0] = 0x01010101 * (uint8_t)*from;
    chunk->u32[1] = chunk->u32[0];
#else
    memset(chunk, *from, sizeof(chunk_t));
#endif
}

static inline void chunkmemset_4(uint8_t *from, chunk_t *chunk) {
#if defined(UNALIGNED64_OK)
    uint32_t half_chunk;
    zmemcpy_4(&half_chunk, from);
    *chunk = 0x0000000100000001 * (uint64_t)half_chunk;
#elif defined(UNALIGNED_OK)
    zmemcpy_4(&chunk->u32[0], from);
    chunk->u32[1] = chunk->u32[0];
#else
    uint8_t *chunkptr = (uint8_t *)chunk;
    zmemcpy_4(chunkptr, from);
    zmemcpy_4(chunkptr+4, from);
#endif
}

static inline void chunkmemset_8(uint8_t *from, chunk_t *chunk) {
#if defined(UNALIGNED_OK) && !defined(UNALIGNED64_OK)
    zmemcpy_4(&chunk->u32[0], from);
    zmemcpy_4(&chunk->u32[1], from+4);
#else
    zmemcpy_8(chunk, from);
#endif
}

static inline void loadchunk(uint8_t const *s, chunk_t *chunk) {
    chunkmemset_8((uint8_t *)s, chunk);
}

static inline void storechunk(uint8_t *out, chunk_t *chunk) {
#if defined(UNALIGNED_OK) && !defined(UNALIGNED64_OK)
    zmemcpy_4(out, &chunk->u32[0]);
    zmemcpy_4(out+4, &chunk->u32[1]);
#else
    zmemcpy_8(out, chunk);
#endif
}

#define CHUNKSIZE        chunksize_c
#define CHUNKCOPY        chunkcopy_c
#define CHUNKCOPY_SAFE   chunkcopy_safe_c
#define CHUNKUNROLL      chunkunroll_c
#define CHUNKMEMSET      chunkmemset_c
#define CHUNKMEMSET_SAFE chunkmemset_safe_c

#include "chunkset_tpl.h"
