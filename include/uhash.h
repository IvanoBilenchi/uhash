/**
 * uHash - a type-safe, generic C hash table.
 *
 * @see test.c for usage examples.
 * @author Attractive Chaos (khash)
 * @author Ivano Bilenchi (uHash)
 *
 * @copyright Copyright (c) 2008, 2009, 2011 Attractive Chaos <attractor@live.co.uk>
 * @copyright Copyright (c) 2019 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 *
 * @file
 */

#ifndef UHASH_H
#define UHASH_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// #########
// # Types #
// #########

/**
 * A type safe, generic hash table.
 * @struct UHash
 */

/**
 * Unsigned integer type.
 *
 * @public @memberof UHash
 */
#if defined UHASH_TINY
    typedef uint16_t uhash_uint_t;
#elif defined UHASH_HUGE
    typedef uint64_t uhash_uint_t;
#else
    typedef uint32_t uhash_uint_t;
#endif

/**
 * Return codes for functions that add elements to the hash table.
 *
 * @public @memberof UHash
 */
typedef enum uhash_ret_t {

    /// The operation failed (as of right now it can only happen if *alloc returns NULL).
    UHASH_ERROR = -1,

    /// The key is already present.
    UHASH_PRESENT = 0,

    /// The key has been inserted (it was absent).
    UHASH_INSERTED = 1

} uhash_ret_t;

// #############
// # Constants #
// #############

/// Maximum value of a uhash_uint_t variable.
#if defined UHASH_TINY
    #define UHASH_UINT_MAX UINT16_MAX
#elif defined UHASH_HUGE
    #define UHASH_UINT_MAX UINT64_MAX
#else
    #define UHASH_UINT_MAX UINT32_MAX
#endif

/// Index returned when a key is not present in the hash table.
#define UHASH_INDEX_MISSING UHASH_UINT_MAX

/// Hash table maximum load factor.
#ifndef UHASH_MAX_LOAD
    #define UHASH_MAX_LOAD 0.77
#endif

// ###############
// # Private API #
// ###############

/// Cross-platform 'inline' specifier.
#ifndef __uhash_inline
    #ifdef _MSC_VER
        #define __uhash_inline __inline
    #else
        #define __uhash_inline inline
    #endif
#endif

/// Cross-platform 'unused' directive.
#ifndef __uhash_unused
    #if (defined __clang__ && __clang_major__ >= 3) || (defined __GNUC__ && __GNUC__ >= 3)
        #define __uhash_unused __attribute__ ((__unused__))
    #else
        #define __uhash_unused
    #endif
#endif

/// Specifier for static inline definitions.
#define __uhash_static_inline static __uhash_inline __uhash_unused

/// Give hints to the static analyzer.
#if (__clang_analyzer__)
    #define __uhash_analyzer_assert(c) do { if (!(c)) exit(1); } while(0)
#else
    #define __uhash_analyzer_assert(c)
#endif

/// Flags manipulation macros.
#define __uhf_size(m) ((m) < 16 ? 1 : (m) >> 4u)
#define __uhf_isempty(flag, i) ((flag[i >> 4u] >> ((i & 0xfu) << 1u)) & 2u)
#define __uhf_isdel(flag, i) ((flag[i >> 4u] >> ((i & 0xfu) << 1u)) & 1u)
#define __uhf_iseither(flag, i) ((flag[i >> 4u] >> ((i & 0xfu) << 1u)) & 3u)
#define __uhf_set_isdel_false(flag, i) (flag[i >> 4u] &= ~(1ul << ((i & 0xfu) << 1u)))
#define __uhf_set_isempty_false(flag, i) (flag[i >> 4u] &= ~(2ul << ((i & 0xfu) << 1u)))
#define __uhf_set_isboth_false(flag, i) (flag[i >> 4u] &= ~(3ul << ((i & 0xfu) << 1u)))
#define __uhf_set_isdel_true(flag, i) (flag[i >> 4u] |= 1ul << ((i & 0xfu) << 1u))

/**
 * Computes the maximum number of elements that the table can contain
 * before it needs to be resized in order to keep its load factor under UHASH_MAX_LOAD.
 *
 * @param n_buckets [uhash_uint_t] Number of buckets.
 * @return [uhash_uint_t] Upper bound.
 */
#define __uhash_upper_bound(n_buckets) ((uhash_uint_t)((n_buckets) * UHASH_MAX_LOAD + 0.5))

/**
 * Karl Nelson <kenelson@ece.ucdavis.edu>'s X31 string hash function.
 *
 * @param key [char const *] The string to hash.
 * @return [uhash_uint_t] The hash value.
 */
__uhash_static_inline uhash_uint_t __uhash_x31_str_hash(char const *key) {
    uhash_uint_t h = (uhash_uint_t)*key;
    if (h) for (++key ; *key; ++key) h = (h << 5u) - h + (uhash_uint_t)(*key);
    return h;
}

#define __uhash_cast_hash(key) (uhash_uint_t)(key)
#define __uhash_int8_hash(key) __uhash_cast_hash(key)
#define __uhash_int16_hash(key) __uhash_cast_hash(key)

#if defined UHASH_TINY
    #define __uhash_uint_next_power_2(x) (                                                          \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u,                                     \
        ++(x)                                                                                       \
    )
    #define __uhash_int32_hash(key) (uhash_uint_t)((key) >> 17u ^ (key) ^ (key) << 6u)
    #define __uhash_int64_hash(key) (uhash_uint_t)(                                                 \
        (key) >> 49u ^ (key) >> 33u ^ (key) >> 17u ^ (key) ^                                        \
        (key) << 6u ^ (key) << 23u ^ (key) << 39u                                                   \
    )
#elif defined UHASH_HUGE
    #define __uhash_uint_next_power_2(x) (                                                          \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u, (x)|=(x)>>16u, (x)|=(x)>>32u,       \
        ++(x)                                                                                       \
    )
    #define __uhash_int32_hash(key) __uhash_cast_hash(key)
    #define __uhash_int64_hash(key) __uhash_cast_hash(key)
