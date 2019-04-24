/** @file
 * Tests for the uHash library.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2019 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
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

UHASH_SET_INIT(IntSet, uint32_t, uhash_int32_hash, uhash_identical)
UHASH_MAP_INIT(IntMap, uint32_t, uint32_t, uhash_int32_hash, uhash_identical)

static bool test_memory(void) {
    UHash(IntSet) *set = uhash_alloc(IntSet);
    uhash_ret_t ret;
    uhash_put(IntSet, set, 0, &ret);
    uhash_assert(uhash_count(set) == 1);

    uhash_uint_t buckets = set->n_buckets;
    uhash_resize(IntSet, set, 200);
    uhash_assert(set->n_buckets > buckets);

    buckets = set->n_buckets;
    uhash_resize(IntSet, set, 100);
    uhash_assert(set->n_buckets < buckets);

    buckets = set->n_buckets;
    uhash_clear(IntSet, set);
    uhash_assert(set->n_buckets == buckets);
    uhash_assert(uhash_count(set) == 0);

    uhash_free(IntSet, set);
    return true;
}

/// @name Tests

static bool test_base(void) {
    uint32_t const max = 100;
    UHash(IntSet) *set = uhash_alloc(IntSet);

    uhash_assert(uhash_get(IntSet, set, 0) == UHASH_INDEX_MISSING);
    uhash_assert(uhash_count(set) == 0);

    for (uint32_t i = 0; i < max; ++i) {
        uhash_ret_t ret;
        uhash_put(IntSet, set, i, &ret);
        uhash_assert(ret == UHASH_INSERTED);
    }

    uhash_assert(uhash_count(set) == max);

    for (uint32_t i = 0; i < max; ++i) {
        uhash_uint_t idx = uhash_get(IntSet, set, i);
        uhash_assert(idx != UHASH_INDEX_MISSING);
        uhash_assert(uhash_exists(set, idx));
    }

    uhash_assert(uhash_get(IntSet, set, 200) == UHASH_INDEX_MISSING);

    for (uint32_t i = 0; i < max; ++i) {
        uhash_uint_t idx = uhash_get(IntSet, set, i);
        uhash_delete(IntSet, set, idx);
        uhash_assert(!uhash_exists(set, idx));
        uhash_assert(uhash_get(IntSet, set, i) == UHASH_INDEX_MISSING);
    }

    uhash_assert(uhash_count(set) == 0);

    uhash_free(IntSet, set);
    return true;
}

static bool test_map(void) {
    uint32_t const max = 100;
    UHash(IntMap) *map = uhash_alloc(IntMap);

    for (uint32_t i = 0; i < max; ++i) {
        uhash_assert(uhmap_overwrite(IntMap, map, i, i) == UHASH_INSERTED);
    }

    uint32_t existing_val;
    uhash_assert(uhmap_set(IntMap, map, 0, 0, &existing_val) == UHASH_PRESENT);
    uhash_assert(existing_val == 0);

    uhash_assert(uhmap_add(IntMap, map, 0, 1, &existing_val) == UHASH_PRESENT);
    uhash_assert(existing_val == 0);

    uhash_assert(uhmap_add(IntMap, map, max, max, &existing_val) == UHASH_INSERTED);
    uhash_assert(uhmap_remove(IntMap, map, max));

    for (uint32_t i = 0; i < max; ++i) {
        uint32_t existing_key;
        uhash_assert(uhmap_pop(IntMap, map, i, &existing_key, &existing_val));
        uhash_assert(existing_key == i);
        uhash_assert(existing_val == i);
    }

    uhash_free(IntMap, map);
    return true;
}

static bool test_set(void) {
    uint32_t const max = 100;
    UHash(IntSet) *set = uhash_alloc(IntSet);
    uhash_ret_t ret;
    
    for (uint32_t i = 0; i < max; ++i) {
        ret = uhset_insert(IntSet, set, i);
        uhash_assert(ret == UHASH_INSERTED);
    }

    uhash_assert(uhset_insert(IntSet, set, 0) == UHASH_PRESENT);
    uhash_assert(uhash_count(set) == max);

    for (uint32_t i = 0; i < max; ++i) {
        uint32_t existing;
        ret = uhset_insert_get_existing(IntSet, set, i, &existing);
        uhash_assert(ret == UHASH_PRESENT);
        uhash_assert(existing == i);
    }

    uint32_t elements[max + 1] = {};
    for (uint32_t i = 0; i < max + 1; ++i) {
        elements[i] = i;
    }

    ret = uhset_insert_all(IntSet, set, elements, max);
    uhash_assert(ret == UHASH_PRESENT);

    ret = uhset_insert_all(IntSet, set, elements, max + 1);
    uhash_assert(ret == UHASH_INSERTED);

    uhash_assert(uhash_contains(IntSet, set, max));
    uhash_assert(uhset_remove(IntSet, set, max));
    uhash_assert(!uhash_contains(IntSet, set, max));

    for (uint32_t i = 0; i < max; ++i) {
        uint32_t existing;
        uhash_assert(uhset_pop(IntSet, set, i, &existing));
        uhash_assert(existing == i);
    }

    UHash(IntSet) *other_set = uhash_alloc(IntSet);
    uhset_insert_all(IntSet, set, elements, max);
    uhset_insert_all(IntSet, other_set, elements, max / 2);

    uhash_assert(uhset_is_superset(IntSet, set, other_set));
    uhash_assert(!uhset_is_superset(IntSet, other_set, set));

    uhash_assert(!uhset_equals(IntSet, set, other_set));
    uhset_insert_all(IntSet, other_set, elements, max);
    uhash_assert(uhset_equals(IntSet, set, other_set));

    uint32_t element = uhset_get_any(IntSet, set, max);
    uhash_assert(element != max);

    uhash_clear(IntSet, set);
    element = uhset_get_any(IntSet, set, max);
    uhash_assert(element == max);

    uhash_free(IntSet, set);
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
