/* functable.h -- Struct containing function pointers to optimized functions
 * Copyright (C) 2017 Hans Kristian Rosbach
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#ifndef FUNCTABLE_H_
#define FUNCTABLE_H_

#include "deflate.h"
#include "crc32_fold.h"

#ifdef X86_FEATURES
#  include "fallback_builtins.h"
#endif

struct functable_s {
    uint32_t (* update_hash)        (deflate_state *const s, uint32_t h, uint32_t val);
    void     (* insert_string)      (deflate_state *const s, uint32_t str, uint32_t count);
    Pos      (* quick_insert_string)(deflate_state *const s, uint32_t str);
    uint32_t (* adler32)            (uint32_t adler, const unsigned char *buf, size_t len);
    uint32_t (* crc32)              (uint32_t crc, const unsigned char *buf, uint64_t len);
    uint32_t (* crc32_fold_reset)   (crc32_fold *crc);
    void     (* crc32_fold_copy)    (crc32_fold *crc, uint8_t *dst, const uint8_t *src, size_t len);
    uint32_t (* crc32_fold_final)   (crc32_fold *crc);
    void     (* slide_hash)         (deflate_state *s);
    uint32_t (* compare258)         (const unsigned char *src0, const unsigned char *src1);
    uint32_t (* longest_match)      (deflate_state *const s, Pos cur_match);
    uint32_t (* longest_match_slow) (deflate_state *const s, Pos cur_match);
    uint32_t (* chunksize)          (void);
    uint8_t* (* chunkcopy)          (uint8_t *out, uint8_t const *from, unsigned len);
    uint8_t* (* chunkcopy_safe)     (uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe);
    uint8_t* (* chunkunroll)        (uint8_t *out, unsigned *dist, unsigned *len);
    uint8_t* (* chunkmemset)        (uint8_t *out, unsigned dist, unsigned len);
    uint8_t* (* chunkmemset_safe)   (uint8_t *out, unsigned dist, unsigned len, unsigned left);
};

Z_INTERNAL extern Z_TLS struct functable_s functable;

#ifdef __cplusplus
extern "C" {
#endif

extern void cpu_check_features();

/* update_hash */
extern uint32_t update_hash_c(deflate_state *const s, uint32_t h, uint32_t val);
#ifdef X86_SSE42_CRC_HASH
extern uint32_t update_hash_sse4(deflate_state *const s, uint32_t h, uint32_t val);
#elif defined(ARM_ACLE_CRC_HASH)
extern uint32_t update_hash_acle(deflate_state *const s, uint32_t h, uint32_t val);
#endif

/* insert_string */
extern void insert_string_c(deflate_state *const s, const uint32_t str, uint32_t count);
#ifdef X86_SSE42_CRC_HASH
extern void insert_string_sse4(deflate_state *const s, const uint32_t str, uint32_t count);
#elif defined(ARM_ACLE_CRC_HASH)
extern void insert_string_acle(deflate_state *const s, const uint32_t str, uint32_t count);
#endif

/* quick_insert_string */
extern Pos quick_insert_string_c(deflate_state *const s, const uint32_t str);
#ifdef X86_SSE42_CRC_HASH
extern Pos quick_insert_string_sse4(deflate_state *const s, const uint32_t str);
#elif defined(ARM_ACLE_CRC_HASH)
extern Pos quick_insert_string_acle(deflate_state *const s, const uint32_t str);
#endif

/* slide_hash */
#ifdef X86_SSE2
void slide_hash_sse2(deflate_state *s);
#elif defined(ARM_NEON_SLIDEHASH)
void slide_hash_neon(deflate_state *s);
#endif
#if defined(PPC_VMX_SLIDEHASH)
void slide_hash_vmx(deflate_state *s);
#endif
#if defined(POWER8_VSX_SLIDEHASH)
void slide_hash_power8(deflate_state *s);
#endif
#ifdef X86_AVX2
void slide_hash_avx2(deflate_state *s);
#endif

