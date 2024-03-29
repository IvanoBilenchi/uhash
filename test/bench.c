/**
 * Benchmarks for the uHash library.
 *
 * @author Attractive Chaos (khash)
 * @author Ivano Bilenchi (uhash)
 *
 * @copyright Copyright (c) 2008, 2009, 2011 Attractive Chaos <attractor@live.co.uk>
 * @copyright Copyright (c) 2019 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include <stdio.h>
#include <time.h>

#include "uhash.h"

UHASH_INIT(str, char*, UHASH_VAL_IGNORE, uhash_str_hash, uhash_str_equals)
UHASH_INIT(int, uint32_t, unsigned char, uhash_int32_hash, uhash_identical)

static uhash_uint int_hash(uint32_t num) { return uhash_int32_hash(num); }
static bool int_eq(uint32_t lhs, uint32_t rhs) { return lhs == rhs; }
UHASH_INIT_PI(intpi, uint32_t, unsigned char, NULL, NULL)

static uhash_uint str_hash(char *str) { return uhash_str_hash(str); }
static bool str_eq(char *lhs, char *rhs) { return strcmp(lhs, rhs) == 0; }
UHASH_INIT_PI(strpi, char*, UHASH_VAL_IGNORE, NULL, NULL)

typedef struct {
    uint32_t key;
    unsigned char val;
} int_unpack_t;

#if defined __GNUC__
    #define UHASH_PACKED_STRUCT(NAME, FIELDS) typedef struct __attribute__((packed)) FIELDS NAME
#elif defined _MSC_VER
    #define UHASH_PACKED_STRUCT(NAME, FIELDS) \
        __pragma(pack(push, 1)) typedef struct FIELDS NAME __pragma(pack(pop))
#else
    #define UHASH_PACKED_STRUCT(NAME, FIELDS) typedef struct FIELDS NAME
#endif

UHASH_PACKED_STRUCT(int_packed_t, {
    uint32_t key;
    unsigned char val;
});

#define hash_eq(a, b) ((a).key == (b).key)
#define hash_func(a) ((a).key)

UHASH_INIT(iun, int_unpack_t, UHASH_VAL_IGNORE, hash_func, hash_eq)
UHASH_INIT(ipk, int_packed_t, UHASH_VAL_IGNORE, hash_func, hash_eq)

static uint32_t data_size = 5000000;
static uint32_t *int_data;
static char **str_data;

void ht_init_data(void) {
    char buf[256];
    uint32_t x = 11;

    int_data = calloc(data_size, sizeof(*int_data));
    str_data = calloc(data_size, sizeof(*str_data));

    for (uint32_t i = 0; i < data_size; ++i) {
        int_data[i] = (uint32_t)(data_size * ((double)x / UINT32_MAX) / 4) * 271828183U;
        snprintf(buf, sizeof(buf), "%x", int_data[i]);
        str_data[i] = malloc(sizeof(buf));
        memcpy(str_data[i], buf, sizeof(buf));
        x = 1664525 * x + 1013904223;
    }
}

void ht_destroy_data(void) {
    for (uint32_t i = 0; i < data_size; ++i) free(str_data[i]);
    free(str_data);
    free(int_data);
}

void ht_uhash_int(void) {
    uint32_t *data = int_data;
    UHash(int) *h = uhmap_alloc(int);

    for (uint32_t i = 0; i < data_size; ++i) {
        uhash_uint k;
        uhash_put(int, h, data[i], &k);
        uhash_value(h, k) = (unsigned char)(i & 0xffU);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_int] size: %llu\n", count);
    uhash_free(int, h);
}

void ht_uhash_int_pi(void) {
    uint32_t *data = int_data;
    UHash(intpi) *h = uhmap_alloc_pi(intpi, int_hash, int_eq);

    for (uint32_t i = 0; i < data_size; ++i) {
        uhash_uint k;
        uhash_put(intpi, h, data[i], &k);
        uhash_value(h, k) = (unsigned char)(i & 0xffU);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_int] size: %llu\n", count);
    uhash_free(intpi, h);
}

void ht_uhash_str(void) {
    char **data = str_data;
    UHash(str) *h = uhset_alloc(str);

    for (uint32_t i = 0; i < data_size; ++i) {
        uhash_put(str, h, data[i], NULL);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_int] size: %llu\n", count);
    uhash_free(str, h);
}

void ht_uhash_str_pi(void) {
    char **data = str_data;
    UHash(strpi) *h = uhset_alloc_pi(strpi, str_hash, str_eq);

    for (uint32_t i = 0; i < data_size; ++i) {
        uhash_put(strpi, h, data[i], NULL);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_int] size: %llu\n", count);
    uhash_free(strpi, h);
}

void ht_uhash_unpack(void) {
    uint32_t *data = int_data;
    UHash(iun) *h = uhset_alloc(iun);

    for (uint32_t i = 0; i < data_size; ++i) {
        int_unpack_t x;
        x.key = data[i];
        x.val = (unsigned char)(i & 0xffU);
        uhash_put(iun, h, x, NULL);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_unpack] size: %llu (sizeof=%u)\n", count, (unsigned)sizeof(int_unpack_t));
    uhash_free(iun, h);
}

void ht_uhash_packed(void) {
    uint32_t *data = int_data;
    UHash(ipk) *h = uhset_alloc(ipk);

    for (uint32_t i = 0; i < data_size; ++i) {
        int_packed_t x;
        x.key = data[i];
        x.val = (unsigned char)(i & 0xffU);
        uhash_put(ipk, h, x, NULL);
    }

    uint64_t count = uhash_count(h);
    printf("[ht_uhash_packed] size: %llu (sizeof=%u)\n", count, (unsigned)sizeof(int_packed_t));
    uhash_free(ipk, h);
}

void ht_timing(void (*f)(void)) {
    clock_t t = clock();
    (*f)();
    printf("[ht_timing] %.3lf sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);
}

int main(int argc, char *argv[]) {
    if (argc > 1) data_size = (uint32_t)strtoul(argv[1], NULL, 10);

    printf("Starting benchmark...\n");

    ht_init_data();
    ht_timing(ht_uhash_int);
    ht_timing(ht_uhash_int_pi);
    ht_timing(ht_uhash_str);
    ht_timing(ht_uhash_str_pi);
    ht_timing(ht_uhash_unpack);
    ht_timing(ht_uhash_packed);
    ht_destroy_data();

    printf("Benchmark finished.\n");

    return 0;
}