#else
    #define __uhash_uint_next_power_2(x) (                                                          \
        --(x),                                                                                      \
        (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u, (x)|=(x)>>16u,                      \
        ++(x)                                                                                       \
    )
    #define __uhash_int32_hash(key) __uhash_cast_hash(key)
    #define __uhash_int64_hash(key) (uhash_uint_t)((key) >> 33u ^ (key) ^ (key) << 11u)
#endif

/**
 * Defines a new hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param uhkey_t [type] Hash table key type.
 * @param uhval_t [type] Hash table value type.
 */
#define __UHASH_DEF_TYPE(T, uhkey_t, uhval_t)                                                       \
    typedef struct UHash_##T {                                                                      \
        /** @cond */                                                                                \
        uhash_uint_t n_buckets;                                                                     \
        uhash_uint_t n_occupied;                                                                    \
        uhash_uint_t count;                                                                         \
        uint32_t *flags;                                                                            \
        uhkey_t *keys;                                                                              \
        uhval_t *vals;                                                                              \
        /** @endcond */                                                                             \
    } UHash_##T;                                                                                    \
                                                                                                    \
    /** @cond */                                                                                    \
    typedef uhkey_t uhash_##T##_key;                                                                \
    typedef uhval_t uhash_##T##_val;                                                                \
    /** @endcond */

/**
 * Generates function declarations for the specified hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the declarations.
 * @param uhkey_t [type] Hash table key type.
 */
#define __UHASH_DECL(T, SCOPE, uhkey_t)                                                             \
    /** @cond */                                                                                    \
    SCOPE UHash_##T *uhash_alloc_##T(void);                                                         \
    SCOPE void uhash_free_##T(UHash_##T *h);                                                        \
    SCOPE void uhash_clear_##T(UHash_##T *h);                                                       \
    SCOPE uhash_uint_t uhash_get_##T(UHash_##T const *h, uhkey_t key);                              \
    SCOPE bool uhash_resize_##T(UHash_##T *h, uhash_uint_t new_n_buckets);                          \
    SCOPE uhash_uint_t uhash_put_##T(UHash_##T *h, uhkey_t key, uhash_ret_t *ret);                  \
    SCOPE void uhash_delete_##T(UHash_##T *h, uhash_uint_t x);                                      \
    /** @endcond */

/**
 * Generates function declarations for the specified hash map type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the declarations.
 * @param uhkey_t [type] Hash table key type.
 * @param uhval_t [type] Hash table value type.
 */
#define __UHASH_MAP_DECL(T, SCOPE, uhkey_t, uhval_t)                                                \
    /** @cond */                                                                                    \
    SCOPE uhval_t uhmap_get_##T(UHash_##T const *h, uhkey_t key, uhval_t if_missing);               \
    SCOPE uhash_ret_t uhmap_set_##T(UHash_##T *h, uhkey_t key, uhval_t value, uhval_t *existing);   \
    SCOPE uhash_ret_t uhmap_add_##T(UHash_##T *h, uhkey_t key, uhval_t value, uhval_t *existing);   \
    SCOPE bool uhmap_replace_##T(UHash_##T *h, uhkey_t key, uhval_t value, uhval_t *replaced);      \
    SCOPE bool uhmap_remove_##T(UHash_##T *h, uhkey_t key, uhkey_t *r_key, uhval_t *r_val);         \
    /** @endcond */

/**
 * Generates function declarations for the specified hash set type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the declarations.
 * @param uhkey_t [type] Hash table key type.
 */
#define __UHASH_SET_DECL(T, SCOPE, uhkey_t)                                                         \
    /** @cond */                                                                                    \
    SCOPE uhash_ret_t uhset_insert_##T(UHash_##T *h, uhkey_t key, uhkey_t *existing);               \
    SCOPE uhash_ret_t uhset_insert_all_##T(UHash_##T *h, uhkey_t const *items, uhash_uint_t n);     \
    SCOPE bool uhset_replace_##T(UHash_##T *h, uhkey_t key, uhkey_t *replaced);                     \
    SCOPE bool uhset_remove_##T(UHash_##T *h, uhkey_t key, uhkey_t *removed);                       \
    SCOPE bool uhset_is_superset_##T(UHash_##T const *h1, UHash_##T const *h2);                     \
    SCOPE uhash_uint_t uhset_hash_##T(UHash_##T const *h);                                          \
    SCOPE uhkey_t uhset_get_any_##T(UHash_##T const *h, uhkey_t if_empty);                          \
    /** @endcond */

/**
 * Generates function definitions for the specified hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the definitions.
 * @param uhkey_t [type] Hash table key type.
 * @param uhval_t [type] Hash table value type.
 * @param uhash_map [bool] True for map types, false for set types.
 * @param __hash_func [(uhkey_t) -> uhash_uint_t] Hash function.
 * @param __equal_func [(uhkey_t, uhkey_t) -> bool] Equality function.
 */