/* adler32 */
extern uint32_t adler32_c(uint32_t adler, const unsigned char *buf, size_t len);
#ifdef ARM_NEON_ADLER32
extern uint32_t adler32_neon(uint32_t adler, const unsigned char *buf, size_t len);
#endif
#ifdef PPC_VMX_ADLER32
extern uint32_t adler32_vmx(uint32_t adler, const unsigned char *buf, size_t len);
#endif
#ifdef X86_SSE41_ADLER32
extern uint32_t adler32_sse41(uint32_t adler, const unsigned char *buf, size_t len);
#endif
#ifdef X86_SSSE3_ADLER32
extern uint32_t adler32_ssse3(uint32_t adler, const unsigned char *buf, size_t len);
#endif
#ifdef X86_AVX2_ADLER32
extern uint32_t adler32_avx2(uint32_t adler, const unsigned char *buf, size_t len);
#endif
#ifdef X86_AVX512_ADLER32
extern uint32_t adler32_avx512(uint32_t adler, const unsigned char *buf, size_t len);
#endif
#ifdef X86_AVX512VNNI_ADLER32
extern uint32_t adler32_avx512_vnni(uint32_t adler, const unsigned char *buf, size_t len);
#endif
#ifdef POWER8_VSX_ADLER32
extern uint32_t adler32_power8(uint32_t adler, const unsigned char* buf, size_t len);
#endif

/* CRC32 folding */
#ifdef X86_PCLMULQDQ_CRC
extern uint32_t crc32_fold_reset_pclmulqdq(crc32_fold *crc);
extern void     crc32_fold_copy_pclmulqdq(crc32_fold *crc, uint8_t *dst, const uint8_t *src, size_t len);
extern uint32_t crc32_fold_final_pclmulqdq(crc32_fold *crc);
#endif

/* memory chunking */
extern uint32_t chunksize_c(void);
extern uint8_t* chunkcopy_c(uint8_t *out, uint8_t const *from, unsigned len);
extern uint8_t* chunkcopy_safe_c(uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe);
extern uint8_t* chunkunroll_c(uint8_t *out, unsigned *dist, unsigned *len);
extern uint8_t* chunkmemset_c(uint8_t *out, unsigned dist, unsigned len);
extern uint8_t* chunkmemset_safe_c(uint8_t *out, unsigned dist, unsigned len, unsigned left);
#ifdef X86_SSE2_CHUNKSET
extern uint32_t chunksize_sse2(void);
extern uint8_t* chunkcopy_sse2(uint8_t *out, uint8_t const *from, unsigned len);
extern uint8_t* chunkcopy_safe_sse2(uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe);
extern uint8_t* chunkunroll_sse2(uint8_t *out, unsigned *dist, unsigned *len);
extern uint8_t* chunkmemset_sse2(uint8_t *out, unsigned dist, unsigned len);
extern uint8_t* chunkmemset_safe_sse2(uint8_t *out, unsigned dist, unsigned len, unsigned left);
#endif
#ifdef X86_AVX_CHUNKSET
extern uint32_t chunksize_avx(void);
extern uint8_t* chunkcopy_avx(uint8_t *out, uint8_t const *from, unsigned len);
extern uint8_t* chunkcopy_safe_avx(uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe);
extern uint8_t* chunkunroll_avx(uint8_t *out, unsigned *dist, unsigned *len);
extern uint8_t* chunkmemset_avx(uint8_t *out, unsigned dist, unsigned len);
extern uint8_t* chunkmemset_safe_avx(uint8_t *out, unsigned dist, unsigned len, unsigned left);
#endif
#ifdef ARM_NEON_CHUNKSET
extern uint32_t chunksize_neon(void);
extern uint8_t* chunkcopy_neon(uint8_t *out, uint8_t const *from, unsigned len);
extern uint8_t* chunkcopy_safe_neon(uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe);
extern uint8_t* chunkunroll_neon(uint8_t *out, unsigned *dist, unsigned *len);
extern uint8_t* chunkmemset_neon(uint8_t *out, unsigned dist, unsigned len);
extern uint8_t* chunkmemset_safe_neon(uint8_t *out, unsigned dist, unsigned len, unsigned left);
#endif
#ifdef POWER8_VSX_CHUNKSET
extern uint32_t chunksize_power8(void);
extern uint8_t* chunkcopy_power8(uint8_t *out, uint8_t const *from, unsigned len);
extern uint8_t* chunkcopy_safe_power8(uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe);
extern uint8_t* chunkunroll_power8(uint8_t *out, unsigned *dist, unsigned *len);
extern uint8_t* chunkmemset_power8(uint8_t *out, unsigned dist, unsigned len);
extern uint8_t* chunkmemset_safe_power8(uint8_t *out, unsigned dist, unsigned len, unsigned left);
#endif

