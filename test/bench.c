/** @file
 * Benchmarks for the uHash library.
 *
 * @author Attractive Chaos (khash)
 * @author Ivano Bilenchi (uhash)
 *
 * @copyright Copyright (c) 2008, 2009, 2011 Attractive Chaos <attractor@live.co.uk>
 * @copyright Copyright (c) 2019 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <time.h>

#include "uhash.h"

UHASH_SET_INIT(str, char*, uhash_str_hash, uhash_str_equals)
UHASH_MAP_INIT(int, uint32_t, unsigned char, uhash_int32_hash, uhash_identical)

typedef struct {
    uint32_t key;
    unsigned char val;
} int_unpack_t;

typedef struct {
    uint32_t key;
    unsigned char val;
} __attribute__ ((__packed__)) int_packed_t;

#define hash_eq(a, b) ((a).key == (b).key)
#define hash_func(a) ((a).key)

UHASH_SET_INIT(iun, int_unpack_t, hash_func, hash_eq)
UHASH_SET_INIT(ipk, int_packed_t, hash_func, hash_eq)

static uint32_t data_size = 5000000;
static uint32_t *int_data;
static char **str_data;

void ht_init_data(void) {
    char buf[256];
    uint32_t x = 11;

    int_data = calloc(data_size, sizeof(*int_data));
    str_data = calloc(data_size, sizeof(*str_data));

    for (uint32_t i = 0; i < data_size; ++i) {
        int_data[i] = (uint32_t)(data_size * ((double)x / UINT32_MAX) / 4) * 271828183u;
        sprintf(buf, "%x", int_data[i]);
        str_data[i] = strdup(buf);
        x = 1664525L * x + 1013904223L;
    }
}

void ht_destroy_data(void) {
    for (uint32_t i = 0; i < data_size; ++i) free(str_data[i]);
    free(str_data);
    free(int_data);
}

void ht_uhash_int(void) {
    uint32_t *data = int_data;
    UHash(int) *h = uhash_alloc(int);

    for (uint32_t i = 0; i < data_size; ++i) {
        uhash_uint_t k = uhash_put(int, h, data[i], NULL);
        uhash_value(h, k) = i&0xff;
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_int] size: %llu\n", count);
    uhash_free(int, h);
}

void ht_uhash_str(void) {
    char **data = str_data;
    UHash(str) *h = uhash_alloc(str);

    for (uint32_t i = 0; i < data_size; ++i) {
        uhash_put(str, h, data[i], NULL);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_int] size: %llu\n", count);
    uhash_free(str, h);
}

void ht_uhash_unpack(void) {
    uint32_t *data = int_data;
    UHash(iun) *h = uhash_alloc(iun);

    for (uint32_t i = 0; i < data_size; ++i) {
        int_unpack_t x;
        x.key = data[i]; x.val = i&0xff;
        uhash_put(iun, h, x, NULL);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_unpack] size: %llu (sizeof=%ld)\n", count, sizeof(int_unpack_t));
    uhash_free(iun, h);
}

void ht_uhash_packed(void) {
    uint32_t *data = int_data;
    UHash(ipk) *h = uhash_alloc(ipk);

    for (uint32_t i = 0; i < data_size; ++i) {
        int_packed_t x;
        x.key = data[i]; x.val = i&0xff;
        uhash_put(ipk, h, x, NULL);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_packed] size: %llu (sizeof=%ld)\n", count, sizeof(int_packed_t));
    uhash_free(ipk, h);
}

void ht_timing(void (*f)(void)) {
    clock_t t = clock();
    (*f)();
    printf("[ht_timing] %.3lf sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);
}

int main(int argc, char *argv[]) {
    if (argc > 1) data_size = atoi(argv[1]);

    printf("Starting benchmark...\n");

    ht_init_data();
    ht_timing(ht_uhash_int);
    ht_timing(ht_uhash_str);
    ht_timing(ht_uhash_unpack);
    ht_timing(ht_uhash_packed);
    ht_destroy_data();

    printf("Benchmark finished.\n");

    return 0;
}
