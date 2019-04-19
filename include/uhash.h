/** @file
 * uHash - a type-safe, generic C hash table.
 *
 * @see test.c for usage examples.
 * @author Attractive Chaos (khash)
 * @author Ivano Bilenchi (uHash)
 *
 * @copyright Copyright (c) 2008, 2009, 2011 Attractive Chaos <attractor@live.co.uk>
 * @copyright Copyright (c) 2019 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: MIT
 */

#ifndef UHASH_H
#define UHASH_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/// @name Types

/// Unsigned integer type.
#define uhash_uint_t uint32_t

/// Return type for hash functions.
#define uhash_hash_t uint32_t

#define __uhash_flags_t uint32_t
#define __uhash_uint_next_power_2(x) (                                                              \
    --(x),                                                                                          \
    (x)|=(x)>>1u, (x)|=(x)>>2u, (x)|=(x)>>4u, (x)|=(x)>>8u, (x)|=(x)>>16u,                          \
    ++(x)                                                                                           \
)

/// Return code for uhash_put.
typedef enum uhash_ret_t {

    /// The operation failed.
    UHASH_ERROR = -1,

    /// The specified key is already present.
    UHASH_PRESENT = 0,

    /// The key has been inserted.
    UHASH_INSERTED = 1

} uhash_ret_t;

/// @name Constants

static double const __ac_HASH_UPPER = 0.77;

/// @name Private API and Implementation

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

#define __ac_isempty(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&2)
#define __ac_isdel(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&1)
#define __ac_iseither(flag, i) ((flag[i>>4]>>((i&0xfU)<<1))&3)
#define __ac_set_isdel_false(flag, i) (flag[i>>4]&=~(1ul<<((i&0xfU)<<1)))
#define __ac_set_isempty_false(flag, i) (flag[i>>4]&=~(2ul<<((i&0xfU)<<1)))
#define __ac_set_isboth_false(flag, i) (flag[i>>4]&=~(3ul<<((i&0xfU)<<1)))
#define __ac_set_isdel_true(flag, i) (flag[i>>4]|=1ul<<((i&0xfU)<<1))

#define __ac_fsize(m) ((m) < 16? 1 : (m)>>4)

/// @cond

#ifndef kcalloc
    #define kcalloc(N,Z) calloc(N,Z)
#endif

#ifndef kmalloc
    #define kmalloc(Z) malloc(Z)
#endif

#ifndef krealloc
    #define krealloc(P,Z) realloc(P,Z)
#endif

#ifndef kfree
    #define kfree(P) free(P)
#endif

/// @endcond

__uhash_static_inline uhash_hash_t __uhash_x31_str_hash(char const *key) {
    uhash_hash_t h = (uhash_hash_t)*key;
    if (h) for (++key ; *key; ++key) h = (h << 5) - h + (uhash_hash_t)*key;
    return h;
}

#define __UHASH_DEF_TYPE(T, uhkey_t, uhval_t)                                                       \
    typedef uhkey_t uhash_##T##_key;                                                                \
    typedef uhkey_t uhash_##T##_val;                                                                \
    typedef struct UHash_##T {                                                                      \
        uhash_uint_t n_buckets;                                                                     \
        uhash_uint_t count;                                                                         \
        uhash_uint_t n_occupied;                                                                    \
        uhash_uint_t upper_bound;                                                                   \
        __uhash_flags_t *flags;                                                                     \
        uhkey_t *keys;                                                                              \
        uhval_t *vals;                                                                              \
    } UHash_##T;

#define __UHASH_DECL(T, SCOPE, uhkey_t, uhval_t)                                                    \
    SCOPE UHash_##T *uhash_alloc_##T(void);                                                         \
    SCOPE void uhash_free_##T(UHash_##T *h);                                                        \
    SCOPE void uhash_clear_##T(UHash_##T *h);                                                       \
    SCOPE uhash_uint_t uhash_get_##T(const UHash_##T *h, uhkey_t key);                              \
    SCOPE bool uhash_resize_##T(UHash_##T *h, uhash_uint_t new_n_buckets);                          \
    SCOPE uhash_uint_t uhash_put_##T(UHash_##T *h, uhkey_t key, uhash_ret_t *ret);                  \
    SCOPE void uhash_delete_##T(UHash_##T *h, uhash_uint_t x);

