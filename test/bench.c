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
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "uhash.h"

UHASH_SET_INIT(str, char*, uhash_str_hash, uhash_str_equals)
UHASH_MAP_INIT(int, uint32_t, unsigned char, uhash_int32_hash, uhash_identical)

typedef struct {
    unsigned key;
    unsigned char val;
} int_unpack_t;

typedef struct {
    unsigned key;
    unsigned char val;
} __attribute__ ((__packed__)) int_packed_t;

#define hash_eq(a, b) ((a).key == (b).key)
#define hash_func(a) ((a).key)

UHASH_SET_INIT(iun, int_unpack_t, hash_func, hash_eq)
UHASH_SET_INIT(ipk, int_packed_t, hash_func, hash_eq)

static int data_size = 5000000;
static unsigned *int_data;
static char **str_data;

void ht_init_data()
{
    int i;
    char buf[256];
    uint32_t x = 11;
    int_data = (unsigned*)calloc(data_size, sizeof(unsigned));
    str_data = (char**)calloc(data_size, sizeof(char*));
    for (i = 0; i < data_size; ++i) {
        int_data[i] = (unsigned)(data_size * ((double)x / UINT_MAX) / 4) * 271828183u;
        sprintf(buf, "%x", int_data[i]);
        str_data[i] = strdup(buf);
        x = 1664525L * x + 1013904223L;
    }
}

void ht_destroy_data()
{
    int i;
    for (i = 0; i < data_size; ++i) free(str_data[i]);
    free(str_data); free(int_data);
}

void ht_uhash_int()
{
    int i, ret;
    unsigned *data = int_data;
    UHash(int) *h;
    unsigned k;

    h = uhash_alloc(int);
    for (i = 0; i < data_size; ++i) {
        k = uhash_put(int, h, data[i], &ret);
        uhash_value(h, k) = i&0xff;
        if (!ret) uhash_delete(int, h, k);
    }
    printf("[ht_uhash_int] size: %u\n", uhash_count(h));
    uhash_free(int, h);
}

void ht_uhash_str()
{
    int i, ret;
    char **data = str_data;
    UHash(str) *h;
    unsigned k;

    h = uhash_alloc(str);
    for (i = 0; i < data_size; ++i) {
        k = uhash_put(str, h, data[i], &ret);
        if (!ret) uhash_delete(str, h, k);
    }
    printf("[ht_uhash_int] size: %u\n", uhash_count(h));
    uhash_free(str, h);
}

void ht_uhash_unpack()
{
    int i, ret;
    unsigned *data = int_data;
    UHash(iun) *h;
    unsigned k;

    h = uhash_alloc(iun);
    for (i = 0; i < data_size; ++i) {
        int_unpack_t x;
        x.key = data[i]; x.val = i&0xff;
        k = uhash_put(iun, h, x, &ret);
        if (!ret) uhash_delete(iun, h, k);
    }
    printf("[ht_uhash_unpack] size: %u (sizeof=%ld)\n", uhash_count(h), sizeof(int_unpack_t));
    uhash_free(iun, h);
}

void ht_uhash_packed()
{
    int i, ret;
    unsigned *data = int_data;
    UHash(ipk) *h;
    unsigned k;

    h = uhash_alloc(ipk);
    for (i = 0; i < data_size; ++i) {
        int_packed_t x;
        x.key = data[i]; x.val = i&0xff;
        k = uhash_put(ipk, h, x, &ret);
        if (!ret) uhash_delete(ipk, h, k);
    }
    printf("[ht_uhash_packed] size: %u (sizeof=%ld)\n", uhash_count(h), sizeof(int_packed_t));
    uhash_free(ipk, h);
}

void ht_timing(void (*f)(void))
{
    clock_t t = clock();
    (*f)();
    printf("[ht_timing] %.3lf sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);
}

int main(int argc, char *argv[])
{
    printf("Starting benchmark...\n");
    if (argc > 1) data_size = atoi(argv[1]);
    ht_init_data();
    ht_timing(ht_uhash_int);
    ht_timing(ht_uhash_str);
    ht_timing(ht_uhash_unpack);
    ht_timing(ht_uhash_packed);
    ht_destroy_data();
    printf("Benchmark finished.\n");
    return 0;
}
