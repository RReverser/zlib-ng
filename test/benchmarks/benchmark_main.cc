#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

#include <benchmark/benchmark.h>

#include "zbuild.h"
#include "zutil.h"

#include "functable.h"

#define MAX_RANDOM_INTS (1024 * 1024)

uint32_t *random_ints = NULL;

int main(int argc, char** argv) {
    int32_t random_ints_size = MAX_RANDOM_INTS * sizeof(uint32_t);

    random_ints = (uint32_t *)malloc(random_ints_size);
    if (random_ints == NULL)
        return EXIT_FAILURE;

    for (int32_t i = 0; i < MAX_RANDOM_INTS; i++) {
        random_ints[i] = rand();
    }

    cpu_check_features();

    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();

    free(random_ints);

    return EXIT_SUCCESS;
}