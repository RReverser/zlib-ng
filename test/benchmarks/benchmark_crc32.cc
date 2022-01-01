#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#include <benchmark/benchmark.h>

#include "zbuild.h"
#include "zutil.h"

#include "functable.h"

#define MAX_RANDOM_INTS (1024 * 1024)

extern uint32_t *random_ints;

typedef uint32_t (*crc32_func)(uint32_t crc32, const unsigned char * buf, uint64_t len);

static inline void crc32_bench(benchmark::State& state, crc32_func crc32) {
    uint32_t hash = 0;

    for (auto _ : state) {
        hash = crc32(hash, (const unsigned char *)random_ints, state.range(0) * sizeof(uint32_t));
    }

    benchmark::DoNotOptimize(hash);
}

static void crc32_generic_bench(benchmark::State& state) {
    crc32_bench(state, crc32_generic);
}
BENCHMARK(crc32_generic_bench)->Range(1, MAX_RANDOM_INTS);
#if BYTE_ORDER == LITTLE_ENDIAN
static void crc32_little_bench(benchmark::State& state) {
    crc32_bench(state, crc32_little);
}
BENCHMARK(crc32_little_bench)->Range(1, MAX_RANDOM_INTS);
#elif BYTE_ORDER == BIG_ENDIAN
static void crc32_big_bench(benchmark::State& state) {
    crc32_bench(state, crc32_big);
}
BENCHMARK(crc32_big_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef X86_PCLMULQDQ_CRC
/* CRC32 fold does a memory copy while hashing */
uint32_t crc32_pclmulqdq_bench(uint32_t crc32, const unsigned char* buf, uint64_t len) {
    crc32_fold ALIGNED_(16) crc_state;
    crc32_fold_reset_pclmulqdq(&crc_state);
    crc32_fold_copy_pclmulqdq(&crc_state, (uint8_t *)buf, buf, len);
    return crc32_fold_final_pclmulqdq(&crc_state);
}
static void crc32_pclmulqdq_bench(benchmark::State& state) {
    crc32_bench(state, crc32_pclmulqdq_bench);
}
BENCHMARK(crc32_pclmulqdq_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef ARM_ACLE_CRC_HASH
static void crc32_acle_bench(benchmark::State& state) {
    crc32_bench(state, crc32_acle);
}
BENCHMARK(crc32_acle_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef POWER8_VSX_CRC32
static void crc32_power8_bench(benchmark::State& state) {
    crc32_bench(state, crc32_power8);
}
BENCHMARK(crc32_power8_bench)->Range(1, MAX_RANDOM_INTS);
#endif
#ifdef S390_CRC32_VX
static void crc32_vx_bench(benchmark::State& state) {
    crc32_bench(state, crc32_vx);
}
BENCHMARK(crc32_vx_bench)->Range(1, MAX_RANDOM_INTS);
#endif