#define __UHASH_IMPL(T, SCOPE, uhkey_t, uhval_t, uhash_map, __hash_func, __equal_func)              \
                                                                                                    \
    SCOPE UHash_##T *uhash_alloc_##T(void) {                                                        \
        return kcalloc(1, sizeof(UHash_##T));                                                       \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhash_free_##T(UHash_##T *h) {                                                       \
        if (h) {                                                                                    \
            kfree((void *)h->keys); kfree(h->flags);                                                \
            kfree((void *)h->vals);                                                                 \
            kfree(h);                                                                               \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhash_clear_##T(UHash_##T *h) {                                                      \
        if (h && h->flags) {                                                                        \
            memset(h->flags, 0xaa, __ac_fsize(h->n_buckets) * sizeof(*h->flags));                   \
            h->count = h->n_occupied = 0;                                                           \
        }                                                                                           \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_uint_t uhash_get_##T(const UHash_##T *h, uhkey_t key) {                             \
        if (!h->n_buckets) return 0;                                                                \
                                                                                                    \
        uhash_uint_t k, i, last, mask, step = 0;                                                    \
        mask = h->n_buckets - 1;                                                                    \
        k = __hash_func(key); i = k & mask;                                                         \
        last = i;                                                                                   \
                                                                                                    \
        while (!__ac_isempty(h->flags, i) &&                                                        \
               (__ac_isdel(h->flags, i) || !__equal_func(h->keys[i], key))) {                       \
            i = (i + (++step)) & mask;                                                              \
            if (i == last) return h->n_buckets;                                                     \
        }                                                                                           \
                                                                                                    \
        return __ac_iseither(h->flags, i)? h->n_buckets : i;                                        \
    }                                                                                               \
                                                                                                    \
    SCOPE bool uhash_resize_##T(UHash_##T *h, uhash_uint_t new_n_buckets) {                         \
        /* Uses (0.25*n_buckets) bytes instead of [sizeof(key_t+val_t)+.25]*n_buckets. */           \
        __uhash_flags_t *new_flags = 0;                                                             \
        uhash_uint_t j = 1;                                                                         \
        {                                                                                           \
            __uhash_uint_next_power_2(new_n_buckets);                                               \
            if (new_n_buckets < 4) new_n_buckets = 4;                                               \
                                                                                                    \
            if (h->count >= (uhash_uint_t)(new_n_buckets * __ac_HASH_UPPER + 0.5)) {                \
                /* Requested size is too small. */                                                  \
                j = 0;                                                                              \
            } else {                                                                                \
                /* Hash table size to be changed (shrink or expand): rehash. */                     \
                new_flags = kmalloc(__ac_fsize(new_n_buckets) * sizeof(*new_flags));                \
                if (!new_flags) return false;                                                       \
                                                                                                    \
                memset(new_flags, 0xaa, __ac_fsize(new_n_buckets) * sizeof(*new_flags));            \
                                                                                                    \
                if (h->n_buckets < new_n_buckets) {                                                 \
                    /* Expand. */                                                                   \
                    uhkey_t *new_keys = krealloc(h->keys, new_n_buckets * sizeof(*new_keys));       \
                                                                                                    \
                    if (!new_keys) {                                                                \
                        kfree(new_flags);                                                           \
                        return false;                                                               \
                    }                                                                               \
                                                                                                    \
                    h->keys = new_keys;                                                             \
                                                                                                    \
                    if (uhash_map) {                                                                \
                        uhval_t *new_vals = krealloc(h->vals, new_n_buckets * sizeof(*new_vals));   \
                                                                                                    \
                        if (!new_vals) {                                                            \
                            kfree(new_flags);                                                       \
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
            if (__ac_iseither(h->flags, j) != 0) continue;                                          \
                                                                                                    \
            uhkey_t key = h->keys[j];                                                               \
            uhval_t val;                                                                            \
            uhash_uint_t new_mask;                                                                  \
            new_mask = new_n_buckets - 1;                                                           \
                                                                                                    \
            if (uhash_map) val = h->vals[j];                                                        \
            __ac_set_isdel_true(h->flags, j);                                                       \
                                                                                                    \
            while (true) {                                                                          \
                /* Kick-out process; sort of like in Cuckoo hashing. */                             \
                uhash_uint_t k, i, step = 0;                                                        \
                k = __hash_func(key);                                                               \
                i = k & new_mask;                                                                   \
                                                                                                    \
                while (!__ac_isempty(new_flags, i)) i = (i + (++step)) & new_mask;                  \
                __ac_set_isempty_false(new_flags, i);                                               \
                                                                                                    \
                if (i < h->n_buckets && __ac_iseither(h->flags, i) == 0) {                          \
                    /* Kick out the existing element. */                                            \
                    { uhkey_t tmp = h->keys[i]; h->keys[i] = key; key = tmp; }                      \
                    if (uhash_map) {                                                                \
                        uhval_t tmp = h->vals[i]; h->vals[i] = val; val = tmp;                      \
                    }                                                                               \
                    /* Mark it as deleted in the old hash table. */                                 \
                    __ac_set_isdel_true(h->flags, i);                                               \
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
            h->keys = krealloc(h->keys, new_n_buckets * sizeof(*h->keys));                          \
            if (uhash_map) h->vals = krealloc(h->vals, new_n_buckets * sizeof(*h->vals));           \
        }                                                                                           \
                                                                                                    \
        /* Free the working space. */                                                               \
        kfree(h->flags);                                                                            \
        h->flags = new_flags;                                                                       \
        h->n_buckets = new_n_buckets;                                                               \
        h->n_occupied = h->count;                                                                   \
        h->upper_bound = (uhash_uint_t)(h->n_buckets * __ac_HASH_UPPER + 0.5);                      \
                                                                                                    \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    SCOPE uhash_uint_t uhash_put_##T(UHash_##T *h, uhkey_t key, uhash_ret_t *ret) {                 \
        uhash_uint_t x;                                                                             \
        if (h->n_occupied >= h->upper_bound) {                                                      \
            /* Update the hash table. */                                                            \
            if (h->n_buckets > (h->count<<1)) {                                                     \
                if (uhash_resize_##T(h, h->n_buckets - 1) < 0) {                                    \
                    /* Clear "deleted" elements. */                                                 \
                    *ret = UHASH_ERROR;                                                             \
                    return h->n_buckets;                                                            \
                }                                                                                   \
            } else if (uhash_resize_##T(h, h->n_buckets + 1) < 0) {                                 \
                /* Expand the hash table. */                                                        \
                *ret = UHASH_ERROR;                                                                 \
                return h->n_buckets;                                                                \
            }                                                                                       \
        }                                                                                           \
        /* TODO: implement automatic shrinking; resize() already support shrinking. */              \
        {                                                                                           \
            uhash_uint_t k, i, site, last, mask = h->n_buckets - 1, step = 0;                       \
            x = site = h->n_buckets; k = __hash_func(key); i = k & mask;                            \
                                                                                                    \
            if (__ac_isempty(h->flags, i)) {                                                        \
                /* Speed up. */                                                                     \
                x = i;                                                                              \
            } else {                                                                                \
                last = i;                                                                           \
                                                                                                    \
                while (!__ac_isempty(h->flags, i) &&                                                \
                       (__ac_isdel(h->flags, i) || !__equal_func(h->keys[i], key))) {               \
                    if (__ac_isdel(h->flags, i)) site = i;                                          \
                    i = (i + (++step)) & mask;                                                      \
                                                                                                    \
                    if (i == last) {                                                                \
                        x = site;                                                                   \
                        break;                                                                      \
                    }                                                                               \
                }                                                                                   \
                                                                                                    \
                if (x == h->n_buckets) {                                                            \
                    x = (__ac_isempty(h->flags, i) && site != h->n_buckets) ? site : i;             \
                }                                                                                   \
            }                                                                                       \
        }                                                                                           \
                                                                                                    \
        if (__ac_isempty(h->flags, x)) {                                                            \
            /* Not present at all. */                                                               \
            h->keys[x] = key;                                                                       \
            __ac_set_isboth_false(h->flags, x);                                                     \
            h->count++;                                                                             \
            h->n_occupied++;                                                                        \
            *ret = UHASH_INSERTED;                                                                  \
        } else if (__ac_isdel(h->flags, x)) {                                                       \
            /* Deleted. */                                                                          \
            h->keys[x] = key;                                                                       \
            __ac_set_isboth_false(h->flags, x);                                                     \
            h->count++;                                                                             \
            *ret = UHASH_INSERTED;                                                                  \
        } else {                                                                                    \
            /* Don't touch h->keys[x] if present and not deleted. */                                \
            *ret = UHASH_PRESENT;                                                                   \
        }                                                                                           \
                                                                                                    \
        return x;                                                                                   \
    }                                                                                               \
                                                                                                    \
    SCOPE void uhash_delete_##T(UHash_##T *h, uhash_uint_t x) {                                     \
        if (x != h->n_buckets && !__ac_iseither(h->flags, x)) {                                     \
            __ac_set_isdel_true(h->flags, x);                                                       \
            h->count--;                                                                             \
        }                                                                                           \
    }

/// @name Public API

/// @name Type definitions

/**
 * Declares a new hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param uhkey_t [symbol] Type of the keys.
 * @param uhval_t [symbol] Type of the values.
 */
#define UHASH_DECL(T, uhkey_t, uhval_t)                                                             \
    __UHASH_DEF_TYPE(T, uhkey_t, uhval_t)                                                           \
    __UHASH_DECL(T, uhkey_t, uhval_t)

/**
 * Implements a previously declared hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param uhkey_t [symbol] Type of the keys.
 * @param uhval_t [symbol] Type of the values.
 * @param uhash_map [bool] True for maps, false for sets.
 * @param __hash_func [(uhkey_t) -> uhash_hash_t] Hash function.
 * @param __equal_func [(uhkey_t, uhkey_t) -> bool] Equality function.
 */
#define UHASH_IMPL(T, uhkey_t, uhval_t, uhash_map, __hash_func, __equal_func) \
    __UHASH_IMPL(T, __uhash_unused, uhkey_t, uhval_t, uhash_map, __hash_func, __equal_func)

/**
 * Defines a new static hash table type.
 *
 * @param T [symbol] Hash table name.
 * @param uhkey_t [symbol] Type of the keys.
 * @param uhval_t [symbol] Type of the values.
 * @param uhash_map [bool] True for maps, false for sets.
 * @param __hash_func [(uhkey_t) -> uhash_hash_t] Hash function.
 * @param __equal_func [(uhkey_t, uhkey_t) -> bool] Equality function.
 */
#define UHASH_INIT(T, uhkey_t, uhval_t, uhash_map, __hash_func, __equal_func)                       \
    __UHASH_DEF_TYPE(T, uhkey_t, uhval_t)                                                           \
    __UHASH_IMPL(T, __uhash_static_inline, uhkey_t, uhval_t, uhash_map, __hash_func, __equal_func)

/// @name Hash and equality functions

/**
 * Identity macro.
 *
 * @param a LHS of the identity.
 * @param b RHS of the identity.
 * @return a == b
 */
#define uhash_identical(a, b) ((a) == (b))

/**
 * Equality function for strings.
 *
 * @param a [char const *] LHS of the equality relation (NULL terminated string).
 * @param b [char const *] RHS of the equality relation (NULL terminated string).
 * @return [bool] True if a is equal to b, false otherwise.
 */
#define uhash_str_equals(a, b) (strcmp(a, b) == 0)

/**
 * Hash function for 32 bit integers.
 *
 * @param key [int32_t/uint32_t] The integer.
 * @return [uhash_hash_t] The hash value.
 */
#define uhash_int32_hash(key) (uhash_hash_t)(key)

/**
 * Hash function for 64 bit integers.
 *
 * @param key [int64_t/uint64_t] The integer.
 * @return [uhash_hash_t] The hash value.
 */
#define uhash_int64_hash(key) (uhash_hash_t)((key)>>33^(key)^(key)<<11)

/**
 * Hash function for strings.
 *
 * @param key [char const *] Pointer to a NULL-terminated string.
 * @return [uhash_hash_t] The hash value.
 */
#define uhash_str_hash(key) __uhash_x31_str_hash(key)

/// @name Declaration

/**
 * Declares a new hash table variable.
 *
 * @param T [symbol] Hash table name.
 */
#define UHash(T) UHash_##T

/// @name Memory management

/**
 * Allocates a new hash table.
 *
 * @param T [symbol] Hash table name.
 * @return [UHash(T)*] Hash table instance.
 */
#define uhash_alloc(T) uhash_alloc_##T()

/**
 * Deallocates the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table to deallocate.
 */
#define uhash_free(T, h) uhash_free_##T(h)

/**
 * Resizes the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table to resize.
 * @param s [uhash_uint_t] Hash table size.
 * @return [bool] True if the operation succeeded, false otherwise.
 */
#define uhash_resize(T, h, s) uhash_resize_##T(h, s)

/// @name Primitives

/**
 * Inserts a key into the specified hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [T key type] Key to insert.
 * @param[out] r [uhash_ret_t] Return code.
 * @return [uhash_uint_t] Iterator to the inserted element.
 */
#define uhash_put(T, h, k, r) uhash_put_##T(h, k, r)

/**
 * Retrieves an iterator to the bucket associated with the specified key.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [T key type] Key whose iterator should be retrieved.
 * @return [uhash_uint_t] Iterator to the key, or uhash_end(h) if it is absent.
 */
#define uhash_get(T, h, k) uhash_get_##T(h, k)

/**
 * Deletes the bucket associated with the specified key.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param k [uhash_uint_t] Iterator to the bucket to delete.
 */
#define uhash_delete(T, h, k) uhash_delete_##T(h, k)

/**
 * Tests whether a bucket contains data.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [uhash_uint_t] Iterator to the bucket to test.
 * @return [bool] True if the bucket contains data, false otherwise.
 */
#define uhash_exists(h, x) (!__ac_iseither((h)->flags, (x)))

/**
 * Retrieves a key given an iterator.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [uhash_uint_t] Iterator to the bucket whose key should be retrieved.
 * @return [T key type] Key.
 */
#define uhash_key(h, x) ((h)->keys[x])

/**
 * Retrieves a value given an iterator.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @param x [uhash_uint_t] Iterator to the bucket whose value should be retrieved.
 * @return [T value type] Value.
 *
 * @note Results in a segfault for hash sets.
 */
#define uhash_value(h, x) ((h)->vals[x])

/**
 * Retrieves the start iterator.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [uhash_uint_t] Start iterator.
 */
#define uhash_begin(h) (uhash_uint_t)(0)

/**
 * Retrieves the 'end' iterator.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [uhash_uint_t] End iterator.
 */
#define uhash_end(h) ((h)->n_buckets)

/**
 * Returns the number of elements in the hash table.
 *
 * @param h [UHash(T)*] Hash table instance.
 * @return [uhash_uint_t] Number of elements.
 *
 * @note For convenience, this macro returns '0' for NULL hash tables.
 */
#define uhash_count(h) ((h) ? (h)->count : 0)

/**
 * Resets the specified hash table without deallocating it.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 */
#define uhash_clear(T, h) uhash_clear_##T(h)

/**
 * Iterates over the entries in the hash table.
 *
 * @param T [symbol] Hash table name.
 * @param h [UHash(T)*] Hash table instance.
 * @param key_name [symbol] Name of the variable to which the key will be assigned.
 * @param val_name [symbol] Name of the variable to which the value will be assigned.
 * @param code [code] Code block to execute.
 */
#define uhash_foreach(T, h, key_name, val_name, code) do {                                          \
    if (h) {                                                                                        \
        uhash_uint_t __n_##key_name = uhash_end(h);                                                 \
        for (uhash_uint_t __i_##key_name = 0; __i_##key_name != __n_##key_name; ++__i_##key_name) { \
            if (!uhash_exists(h, __i_##key_name)) continue;                                         \
            uhash_##T##_key key_name = uhash_key(h, __i_##key_name);                                \
            uhash_##T##_val val_name = uhash_val(h, __i_##key_name);                                \
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
 */
#define uhash_foreach_key(T, h, key_name, code) do {                                                \
    if (h) {                                                                                        \
        uhash_uint_t __n_##key_name = uhash_end(h);                                                 \
        for (uhash_uint_t __i_##key_name = 0; __i_##key_name != __n_##key_name; ++__i_##key_name) { \
            if (!uhash_exists(h, __i_##key_name)) continue;                                         \
            uhash_##T##_key key_name = uhash_key(h, __i_##key_name);                                \
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
 */
#define uhash_foreach_value(T, h, val_name, code) do {                                              \
    if (h) {                                                                                        \
        uhash_uint_t __n_##val_name = uhash_end(h);                                                 \
        for (uhash_uint_t __i_##val_name = 0; __i_##val_name != __n_##val_name; ++__i_##val_name) { \
            if (!uhash_exists(h, __i_##val_name)) continue;                                         \
            uhash_##T##_val val_name = uhash_val(h, __i_##val_name);                                \
            code;                                                                                   \
        }                                                                                           \
    }                                                                                               \
} while(0)

#endif // UHASH_H
