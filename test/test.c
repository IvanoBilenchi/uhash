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

#define array_size(array) (sizeof(array) / sizeof(*(array)))

#define uhash_assert(exp) do {                                                                      \
    if (!(exp)) {                                                                                   \
        printf("Test failed: %s, line %d (%s)\n", __func__, __LINE__, #exp);                        \
        return false;                                                                               \
    }                                                                                               \
} while(0)

/// @name Type definitions

UHASH_INIT(IntHash, uint32_t, uint32_t, uhash_int32_hash, uhash_identical)
UHASH_INIT_PI(IntHashPi, uint32_t, uint32_t)

static bool test_memory(void) {
    UHash(IntHash) *set = uhset_alloc(IntHash);
    uhash_assert(set);

    uhash_ret ret = uhash_put(IntHash, set, 0, NULL);
    uhash_assert(ret == UHASH_INSERTED);
    uhash_assert(uhash_count(set) == 1);

    uhash_uint buckets = set->n_buckets;
    ret = uhash_resize(IntHash, set, 200);
    uhash_assert(ret == UHASH_OK);
    uhash_assert(set->n_buckets > buckets);

    buckets = set->n_buckets;
    ret = uhash_resize(IntHash, set, 100);
    uhash_assert(ret == UHASH_OK);
    uhash_assert(set->n_buckets < buckets);

    buckets = set->n_buckets;
    uhash_clear(IntHash, set);
    uhash_assert(set->n_buckets == buckets);
    uhash_assert(uhash_count(set) == 0);

    uhash_free(IntHash, set);
    return true;
}

/// @name Tests

#define MAX_VAL 100

static bool test_base(void) {
    UHash(IntHash) *set = uhset_alloc(IntHash);
    uhash_assert(set);

    uhash_assert(uhash_get(IntHash, set, 0) == UHASH_INDEX_MISSING);
    uhash_assert(uhash_count(set) == 0);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uhash_assert(uhash_put(IntHash, set, i, NULL) == UHASH_INSERTED);
    }

    uhash_assert(uhash_count(set) == MAX_VAL);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uhash_uint idx = uhash_get(IntHash, set, i);
        uhash_assert(idx != UHASH_INDEX_MISSING);
        uhash_assert(uhash_exists(set, idx));
    }

    uhash_assert(uhash_get(IntHash, set, 200) == UHASH_INDEX_MISSING);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uhash_uint idx = uhash_get(IntHash, set, i);
        uhash_delete(IntHash, set, idx);
        uhash_assert(!uhash_exists(set, idx));
        uhash_assert(uhash_get(IntHash, set, i) == UHASH_INDEX_MISSING);
    }

    uhash_assert(uhash_count(set) == 0);

    uhash_free(IntHash, set);
    return true;
}

static bool test_map(void) {
    UHash(IntHash) *map = uhmap_alloc(IntHash);
    uhash_assert(map);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uhash_assert(uhmap_set(IntHash, map, i, i, NULL) == UHASH_INSERTED);
    }

    UHash(IntHash) *set = uhset_alloc(IntHash);
    uhash_assert(set);
    uhash_assert(uhash_copy_as_set(IntHash, map, set) == UHASH_OK);
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

    uhash_assert(uhmap_add(IntHash, map, MAX_VAL, MAX_VAL, &existing_val) == UHASH_INSERTED);
    uhash_assert(uhmap_remove(IntHash, map, MAX_VAL));

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uint32_t existing_key;
        uhash_assert(uhmap_pop(IntHash, map, i, &existing_key, &existing_val));
        uhash_assert(existing_key == i);
        uhash_assert(existing_val == i);
    }

    uhash_free(IntHash, map);
    return true;
}

