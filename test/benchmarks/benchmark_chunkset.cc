/* benchmark_chunkset.cc -- benchmark chunkset variants
 * Copyright (C) 2022 Nathan Moinvaziri
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include <stdint.h>
#include <stdint.h>
#include <limits.h>

extern "C" {
#  include "zbuild.h"
#  include "zutil.h"
#  include "zutil_p.h"
#  include "cpu_features.h"
}

#include <benchmark/benchmark.h>

#define MAX_RANDOM_INTS 32768
#define MAX_RANDOM_INTS_SIZE (MAX_RANDOM_INTS * sizeof(uint32_t))

typedef uint8_t* (*chunkcopy_func)(uint8_t *out, uint8_t const *from, unsigned len);

uint8_t *chunkcopy_memcpy(uint8_t *out, uint8_t const *from, unsigned len) {
    return (uint8_t *)memcpy(out, from, len);
}

class chunkcopy: public benchmark::Fixture {
private:
    uint16_t *random_ints;
    uint8_t *out;

public:
    void SetUp(const ::benchmark::State& state) {
        random_ints = (uint16_t *)zng_alloc(MAX_RANDOM_INTS_SIZE);
        assert(random_ints != NULL);

        for (int32_t i = 0; i < MAX_RANDOM_INTS; i++) {
            random_ints[i] = rand();
        }

        out = (uint8_t *)zng_alloc(MAX_RANDOM_INTS_SIZE);
    }

    void Bench(benchmark::State& state, chunkcopy_func chunkcopy) {
        const uint8_t *ret;
        for (auto _ : state) {
            ret = chunkcopy(out, (uint8_t *)random_ints, MAX_RANDOM_INTS_SIZE);
            benchmark::DoNotOptimize(ret);
        }
    }

    void TearDown(const ::benchmark::State& state) {
        zng_free(random_ints);
        zng_free(out);
    }
};

#define BENCHMARK_CHUNKCOPY(name, fptr, support_flag) \
    BENCHMARK_DEFINE_F(chunkcopy, name)(benchmark::State& state) { \
        if (!support_flag) { \
            state.SkipWithError("CPU does not support " #name); \
        } \
        Bench(state, fptr); \
    } \
    BENCHMARK_REGISTER_F(chunkcopy, name) \
        ->RangeMultiplier(2)->Range(1024, MAX_RANDOM_INTS);

BENCHMARK_CHUNKCOPY(memcpy, chunkcopy_memcpy, 1);
BENCHMARK_CHUNKCOPY(c, chunkcopy_c, 1);

#ifdef ARM_NEON_CHUNKSET
BENCHMARK_CHUNKCOPY(neon, chunkcopy_neon, arm_has_neon);
#elif defined(POWER8_VSX_CHUNKSET)
BENCHMARK_CHUNKCOPY(power8, chunkcopy_power8, power_cpu_has_arch_2_07);
#endif

#ifdef X86_SSE2_CHUNKSET
BENCHMARK_CHUNKCOPY(sse2, chunkcopy_sse2, x86_cpu_has_sse2);
#endif
#ifdef X86_AVX_CHUNKSET
BENCHMARK_CHUNKCOPY(avx, chunkcopy_avx, x86_cpu_has_avx2);
#endif

typedef uint8_t* (*chunkcopy_safe_func)(uint8_t *out, uint8_t const *from, unsigned len, uint8_t *safe);

uint8_t *chunkcopy_safe_memcpy(uint8_t *out, uint8_t const *from, unsigned len, uint8_t *limit) {
    return (uint8_t *)memcpy(out, from, MAX((int32_t)(limit - from), len));
}

class chunkcopy_safe: public benchmark::Fixture {
private:
    uint16_t *random_ints;
    uint8_t *out;

public:
    void SetUp(const ::benchmark::State& state) {
        random_ints = (uint16_t *)zng_alloc(MAX_RANDOM_INTS_SIZE);
        assert(random_ints != NULL);

        for (int32_t i = 0; i < MAX_RANDOM_INTS; i++) {
            random_ints[i] = rand();
        }

        out = (uint8_t *)zng_alloc(MAX_RANDOM_INTS_SIZE);
    }

    void Bench(benchmark::State& state, chunkcopy_safe_func chunkcopy_safe) {
        const uint8_t *ret;
        uint8_t *limit = (uint8_t *)random_ints + MAX_RANDOM_INTS_SIZE;
        for (auto _ : state) {
            ret = chunkcopy_safe(out, (uint8_t *)random_ints, MAX_RANDOM_INTS_SIZE, limit);
            benchmark::DoNotOptimize(ret);
        }
    }

    void TearDown(const ::benchmark::State& state) {
        zng_free(random_ints);
        zng_free(out);
    }
};

#define BENCHMARK_CHUNKCOPYSAFE(name, fptr, support_flag) \
    BENCHMARK_DEFINE_F(chunkcopy_safe, name)(benchmark::State& state) { \
        if (!support_flag) { \
            state.SkipWithError("CPU does not support " #name); \
        } \
        Bench(state, fptr); \
    } \
    BENCHMARK_REGISTER_F(chunkcopy_safe, name) \
        ->RangeMultiplier(2)->Range(1024, MAX_RANDOM_INTS);

BENCHMARK_CHUNKCOPYSAFE(memcpy, chunkcopy_safe_memcpy, 1);
BENCHMARK_CHUNKCOPYSAFE(c, chunkcopy_safe_c, 1);

#ifdef ARM_NEON_CHUNKSET
BENCHMARK_CHUNKCOPYSAFE(neon, chunkcopy_safe_neon, arm_has_neon);
#elif defined(POWER8_VSX_CHUNKSET)
BENCHMARK_CHUNKCOPYSAFE(power8, chunkcopy_safe_power8, power_cpu_has_arch_2_07);
#endif

#ifdef X86_SSE2_CHUNKSET
BENCHMARK_CHUNKCOPYSAFE(sse2, chunkcopy_safe_sse2, x86_cpu_has_sse2);
#endif
#ifdef X86_AVX_CHUNKSET
BENCHMARK_CHUNKCOPYSAFE(avx, chunkcopy_safe_avx, x86_cpu_has_avx2);
#endif

typedef uint8_t* (*chunkmemset_func)(uint8_t *out, unsigned dist, unsigned len);

class chunkmemset: public benchmark::Fixture {
private:
    uint16_t *random_ints;
    uint8_t *out;

public:
    void SetUp(const ::benchmark::State& state) {
        random_ints = (uint16_t *)zng_alloc(MAX_RANDOM_INTS_SIZE);
        assert(random_ints != NULL);

        for (int32_t i = 0; i < MAX_RANDOM_INTS; i++) {
            random_ints[i] = rand();
        }

        out = (uint8_t *)zng_alloc(MAX_RANDOM_INTS_SIZE);
    }

    void Bench(benchmark::State& state, chunkmemset_func chunkmemset) {
        const uint32_t dist = state.range(0);
        const uint32_t len = state.range(1) - dist;
        const uint8_t *ret;
        for (auto _ : state) {
            ret = chunkmemset(out + dist, dist, len);
            benchmark::DoNotOptimize(ret);
        }
    }

    void TearDown(const ::benchmark::State& state) {
        zng_free(random_ints);
        zng_free(out);
    }
};

#define BENCHMARK_CHUNKMEMSET(name, fptr, support_flag) \
    BENCHMARK_DEFINE_F(chunkmemset, name)(benchmark::State& state) { \
        if (!support_flag) { \
            state.SkipWithError("CPU does not support " #name); \
        } \
        Bench(state, fptr); \
    } \
    BENCHMARK_REGISTER_F(chunkmemset, name) \
        ->ArgsProduct({{1,2,4,8,16,32,64,128,256}, {4096}});

BENCHMARK_CHUNKMEMSET(c, chunkmemset_c, 1);

#ifdef ARM_NEON_CHUNKSET
BENCHMARK_CHUNKMEMSET(neon, chunkmemset_neon, arm_has_neon);
#elif defined(POWER8_VSX_CHUNKSET)
BENCHMARK_CHUNKMEMSET(power8, chunkmemset_power8, power_cpu_has_arch_2_07);
#endif

#ifdef X86_SSE2_CHUNKSET
BENCHMARK_CHUNKMEMSET(sse2, chunkmemset_sse2, x86_cpu_has_sse2);
#endif
#ifdef X86_AVX_CHUNKSET
BENCHMARK_CHUNKMEMSET(avx, chunkmemset_avx, x86_cpu_has_avx2);
#endif
