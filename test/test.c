/**
 * Tests for the uHash library.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2019 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#include <stdio.h>
#include "uhash.h"

/// @name Utility macros

#define array_size(array) (sizeof(array) / sizeof(*array))

#define uhash_assert(exp) do {                                                                      \
    if (!(exp)) {                                                                                   \
        printf("Test failed: %s, line %d (%s)\n", __func__, __LINE__, #exp);                        \
        return false;                                                                               \
    }                                                                                               \
} while(0)

/// @name Type definitions

UHASH_INIT(IntHash, uint32_t, uint32_t, uhash_int32_hash, uhash_identical)

static bool test_memory(void) {
    UHash(IntHash) *set = uhset_alloc(IntHash);
    uhash_ret_t ret;
    uhash_put(IntHash, set, 0, &ret);
    uhash_assert(uhash_count(set) == 1);

    uhash_uint_t buckets = set->n_buckets;
    uhash_resize(IntHash, set, 200);
    uhash_assert(set->n_buckets > buckets);

    buckets = set->n_buckets;
    uhash_resize(IntHash, set, 100);
    uhash_assert(set->n_buckets < buckets);

    buckets = set->n_buckets;
    uhash_clear(IntHash, set);
    uhash_assert(set->n_buckets == buckets);
    uhash_assert(uhash_count(set) == 0);

    uhash_free(IntHash, set);
    return true;
}

/// @name Tests

static bool test_base(void) {
    uint32_t const max = 100;
    UHash(IntHash) *set = uhset_alloc(IntHash);

    uhash_assert(uhash_get(IntHash, set, 0) == UHASH_INDEX_MISSING);
    uhash_assert(uhash_count(set) == 0);

    for (uint32_t i = 0; i < max; ++i) {
        uhash_ret_t ret;
        uhash_put(IntHash, set, i, &ret);
        uhash_assert(ret == UHASH_INSERTED);
    }

    uhash_assert(uhash_count(set) == max);

    for (uint32_t i = 0; i < max; ++i) {
        uhash_uint_t idx = uhash_get(IntHash, set, i);
        uhash_assert(idx != UHASH_INDEX_MISSING);
        uhash_assert(uhash_exists(set, idx));
    }

    uhash_assert(uhash_get(IntHash, set, 200) == UHASH_INDEX_MISSING);

    for (uint32_t i = 0; i < max; ++i) {
        uhash_uint_t idx = uhash_get(IntHash, set, i);
        uhash_delete(IntHash, set, idx);
        uhash_assert(!uhash_exists(set, idx));
        uhash_assert(uhash_get(IntHash, set, i) == UHASH_INDEX_MISSING);
    }

    uhash_assert(uhash_count(set) == 0);

    uhash_free(IntHash, set);
    return true;
}

static bool test_map(void) {
    uint32_t const max = 100;
    UHash(IntHash) *map = uhmap_alloc(IntHash);

    for (uint32_t i = 0; i < max; ++i) {
        uhash_assert(uhmap_set(IntHash, map, i, i, NULL) == UHASH_INSERTED);
    }

    UHash(IntHash) *set = uhash_copy_as_set(IntHash, map);
    uhash_assert(uhset_equals(IntHash, set, map));
    uhash_free(IntHash, set);

    uint32_t existing_val;
    uhash_assert(uhmap_set(IntHash, map, 0, 1, &existing_val) == UHASH_PRESENT);
    uhash_assert(existing_val == 0);

    uhash_assert(uhmap_add(IntHash, map, 0, 1, &existing_val) == UHASH_PRESENT);
    uhash_assert(existing_val == 1);

    uhash_assert(uhmap_replace(IntHash, map, 0, 0, &existing_val));
    uhash_assert(uhmap_get(IntHash, map, 0, UINT32_MAX) == 0);
    uhash_assert(existing_val == 1);

    uhash_assert(uhmap_add(IntHash, map, max, max, &existing_val) == UHASH_INSERTED);
    uhash_assert(uhmap_remove(IntHash, map, max));

    for (uint32_t i = 0; i < max; ++i) {
        uint32_t existing_key;
        uhash_assert(uhmap_pop(IntHash, map, i, &existing_key, &existing_val));
        uhash_assert(existing_key == i);
        uhash_assert(existing_val == i);
    }

    uhash_free(IntHash, map);
    return true;
}

static bool test_set(void) {
    uint32_t const max = 100;
    UHash(IntHash) *set = uhset_alloc(IntHash);
    
    for (uint32_t i = 0; i < max; ++i) {
        uhash_assert(uhset_insert(IntHash, set, i) == UHASH_INSERTED);
    }

    uhash_assert(uhset_insert(IntHash, set, 0) == UHASH_PRESENT);
    uhash_assert(uhash_count(set) == max);

    for (uint32_t i = 0; i < max; ++i) {
        uint32_t existing;
        uhash_assert(uhset_insert_get_existing(IntHash, set, i, &existing) == UHASH_PRESENT);
        uhash_assert(existing == i);
    }

    uint32_t elements[max + 1] = {};
    for (uint32_t i = 0; i < max + 1; ++i) {
        elements[i] = i;
    }

    uhash_assert(uhset_insert_all(IntHash, set, elements, max) == UHASH_PRESENT);
    uhash_assert(uhset_insert_all(IntHash, set, elements, max + 1) == UHASH_INSERTED);

    uhash_assert(uhash_contains(IntHash, set, max));
    uhash_assert(uhset_remove(IntHash, set, max));
    uhash_assert(!uhash_contains(IntHash, set, max));

    for (uint32_t i = 0; i < max; ++i) {
        uint32_t existing;
        uhash_assert(uhset_pop(IntHash, set, i, &existing));
        uhash_assert(existing == i);
    }

    UHash(IntHash) *other_set = uhset_alloc(IntHash);
    uhset_insert_all(IntHash, set, elements, max);
    uhset_insert_all(IntHash, other_set, elements, max / 2);

    uhash_assert(uhset_is_superset(IntHash, set, other_set));
    uhash_assert(!uhset_is_superset(IntHash, other_set, set));

    uhash_assert(!uhset_equals(IntHash, set, other_set));
    uhset_insert_all(IntHash, other_set, elements, max);
    uhash_assert(uhset_equals(IntHash, set, other_set));

    uhash_free(IntHash, other_set);
    other_set = uhash_copy(IntHash, set);
    uhash_assert(uhset_equals(IntHash, set, other_set));

    uint32_t element = uhset_get_any(IntHash, set, max);
    uhash_assert(element != max);

    uint32_t replaced;
    uhash_assert(uhset_replace(IntHash, set, element, &replaced));
    uhash_assert(replaced == element);

    uhash_clear(IntHash, set);
    element = uhset_get_any(IntHash, set, max);
    uhash_assert(element == max);

    uhash_free(IntHash, set);
    return true;
}

int main(void) {
    printf("Starting tests...\n");
    
    int exit_code = EXIT_SUCCESS;
    bool (*tests[])(void) = {
        test_memory,
        test_base,
        test_map,
        test_set
    };

    for (uint32_t i = 0; i < array_size(tests); ++i) {
        if (!tests[i]()) exit_code = EXIT_FAILURE;
    }

    if (exit_code == EXIT_SUCCESS) {
        printf("All tests passed.\n");
    } else {
        printf("Some tests failed.\n");
    }
    
    return exit_code;
}