/* CRC32 */
Z_INTERNAL uint32_t crc32_generic(uint32_t, const unsigned char *, uint64_t);

#if BYTE_ORDER == LITTLE_ENDIAN
extern uint32_t crc32_little(uint32_t crc, const unsigned char *buf, uint64_t len);
#elif BYTE_ORDER == BIG_ENDIAN
extern uint32_t crc32_big(uint32_t crc, const unsigned char *buf, uint64_t len);
#endif
#ifdef ARM_ACLE_CRC_HASH
extern uint32_t crc32_acle(uint32_t crc, const unsigned char *buf, uint64_t len);
#elif defined(POWER8_VSX_CRC32)
extern uint32_t crc32_power8(uint32_t crc, const unsigned char *buf, uint64_t len);
#elif defined(S390_CRC32_VX)
extern uint32_t s390_crc32_vx(uint32_t crc, const unsigned char *buf, uint64_t len);
#endif

/* compare258 */
extern uint32_t compare258_c(const unsigned char *src0, const unsigned char *src1);
#ifdef UNALIGNED_OK
extern uint32_t compare258_unaligned_16(const unsigned char *src0, const unsigned char *src1);
extern uint32_t compare258_unaligned_32(const unsigned char *src0, const unsigned char *src1);
#ifdef UNALIGNED64_OK
extern uint32_t compare258_unaligned_64(const unsigned char *src0, const unsigned char *src1);
#endif
#ifdef X86_SSE42_CMP_STR
extern uint32_t compare258_unaligned_sse4(const unsigned char *src0, const unsigned char *src1);
#endif
#if defined(X86_AVX2) && defined(HAVE_BUILTIN_CTZ)
extern uint32_t compare258_unaligned_avx2(const unsigned char *src0, const unsigned char *src1);
#endif
#endif

/* longest_match */
extern uint32_t longest_match_c(deflate_state *const s, Pos cur_match);
#ifdef UNALIGNED_OK
extern uint32_t longest_match_unaligned_16(deflate_state *const s, Pos cur_match);
extern uint32_t longest_match_unaligned_32(deflate_state *const s, Pos cur_match);
#ifdef UNALIGNED64_OK
extern uint32_t longest_match_unaligned_64(deflate_state *const s, Pos cur_match);
#endif
#ifdef X86_SSE42_CMP_STR
extern uint32_t longest_match_unaligned_sse4(deflate_state *const s, Pos cur_match);
#endif
#if defined(X86_AVX2) && defined(HAVE_BUILTIN_CTZ)
extern uint32_t longest_match_unaligned_avx2(deflate_state *const s, Pos cur_match);
#endif
#endif

/* longest_match_slow */
extern uint32_t longest_match_slow_c(deflate_state *const s, Pos cur_match);
#ifdef UNALIGNED_OK
extern uint32_t longest_match_slow_unaligned_16(deflate_state *const s, Pos cur_match);
extern uint32_t longest_match_slow_unaligned_32(deflate_state *const s, Pos cur_match);
#ifdef UNALIGNED64_OK
extern uint32_t longest_match_slow_unaligned_64(deflate_state *const s, Pos cur_match);
#endif
#ifdef X86_SSE42_CMP_STR
extern uint32_t longest_match_slow_unaligned_sse4(deflate_state *const s, Pos cur_match);
#endif
#if defined(X86_AVX2) && defined(HAVE_BUILTIN_CTZ)
extern uint32_t longest_match_slow_unaligned_avx2(deflate_state *const s, Pos cur_match);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