#define __UHASH_IMPL(T, SCOPE, uhkey_t, uhval_t, uhash_map, __hash_func, __equal_func)              \
                                                                                                    \
    SCOPE UHash_##T *uhash_alloc_##T(void) {                                                        \
        return calloc(1, sizeof(UHash_##T));                                                        \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhash_free_##T(UHash_##T *h) {                                                       \
        if (!h) return;                                                                             \
        free((void *)h->keys);                                                                      \
        free((void *)h->vals);                                                                      \
        free(h->flags);                                                                             \
        free(h);                                                                                    \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhash_clear_##T(UHash_##T *h) {                                                      \
        if (h && h->flags) {                                                                        \
            memset(h->flags, 0xaa, __uhf_size(h->n_buckets) * sizeof(*h->flags));                   \
            h->count = h->n_occupied = 0;                                                           \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_uint_t uhash_get_##T(UHash_##T const *h, uhkey_t key) {                             \
        __uhash_analyzer_assert(h->vals);                                                           \
        if (!h->n_buckets) return UHASH_INDEX_MISSING;                                              \
                                                                                                    \
        uhash_uint_t const mask = h->n_buckets - 1;                                                 \
        uhash_uint_t i = (uhash_uint_t)(__hash_func(key)) & mask;                                   \
        uhash_uint_t step = 0;                                                                      \
        uhash_uint_t const last = i;                                                                \
                                                                                                    \
        while (!__uhf_isempty(h->flags, i) &&                                                       \
               (__uhf_isdel(h->flags, i) || !__equal_func(h->keys[i], key))) {                      \
            i = (i + (++step)) & mask;                                                              \
            if (i == last) return UHASH_INDEX_MISSING;                                              \
        }                                                                                           \
                                                                                                    \
        return __uhf_iseither(h->flags, i) ? UHASH_INDEX_MISSING : i;                               \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhash_resize_##T(UHash_##T *h, uhash_uint_t new_n_buckets) {                         \
        __uhash_analyzer_assert(h->vals);                                                           \
        /* Uses (0.25*n_buckets) bytes instead of [sizeof(key_t+val_t)+.25]*n_buckets. */           \
        uint32_t *new_flags = NULL;                                                                 \
        uhash_uint_t j = 1;                                                                         \
        {                                                                                           \
            __uhash_uint_next_power_2(new_n_buckets);                                               \
            if (new_n_buckets < 4) new_n_buckets = 4;                                               \
                                                                                                    \
            if (h->count >= __uhash_upper_bound(new_n_buckets)) {                                   \
                /* Requested size is too small. */                                                  \
                j = 0;                                                                              \
            } else {                                                                                \
                /* Hash table size needs to be changed (shrink or expand): rehash. */               \
                new_flags = malloc(__uhf_size(new_n_buckets) * sizeof(*new_flags));                 \
                if (!new_flags) return false;                                                       \
                                                                                                    \
                memset(new_flags, 0xaa, __uhf_size(new_n_buckets) * sizeof(*new_flags));            \
                                                                                                    \
                if (h->n_buckets < new_n_buckets) {                                                 \
                    /* Expand. */                                                                   \
                    uhkey_t *new_keys = realloc(h->keys, new_n_buckets * sizeof(*new_keys));        \
                                                                                                    \
                    if (!new_keys) {                                                                \
                        free(new_flags);                                                            \
                        return false;                                                               \
                    }                                                                               \
                                                                                                    \
                    h->keys = new_keys;                                                             \
                                                                                                    \
                    if (uhash_map) {                                                                \
                        uhval_t *new_vals = realloc(h->vals, new_n_buckets * sizeof(*new_vals));    \
                                                                                                    \
                        if (!new_vals) {                                                            \
                            free(new_flags);                                                        \
                            return false;                                                           \
                        }                                                                           \
                                                                                                    \
                        h->vals = new_vals;                                                         \
                    }                                                                               \
                } /* Otherwise shrink. */                                                           \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        if (!j) return true;                                                                        \
                                                                                                    \
        /* Rehashing is needed. */                                                                  \
        for (j = 0; j != h->n_buckets; ++j) {                                                       \
            if (__uhf_iseither(h->flags, j)) continue;                                              \
                                                                                                    \
            uhash_uint_t const new_mask = new_n_buckets - 1;                                        \
            uhkey_t key = h->keys[j];                                                               \
            uhval_t val;                                                                            \
            if (uhash_map) val = h->vals[j];                                                        \
            __uhf_set_isdel_true(h->flags, j);                                                      \
                                                                                                    \
            while (true) {                                                                          \
                /* Kick-out process; sort of like in Cuckoo hashing. */                             \
                uhash_uint_t i = (uhash_uint_t)(__hash_func(key)) & new_mask;                       \
                uhash_uint_t step = 0;                                                              \
                                                                                                    \
                while (!__uhf_isempty(new_flags, i)) i = (i + (++step)) & new_mask;                 \
                __uhf_set_isempty_false(new_flags, i);                                              \
                                                                                                    \
                if (i < h->n_buckets && !__uhf_iseither(h->flags, i)) {                             \
                    /* Kick out the existing element. */                                            \
                    { uhkey_t tmp = h->keys[i]; h->keys[i] = key; key = tmp; }                      \
                    if (uhash_map) { uhval_t tmp = h->vals[i]; h->vals[i] = val; val = tmp; }       \
                    /* Mark it as deleted in the old hash table. */                                 \
                    __uhf_set_isdel_true(h->flags, i);                                              \
                } else {                                                                            \
                    /* Write the element and jump out of the loop. */                               \
                    h->keys[i] = key;                                                               \
                    if (uhash_map) h->vals[i] = val;                                                \
                    break;                                                                          \
                }                                                                                   \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        if (h->n_buckets > new_n_buckets) {                                                         \
            /* Shrink the hash table. */                                                            \
            h->keys = realloc(h->keys, new_n_buckets * sizeof(*h->keys));                           \
            if (uhash_map) h->vals = realloc(h->vals, new_n_buckets * sizeof(*h->vals));            \
        }                                                                                           \
                                                                                                    \
        /* Free the working space. */                                                               \
        free(h->flags);                                                                             \
        h->flags = new_flags;                                                                       \
        h->n_buckets = new_n_buckets;                                                               \
        h->n_occupied = h->count;                                                                   \
                                                                                                    \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_uint_t uhash_put_##T(UHash_##T *h, uhkey_t key, uhash_ret_t *ret) {                 \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_uint_t x;                                                                             \
                                                                                                    \
        if (h->n_occupied >= __uhash_upper_bound(h->n_buckets)) {                                   \
            /* Update the hash table. */                                                            \
            if (h->n_buckets > (h->count << 1u)) {                                                  \
                if (!uhash_resize_##T(h, h->n_buckets - 1)) {                                       \
                    /* Clear "deleted" elements. */                                                 \
                    if (ret) *ret = UHASH_ERROR;                                                    \
                    return UHASH_INDEX_MISSING;                                                     \
                }                                                                                   \
            } else if (!uhash_resize_##T(h, h->n_buckets + 1)) {                                    \
                /* Expand the hash table. */                                                        \
                if (ret) *ret = UHASH_ERROR;                                                        \
                return UHASH_INDEX_MISSING;                                                         \
            }                                                                                       \
        }                                                                                           \
        /* TODO: implement automatic shrinking; resize() already support shrinking. */              \
        {                                                                                           \
            uhash_uint_t const mask = h->n_buckets - 1;                                             \
            uhash_uint_t i = (uhash_uint_t)(__hash_func(key)) & mask;                               \
            uhash_uint_t step = 0;                                                                  \
            uhash_uint_t site = h->n_buckets;                                                       \
            x = site;                                                                               \
                                                                                                    \
            if (__uhf_isempty(h->flags, i)) {                                                       \
                /* Speed up. */                                                                     \
                x = i;                                                                              \
            } else {                                                                                \
                uhash_uint_t const last = i;                                                        \
                                                                                                    \
                while (!__uhf_isempty(h->flags, i) &&                                               \
                       (__uhf_isdel(h->flags, i) || !__equal_func(h->keys[i], key))) {              \
                    if (__uhf_isdel(h->flags, i)) site = i;                                         \
                    i = (i + (++step)) & mask;                                                      \
                                                                                                    \
                    if (i == last) {                                                                \
                        x = site;                                                                   \
                        break;                                                                      \
                    }                                                                               \
                }                                                                                   \
                                                                                                    \
                if (x == h->n_buckets) {                                                            \
                    x = (__uhf_isempty(h->flags, i) && site != h->n_buckets) ? site : i;            \
                }                                                                                   \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        if (__uhf_isempty(h->flags, x)) {                                                           \
            /* Not present at all. */                                                               \
            h->keys[x] = key;                                                                       \
            __uhf_set_isboth_false(h->flags, x);                                                    \
            h->count++;                                                                             \
            h->n_occupied++;                                                                        \
            if (ret) *ret = UHASH_INSERTED;                                                         \
        } else if (__uhf_isdel(h->flags, x)) {                                                      \
            /* Deleted. */                                                                          \
            h->keys[x] = key;                                                                       \
            __uhf_set_isboth_false(h->flags, x);                                                    \
            h->count++;                                                                             \
            if (ret) *ret = UHASH_INSERTED;                                                         \
        } else {                                                                                    \
            /* Don't touch h->keys[x] if present and not deleted. */                                \
            if (ret) *ret = UHASH_PRESENT;                                                          \
        }                                                                                           \
                                                                                                    \
        return x == h->n_buckets ? UHASH_INDEX_MISSING : x;                                         \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhash_delete_##T(UHash_##T *h, uhash_uint_t x) {                                     \
        if (!__uhf_iseither(h->flags, x)) {                                                         \
            __uhf_set_isdel_true(h->flags, x);                                                      \
            h->count--;                                                                             \
        }                                                                                           \
    }

/**
 * Generates function definitions for the specified hash map type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the definitions.
 * @param uhkey_t [type] Hash table key type.
 * @param uhval_t [type] Hash table value type.
 */
#define __UHASH_MAP_IMPL(T, SCOPE, uhkey_t, uhval_t)                                                \
                                                                                                    \
    SCOPE uhval_t uhmap_get_##T(UHash_##T const *h, uhkey_t key, uhval_t if_missing) {              \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_uint_t k = uhash_get_##T(h, key);                                                     \
        return k == UHASH_INDEX_MISSING ? (if_missing) : h->vals[k];                                \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret_t uhmap_set_##T(UHash_##T *h, uhkey_t key, uhval_t value, uhval_t *existing) {  \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_ret_t ret;                                                                            \
        uhash_uint_t k = uhash_put_##T(h, key, &ret);                                               \
                                                                                                    \
        if (ret != UHASH_ERROR) {                                                                   \
            if (ret == UHASH_PRESENT && existing) *existing = h->vals[k];                           \
            h->vals[k] = value;                                                                     \
        }                                                                                           \
                                                                                                    \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret_t uhmap_add_##T(UHash_##T *h, uhkey_t key, uhval_t value, uhval_t *existing) {  \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_ret_t ret;                                                                            \
        uhash_uint_t k = uhash_put_##T(h, key, &ret);                                               \
                                                                                                    \
        if (ret == UHASH_INSERTED) {                                                                \
            h->vals[k] = value;                                                                     \
        } else if (ret == UHASH_PRESENT && existing) {                                              \
            *existing = h->vals[k];                                                                 \
        }                                                                                           \
                                                                                                    \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhmap_replace_##T(UHash_##T *h, uhkey_t key, uhval_t value, uhval_t *replaced) {     \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_uint_t k = uhash_get_##T(h, key);                                                     \
        if (k == UHASH_INDEX_MISSING) return false;                                                 \
        if (replaced) *replaced = h->vals[k];                                                       \
        h->vals[k] = value;                                                                         \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhmap_remove_##T(UHash_##T *h, uhkey_t key, uhkey_t *r_key, uhval_t *r_val) {        \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_uint_t k = uhash_get_##T(h, key);                                                     \
        if (k == UHASH_INDEX_MISSING) return false;                                                 \
        if (r_key) *r_key = h->keys[k];                                                             \
        if (r_val) *r_val = h->vals[k];                                                             \
        uhash_delete_##T(h, k);                                                                     \
        return true;                                                                                \
    }

/**
 * Generates function definitions for the specified hash set type.
 *
 * @param T [symbol] Hash table name.
 * @param SCOPE [scope] Scope of the definitions.
 * @param uhkey_t [type] Hash table key type.
 * @param __hash_func [(uhkey_t) -> uhash_uint_t] Hash function.
 */
#define __UHASH_SET_IMPL(T, SCOPE, uhkey_t, __hash_func)                                            \
                                                                                                    \
    SCOPE uhash_ret_t uhset_insert_##T(UHash_##T *h, uhkey_t key, uhkey_t *existing) {              \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_ret_t ret;                                                                            \
        uhash_uint_t k = uhash_put_##T(h, key, &ret);                                               \
        if (ret == UHASH_PRESENT && existing) *existing = h->keys[k];                               \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_ret_t uhset_insert_all_##T(UHash_##T *h, uhkey_t const *items, uhash_uint_t n) {    \
        __uhash_analyzer_assert(h->vals);                                                           \
        if (!uhash_resize_##T(h, n)) return UHASH_ERROR;                                            \
        uhash_ret_t ret = UHASH_PRESENT;                                                            \
                                                                                                    \
        for (uhash_uint_t i = 0; i < n; ++i) {                                                      \
            uhash_ret_t l_ret = uhset_insert_##T(h, items[i], NULL);                                \
            if (l_ret == UHASH_ERROR) return UHASH_ERROR;                                           \
            if (l_ret == UHASH_INSERTED) ret = UHASH_INSERTED;                                      \
        }                                                                                           \
                                                                                                    \
        return ret;                                                                                 \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhset_replace_##T(UHash_##T *h, uhkey_t key, uhkey_t *replaced) {                    \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_uint_t k = uhash_get_##T(h, key);                                                     \
        if (k == UHASH_INDEX_MISSING) return false;                                                 \
        if (replaced) *replaced = h->keys[k];                                                       \
        h->keys[k] = key;                                                                           \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhset_remove_##T(UHash_##T *h, uhkey_t key, uhkey_t *removed) {                      \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_uint_t k = uhash_get_##T(h, key);                                                     \
        if (k == UHASH_INDEX_MISSING) return false;                                                 \
        if (removed) *removed = h->keys[k];                                                         \
        uhash_delete_##T(h, k);                                                                     \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhset_is_superset_##T(UHash_##T const *h1, UHash_##T const *h2) {                    \
        __uhash_analyzer_assert(h1->vals && h2->vals);                                              \
        for (uhash_uint_t i = 0; i != h2->n_buckets; ++i) {                                         \
            if (uhash_exists(h2, i) && uhash_get_##T(h1, h2->keys[i]) == UHASH_INDEX_MISSING) {     \
                return false;                                                                       \
            }                                                                                       \
        }                                                                                           \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_uint_t uhset_hash_##T(UHash_##T const *h) {                                         \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_uint_t hash = 0;                                                                      \
        for (uhash_uint_t i = 0; i != h->n_buckets; ++i) {                                          \
            if (uhash_exists(h, i)) hash ^= __hash_func(h->keys[i]);                                \
        }                                                                                           \
        return hash;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE uhkey_t uhset_get_any_##T(UHash_##T const *h, uhkey_t if_empty) {                         \
        __uhash_analyzer_assert(h->vals);                                                           \
        uhash_uint_t i = 0;                                                                         \
        while(i != h->n_buckets && !uhash_exists(h, i)) ++i;                                        \
        return i == h->n_buckets ? if_empty : h->keys[i];                                           \
    }

// ##############
// # Public API #
// ##############

/// @name Type definitions

/**
 * Declares a new hash map type.
 *
 * @param T [symbol] Hash table name.
 * @param uhkey_t [symbol] Type of the keys.
 * @param uhval_t [symbol] Type of the values.
 *
 * @public @related UHash
 */
#define UHASH_MAP_DECL(T, uhkey_t, uhval_t)                                                         \
    __UHASH_DEF_TYPE(T, uhkey_t, uhval_t)                                                           \
    __UHASH_DECL(T, __uhash_unused, uhkey_t)                                                        \
    __UHASH_MAP_DECL(T, __uhash_unused, uhkey_t, uhval_t)

/**
 * Declares a new hash map type, prepending a specifier to the generated declarations.
 *
 * @param T [symbol] Hash table name.
 * @param uhkey_t [symbol] Type of the keys.
 * @param uhval_t [symbol] Type of the values.
 * @param SPEC [specifier] Specifier.
 *
 * @public @related UHash
 */
#define UHASH_MAP_DECL_SPEC(T, uhkey_t, uhval_t, SPEC)                                              \
    __UHASH_DEF_TYPE(T, uhkey_t, uhval_t)                                                           \
    __UHASH_DECL(T, SPEC __uhash_unused, uhkey_t)                                                   \
    __UHASH_MAP_DECL(T, SPEC __uhash_unused, uhkey_t, uhval_t)

/**
 * Declares a new hash set type.
 *
 * @param T [symbol] Hash table name.
 * @param uhelem_t [symbol] Type of the elements.
 *
 * @public @related UHash
 */
#define UHASH_SET_DECL(T, uhelem_t)                                                                 \
    __UHASH_DEF_TYPE(T, uhelem_t, char)                                                             \
    __UHASH_DECL(T, __uhash_unused, uhelem_t)                                                       \
    __UHASH_SET_DECL(T, __uhash_unused, uhelem_t)

/**
 * Declares a new hash set type, prepending a specifier to the generated declarations.
 *
 * @param T [symbol] Hash table name.
 * @param uhelem_t [symbol] Type of the elements.
 * @param SPEC [specifier] Specifier.
 *
 * @public @related UHash
 */
#define UHASH_SET_DECL_SPEC(T, uhelem_t, SPEC)                                                      \
    __UHASH_DEF_TYPE(T, uhelem_t, char)                                                             \
    __UHASH_DECL(T, SPEC __uhash_unused, uhelem_t)                                                  \
    __UHASH_SET_DECL(T, SPEC __uhash_unused, uhelem_t)

/**
 * Implements a previously declared hash map type.
 *
 * @param T [symbol] Hash table name.
 * @param uhkey_t [symbol] Type of the keys.
 * @param uhval_t [symbol] Type of the values.
 * @param __hash_func [(uhkey_t) -> uhash_uint_t] Hash function.
 * @param __equal_func [(uhkey_t, uhkey_t) -> bool] Equality function.
 *
 * @public @related UHash
 */
#define UHASH_MAP_IMPL(T, uhkey_t, uhval_t, __hash_func, __equal_func)                              \
    __UHASH_IMPL(T, __uhash_unused, uhkey_t, uhval_t, 1, __hash_func, __equal_func)                 \
    __UHASH_MAP_IMPL(T, __uhash_unused, uhkey_t, uhval_t)

/**
 * Implements a previously declared hash set type.
 *
 * @param T [symbol] Hash table name.
 * @param uhelem_t [symbol] Type of the elements.
 * @param __hash_func [(uhelem_t) -> uhash_uint_t] Hash function.
 * @param __equal_func [(uhelem_t, uhelem_t) -> bool] Equality function.
 *
 * @public @related UHash
 */
#define UHASH_SET_IMPL(T, uhelem_t, __hash_func, __equal_func)                                      \
    __UHASH_IMPL(T, __uhash_unused, uhelem_t, char, 0, __hash_func, __equal_func)                   \
    __UHASH_SET_IMPL(T, __uhash_unused, uhelem_t, __hash_func)

/**
 * Defines a new static hash map type.
 *
 * @param T [symbol] Hash table name.
 * @param uhkey_t [symbol] Type of the keys.
 * @param uhval_t [symbol] Type of the values.
 * @param __hash_func [(uhkey_t) -> uhash_uint_t] Hash function.
 * @param __equal_func [(uhkey_t, uhkey_t) -> bool] Equality function.
 *
 * @public @related UHash
 */
#define UHASH_MAP_INIT(T, uhkey_t, uhval_t, __hash_func, __equal_func)                              \
    __UHASH_DEF_TYPE(T, uhkey_t, uhval_t)                                                           \
    __UHASH_IMPL(T, __uhash_static_inline, uhkey_t, uhval_t, 1, __hash_func, __equal_func)          \
    __UHASH_MAP_IMPL(T, __uhash_static_inline, uhkey_t, uhval_t)

/**
 * Defines a new static hash set type.
 *
 * @param T [symbol] Hash table name.
 * @param uhelem_t [symbol] Type of the elements.
 * @param __hash_func [(uhkey_t) -> uhash_uint_t] Hash function.
 * @param __equal_func [(uhkey_t, uhkey_t) -> bool] Equality function.
 *
 * @public @related UHash
 */
#define UHASH_SET_INIT(T, uhelem_t, __hash_func, __equal_func)                                      \
    __UHASH_DEF_TYPE(T, uhelem_t, char)                                                             \
    __UHASH_IMPL(T, __uhash_static_inline, uhelem_t, char, 1, __hash_func, __equal_func)            \
    __UHASH_SET_IMPL(T, __uhash_static_inline, uhelem_t, __hash_func)

/// @name Hash and equality functions

/**
 * Identity macro.
 *
 * @param a LHS of the identity.
 * @param b RHS of the identity.
 * @return a == b
 *
 * @public @related UHash
 */
#define uhash_identical(a, b) ((a) == (b))

/**
 * Equality function for strings.
 *
 * @param a [char const *] LHS of the equality relation (NULL terminated string).
 * @param b [char const *] RHS of the equality relation (NULL terminated string).
 * @return [bool] True if a is equal to b, false otherwise.
 *
 * @public @related UHash
 */
#define uhash_str_equals(a, b) (strcmp(a, b) == 0)

/**
 * Hash function for 8 bit integers.
 *
 * @param key [int8_t/uint8_t] The integer.
 * @return [uhash_uint_t] The hash value.
 *
 * @public @related UHash
 */
#define uhash_int8_hash(key) __uhash_int8_hash(key)

/**
 * Hash function for 16 bit integers.
 *
 * @param key [int16_t/uint16_t] The integer.
 * @return [uhash_uint_t] The hash value.
 *
 * @public @related UHash
 */
#define uhash_int16_hash(key) __uhash_int16_hash(key)

/**
 * Hash function for 32 bit integers.
 *
 * @param key [int32_t/uint32_t] The integer.
 * @return [uhash_uint_t] The hash value.
 *
 * @public @related UHash
 */
#define uhash_int32_hash(key) __uhash_int32_hash(key)

/**
 * Hash function for 64 bit integers.
 *
 * @param key [int64_t/uint64_t] The integer.
 * @return [uhash_uint_t] The hash value.
 *
 * @public @related UHash
 */
#define uhash_int64_hash(key) __uhash_int64_hash(key)

/**
 * Hash function for strings.
 *
 * @param key [char const *] Pointer to a NULL-terminated string.
 * @return [uhash_uint_t] The hash value.
 *
 * @public @related UHash
 */
#define uhash_str_hash(key) __uhash_x31_str_hash(key)

/**
 * Hash function for pointers.
 *
 * @param key [pointer] The pointer.
 * @return [uhash_uint_t] The hash value.
 *
 * @public @related UHash
 */
#if UINTPTR_MAX <= 0xffffffff
    #define uhash_ptr_hash(key) __uhash_int32_hash((uint32_t)(key))
#else
    #define uhash_ptr_hash(key) __uhash_int64_hash((uint64_t)(key))
#endif

/// @name Declaration

/**
 * Declares a new hash table variable.
 *
 * @param T [symbol] Hash table name.
 *
 * @public @related UHash
 */
#define UHash(T) UHash_##T

/// @name Memory management

/**
 * Allocates a new hash table.
 *
 * @param T [symbol] Hash table name.
 * @return [UHash(T)*] Hash table instance.
 *
 * @public @related UHash
 */
#define uhash_alloc(T) uhash_alloc_##T()

/**
 * Deallocates the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table to deallocate.
 *
 * @public @related UHash
 */
#define uhash_free(T, h) uhash_free_##T(h)

/**
 * Resizes the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table to resize.
 * @param s [uhash_uint_t] Hash table size.
 * @return [bool] True if the operation succeeded, false otherwise.
 *
 * @public @related UHash
 */
#define uhash_resize(T, h, s) uhash_resize_##T(h, s)

/// @name Primitives

/**
 * Inserts a key into the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Key to insert.
 * @param[out] r [uhash_ret_t*] Return code (see uhash_ret_t).
 * @return [uhash_uint_t] Index of the inserted element.
 *
 * @public @related UHash
 */
#define uhash_put(T, h, k, r) uhash_put_##T(h, k, r)

/**
 * Retrieves the index of the bucket associated with the specified key.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Key whose index should be retrieved.
 * @return [uhash_uint_t] Index of the key, or UHASH_INDEX_MISSING if it is absent.
 *
 * @public @related UHash
 */
#define uhash_get(T, h, k) uhash_get_##T(h, k)

/**
 * Deletes the bucket at the specified index.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_uint_t] Index of the bucket to delete.
 *
 * @public @related UHash
 */
#define uhash_delete(T, h, k) uhash_delete_##T(h, k)

/**
 * Checks whether the hash table contains the specified key.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Key to test.
 * @return [bool] True if the hash table contains the specified key, false otherwise.
 *
 * @public @related UHash
 */
#define uhash_contains(T, h, k) (uhash_get_##T(h, k) != UHASH_INDEX_MISSING)

/**
 * Tests whether a bucket contains data.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [uhash_uint_t] Index of the bucket to test.
 * @return [bool] True if the bucket contains data, false otherwise.
 *
 * @public @related UHash
 */
#define uhash_exists(h, x) (!__uhf_iseither((h)->flags, (x)))

/**
 * Retrieves the key at the specified index.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [uhash_uint_t] Index of the bucket whose key should be retrieved.
 * @return [uhash_T_key] Key.
 *
 * @public @related UHash
 */
#define uhash_key(h, x) ((h)->keys[x])

/**
 * Retrieves the value at the specified index.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [uhash_uint_t] Index of the bucket whose value should be retrieved.
 * @return [T value type] Value.
 *
 * @note Results in a segfault for hash sets.
 *
 * @public @related UHash
 */
#define uhash_value(h, x) ((h)->vals[x])

/**
 * Retrieves the start index.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [uhash_uint_t] Start index.
 *
 * @public @related UHash
 */
#define uhash_begin(h) (uhash_uint_t)(0)

/**
 * Retrieves the end index.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [uhash_uint_t] End index.
 *
 * @public @related UHash
 */
#define uhash_end(h) ((h)->n_buckets)

/**
 * Returns the number of elements in the hash table.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [uhash_uint_t] Number of elements.
 *
 * @note For convenience, this macro returns '0' for NULL hash tables.
 *
 * @public @related UHash
 */
#define uhash_count(h) ((h) ? (h)->count : 0)

/**
 * Resets the specified hash table without deallocating it.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 *
 * @public @related UHash
 */
#define uhash_clear(T, h) uhash_clear_##T(h)

/// @name Map-specific API

/**
 * Returns the value associated with the specified key.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param m [uhash_T_val] Value to return if the key is missing.
 * @return [uhash_T_val] Value associated with the specified key.
 *
 * @public @related UHash
 */
#define uhmap_get(T, h, k, m) uhmap_get_##T(h, k, m)

/**
 * Adds a key:value pair to the map, returning the replaced value (if any).
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param v [uhash_T_val] The value.
 * @param[out] e [uhash_T_val*] Existing value, only set if key was already in the map.
 * @return [uhash_ret_t] Return code (see uhash_ret_t).
 *
 * @public @related UHash
 */
#define uhmap_set(T, h, k, v, e) uhmap_set_##T(h, k, v, e)

/**
 * Adds a key:value pair to the map, only if the key is missing.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param v [uhash_T_val] The value.
 * @param[out] e [uhash_T_val*] Existing value, only set if key was already in the map.
 * @return [uhash_ret_t] Return code (see uhash_ret_t).
 *
 * @public @related UHash
 */
#define uhmap_add(T, h, k, v, e) uhmap_add_##T(h, k, v, e)

/**
 * Replaces a value in the map, only if its associated key exists.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param v [uhash_T_val] The value.
 * @param[out] r [uhash_T_val*] Replaced value, only set if the return value is true.
 * @return [bool] True if the value was replaced (its key was present), false otherwise.
 *
 * @public @related UHash
 */
#define uhmap_replace(T, h, k, v, r) uhmap_replace_##T(h, k, v, r)

/**
 * Removes a key:value pair from the map.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @return [bool] True if the key was present (it was deleted), false otherwise.
 *
 * @public @related UHash
 */
#define uhmap_remove(T, h, k) uhmap_remove_##T(h, k, NULL, NULL)

/**
 * Removes a key:value pair from the map, returning the deleted key and value.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] The key.
 * @param[out] dk [uhash_T_key*] Deleted key, only set if key was present in the map.
 * @param[out] dv [uhash_T_val*] Deleted value, only set if key was present in the map.
 * @return [bool] True if the key was present (it was deleted), false otherwise.
 *
 * @public @related UHash
 */
#define uhmap_pop(T, h, k, dk, dv) uhmap_remove_##T(h, k, dk, dv)

/// @name Set-specific API

/**
 * Inserts an element in the set.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to insert.
 * @return [uhash_ret_t] Return code (see uhash_ret_t).
 *
 * @public @related UHash
 */
#define uhset_insert(T, h, k) uhset_insert_##T(h, k, NULL)

/**
 * Inserts an element in the set, returning the existing element if it was already present.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to insert.
 * @param[out] e [uhash_T_key*] Existing element, only set if it was already in the set.
 * @return [uhash_ret_t] Return code (see uhash_ret_t).
 *
 * @public @related UHash
 */
#define uhset_insert_get_existing(T, h, k, e) uhset_insert_##T(h, k, e)

/**
 * Populates the set with elements from an array.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param a [uhash_T_key*] Array of elements.
 * @param n [uhash_uint_t] Size of the array.
 * @return [uhash_ret_t] Return code (see uhash_ret_t).
 *
 * @note This function returns UHASH_INSERTED if at least one element in the array
 *       was missing from the set.
 *
 * @public @related UHash
 */
#define uhset_insert_all(T, h, a, n) uhset_insert_all_##T(h, a, n)

/**
 * Replaces an element in the set, only if it exists.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to replace.
 * @param[out] r [uhash_T_key*] Replaced element, only set if the return value is true.
 * @return [bool] True if the element was replaced (it was present), false otherwise.
 *
 * @public @related UHash
 */
#define uhset_replace(T, h, k, r) uhset_replace_##T(h, k, r)

/**
 * Removes an element from the set.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to remove.
 * @return [bool] True if the element was removed (it was present), false otherwise.
 *
 * @public @related UHash
 */
#define uhset_remove(T, h, k) uhset_remove_##T(h, k, NULL)

/**
 * Removes an element from the set, returning the deleted element.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_T_key] Element to remove.
 * @param[out] d [uhash_T_key*] Deleted element, only set if element was present in the set.
 * @return [bool] True if the element was removed (it was present), false otherwise.
 *
 * @public @related UHash
 */
#define uhset_pop(T, h, k, d) uhset_remove_##T(h, k, d)

/**
 * Checks whether the set is a superset of another set.
 *
 * @param T [symbol] Hash table name.
 * @param h1 [UHash(T)*] Superset.
 * @param h2 [UHash(T)*] Subset.
 * @return [bool] True if the superset relation holds, false otherwise.
 *
 * @public @related UHash
 */
#define uhset_is_superset(T, h1, h2) uhset_is_superset_##T(h1, h2)

/**
 * Checks whether the set is equal to another set.
 *
 * @param T [symbol] Hash table name.
 * @param h1 [UHash(T)*] LHS of the equality relation.
 * @param h2 [UHash(T)*] RHS of the equality relation.
 * @return [bool] True if the equality relation holds, false otherwise.
 *
 * @public @related UHash
 */
#define uhset_equals(T, h1, h2) ((h1)->count == (h2)->count && uhset_is_superset_##T(h1, h2))

/**
 * Computes the hash of the set.
 * The computed hash does not depend on the order of the elements.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @return [uhash_uint_t] Hash of the set.
 *
 * @public @related UHash
 */
#define uhset_hash(T, h) uhset_hash_##T(h)

/**
 * Returns one of the elements in the set.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param m [uhash_T_key] Value returned if the set is empty.
 * @return [uhash_T_key] One of the elements in the set.
 *
 * @public @related UHash
 */
#define uhset_get_any(T, h, m) uhset_get_any_##T(h, m)

/// @name Iteration

/**
 * Iterates over the entries in the hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param key_name [symbol] Name of the variable to which the key will be assigned.
 * @param val_name [symbol] Name of the variable to which the value will be assigned.
 * @param code [code] Code block to execute.
 *
 * @public @related UHash
 */
#define uhash_foreach(T, h, key_name, val_name, code) do {                                          \
    if (h) {                                                                                        \
        uhash_uint_t __n_##key_name = (h)->n_buckets;                                               \
        for (uhash_uint_t __i_##key_name = 0; __i_##key_name != __n_##key_name; ++__i_##key_name) { \
            if (!uhash_exists(h, __i_##key_name)) continue;                                         \
            uhash_##T##_key key_name = (h)->keys[__i_##key_name];                                   \
            uhash_##T##_val val_name = (h)->vals[__i_##key_name];                                   \
            code;                                                                                   \
        }                                                                                           \
    }                                                                                               \
} while(0)

/**
 * Iterates over the keys in the hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param key_name [symbol] Name of the variable to which the key will be assigned.
 * @param code [code] Code block to execute.
 *
 * @public @related UHash
 */
#define uhash_foreach_key(T, h, key_name, code) do {                                                \
    if (h) {                                                                                        \
        uhash_uint_t __n_##key_name = (h)->n_buckets;                                               \
        for (uhash_uint_t __i_##key_name = 0; __i_##key_name != __n_##key_name; ++__i_##key_name) { \
            if (!uhash_exists(h, __i_##key_name)) continue;                                         \
            uhash_##T##_key key_name = (h)->keys[__i_##key_name];                                   \
            code;                                                                                   \
        }                                                                                           \
    }                                                                                               \
} while(0)

/**
 * Iterates over the values in the hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param val_name [symbol] Name of the variable to which the value will be assigned.
 * @param code [code] Code block to execute.
 *
 * @public @related UHash
 */
#define uhash_foreach_value(T, h, val_name, code) do {                                              \
    if (h) {                                                                                        \
        uhash_uint_t __n_##val_name = (h)->n_buckets;                                               \
        for (uhash_uint_t __i_##val_name = 0; __i_##val_name != __n_##val_name; ++__i_##val_name) { \
            if (!uhash_exists(h, __i_##val_name)) continue;                                         \
            uhash_##T##_val val_name = (h)->vals[__i_##val_name];                                   \
            code;                                                                                   \
        }                                                                                           \
    }                                                                                               \
} while(0)

#endif // UHASH_H
