#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#include <benchmark/benchmark.h>

#include "zbuild.h"
#include "zutil.h"
#include "functable.h"

#define MAX_RANDOM_INTS (1024 * 1024)

extern uint32_t *random_ints;

typedef uint32_t (*adler32_func)(uint32_t adler, const unsigned char *buf, size_t len);

static inline void adler32_bench(benchmark::State& state, adler32_func adler32) {
    uint32_t hash = 0;

    for (auto _ : state) {
        hash = adler32(hash, (const unsigned char *)random_ints, state.range(0) * sizeof(uint32_t));
    }

    benchmark::DoNotOptimize(hash);
}

static void adler32_c_bench(benchmark::State& state) {
    adler32_bench(state, adler32_c);
}
BENCHMARK(adler32_c_bench)->Range(1, MAX_RANDOM_INTS);
#ifdef ARM_NEON_ADLER32
static void adler32_neon_bench(benchmark::State& state) {
    adler32_bench(state, adler32_neon);
}
BENCHMARK(adler32_neon_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef PPC_VMX_ADLER32
static void adler32_vmx_bench(benchmark::State& state) {
    adler32_bench(state, adler32_vmx);
}
BENCHMARK(adler32_vmx_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef X86_SSE41_ADLER32
static void adler32_sse41_bench(benchmark::State& state) {
    adler32_bench(state, adler32_sse41);
}
BENCHMARK(adler32_sse41_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef X86_SSSE3_ADLER32
static void adler32_ssse3_bench(benchmark::State& state) {
    adler32_bench(state, adler32_ssse3);
}
BENCHMARK(adler32_ssse3_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef X86_AVX2_ADLER32
static void adler32_avx2_bench(benchmark::State& state) {
    adler32_bench(state, adler32_avx2);
}
BENCHMARK(adler32_avx2_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#if defined(X86_AVX512_ADLER32)
static void adler32_avx512_bench(benchmark::State& state) {
    if (!x86_cpu_has_avx512) {
        state.SkipWithError("AVX512 not supported");
        return;
    }
    adler32_bench(state, adler32_avx512);
}
BENCHMARK(adler32_avx512_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#if defined(X86_AVX512VNNI_ADLER32)
static void adler32_avx512_vnni_bench(benchmark::State& state) {
    if (!x86_cpu_has_avx512vnni) {
        state.SkipWithError("AVX512_VNNI not supported");
        return;
    }
    adler32_bench(state, adler32_avx512_vnni);
}
BENCHMARK(adler32_avx512_vnni_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef POWER8_VSX_ADLER32
static void adler32_power8_bench(benchmark::State& state) {
    adler32_bench(state, adler32_power8);
}
BENCHMARK(adler32_power8_bench)->Range(1, MAX_RANDOM_INTS);
#endif