static bool test_set(void) {
    UHash(IntHash) *set = uhset_alloc(IntHash);
    uhash_assert(set);
    
    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uhash_assert(uhset_insert(IntHash, set, i) == UHASH_INSERTED);
    }

    uhash_assert(uhset_insert(IntHash, set, 0) == UHASH_PRESENT);
    uhash_assert(uhash_count(set) == MAX_VAL);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uint32_t existing;
        uhash_assert(uhset_insert_get_existing(IntHash, set, i, &existing) == UHASH_PRESENT);
        uhash_assert(existing == i);
    }

    uint32_t elements[MAX_VAL + 1];
    for (uint32_t i = 0; i < MAX_VAL + 1; ++i) {
        elements[i] = i;
    }

    uhash_assert(uhset_insert_all(IntHash, set, elements, MAX_VAL) == UHASH_PRESENT);
    uhash_assert(uhset_insert_all(IntHash, set, elements, MAX_VAL + 1) == UHASH_INSERTED);

    uhash_assert(uhash_contains(IntHash, set, MAX_VAL));
    uhash_assert(uhset_remove(IntHash, set, MAX_VAL));
    uhash_assert(!uhash_contains(IntHash, set, MAX_VAL));

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uint32_t existing;
        uhash_assert(uhset_pop(IntHash, set, i, &existing));
        uhash_assert(existing == i);
    }

    UHash(IntHash) *other_set = uhset_alloc(IntHash);
    uhash_assert(other_set);
    uhset_insert_all(IntHash, set, elements, MAX_VAL);
    uhset_insert_all(IntHash, other_set, elements, MAX_VAL / 2);

    uhash_assert(uhset_is_superset(IntHash, set, other_set));
    uhash_assert(!uhset_is_superset(IntHash, other_set, set));

    uhash_assert(!uhset_equals(IntHash, set, other_set));
    uhset_insert_all(IntHash, other_set, elements, MAX_VAL);
    uhash_assert(uhset_equals(IntHash, set, other_set));

    uhash_free(IntHash, other_set);
    other_set = uhset_alloc(IntHash);
    uhash_assert(other_set);
    uhash_assert(uhash_copy(IntHash, set, other_set) == UHASH_OK);
    uhash_assert(uhset_equals(IntHash, set, other_set));
    uhash_free(IntHash, other_set);

    other_set = uhset_alloc(IntHash);
    uhash_assert(other_set);
    uhset_insert(IntHash, other_set, MAX_VAL);
    uhash_assert(uhset_union(IntHash, other_set, set) == UHASH_OK);

    uhash_assert(uhset_is_superset(IntHash, other_set, set));
    uhash_assert(!uhset_is_superset(IntHash, set, other_set));

    uhset_intersect(IntHash, other_set, set);
    uhash_assert(uhset_equals(IntHash, other_set, set));
    uhash_free(IntHash, other_set);

    uint32_t element = uhset_get_any(IntHash, set, MAX_VAL);
    uhash_assert(element != MAX_VAL);

    uint32_t replaced;
    uhash_assert(uhset_replace(IntHash, set, element, &replaced));
    uhash_assert(replaced == element);

    uhash_clear(IntHash, set);
    element = uhset_get_any(IntHash, set, MAX_VAL);
    uhash_assert(element == MAX_VAL);

    uhash_free(IntHash, set);
    return true;
}

static uhash_uint int32_hash(uint32_t num) {
    return uhash_int32_hash(num);
}

static bool int32_eq(uint32_t lhs, uint32_t rhs) {
    return lhs == rhs;
}

static bool test_per_instance(void) {
    UHash(IntHashPi) *map = uhmap_alloc_pi(IntHashPi, int32_hash, int32_eq);
    uhash_assert(map);

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uhash_assert(uhmap_set(IntHashPi, map, i, i, NULL) == UHASH_INSERTED);
    }

    uint32_t existing_val;
    uhash_assert(uhmap_set(IntHashPi, map, 0, 1, &existing_val) == UHASH_PRESENT);
    uhash_assert(existing_val == 0);

    uhash_assert(uhmap_add(IntHashPi, map, 0, 1, &existing_val) == UHASH_PRESENT);
    uhash_assert(existing_val == 1);

    uhash_assert(uhmap_replace(IntHashPi, map, 0, 0, &existing_val));
    uhash_assert(uhmap_get(IntHashPi, map, 0, UINT32_MAX) == 0);
    uhash_assert(existing_val == 1);

    uhash_assert(uhmap_add(IntHashPi, map, MAX_VAL, MAX_VAL, &existing_val) == UHASH_INSERTED);
    uhash_assert(uhmap_remove(IntHashPi, map, MAX_VAL));

    for (uint32_t i = 0; i < MAX_VAL; ++i) {
        uint32_t existing_key;
        uhash_assert(uhmap_pop(IntHashPi, map, i, &existing_key, &existing_val));
        uhash_assert(existing_key == i);
        uhash_assert(existing_val == i);
    }

    uhash_free(IntHashPi, map);
    return true;
}

int main(void) {
    printf("Starting tests...\n");
    
    int exit_code = EXIT_SUCCESS;
    bool (*tests[])(void) = {
        test_memory,
        test_base,
        test_map,
        test_set,
        test_per_instance
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
