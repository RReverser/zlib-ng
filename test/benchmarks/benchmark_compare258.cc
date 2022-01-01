/* compare258_benchmark.cc -- test compare258 variants
 * Copyright (C) 2020 Nathan Moinvaziri
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#include <benchmark/benchmark.h>

#include "zbuild.h"
#include "zutil.h"

#include "functable.h"

typedef uint32_t (*compare258_func)(const unsigned char *src0, const unsigned char *src1);

static inline void compare258_bench(benchmark::State& state, compare258_func compare258) {
    unsigned char str1[274];
    unsigned char str2[274];
    uint32_t len;
    int32_t x = 0;

    memset(str1, 'a', sizeof(str1));
    memset(str2, 'a', sizeof(str2));

    while (state.KeepRunning()) {
        str2[x] = 0;
        len = compare258((const unsigned char *)str1, (const unsigned char *)str2);
        str2[x] = 'a';
        if (++x >= 274) {
            x = 0;
        }
    }

    benchmark::DoNotOptimize(len);
}

static void compare258_c_bench(benchmark::State& state) {
    compare258_bench(state, compare258_c);
}
BENCHMARK(compare258_c_bench);
#ifdef UNALIGNED_OK
static void compare258_unaligned_16_bench(benchmark::State& state) {
    compare258_bench(state, compare258_unaligned_16);
}
BENCHMARK(compare258_unaligned_16_bench);
#ifdef HAVE_BUILTIN_CTZ
static void compare258_unaligned_32_bench(benchmark::State& state) {
    compare258_bench(state, compare258_unaligned_32);
}
BENCHMARK(compare258_unaligned_32_bench);
#endif
#if defined(UNALIGNED64_OK) && defined(HAVE_BUILTIN_CTZLL)
static void compare258_unaligned_64_bench(benchmark::State& state) {
    compare258_bench(state, compare258_unaligned_64);
}
BENCHMARK(compare258_unaligned_64_bench);
#endif
#ifdef X86_SSE42_CMP_STR
static void compare258_unaligned_sse4_bench(benchmark::State& state) {
    compare258_bench(state, compare258_unaligned_sse4);
}
BENCHMARK(compare258_unaligned_sse4_bench);
#endif
#if defined(X86_AVX2) && defined(HAVE_BUILTIN_CTZ)
static void compare258_unaligned_avx2_bench(benchmark::State& state) {
    compare258_bench(state, compare258_unaligned_avx2);
}
BENCHMARK(compare258_unaligned_avx2_bench);
#endif
#endif
