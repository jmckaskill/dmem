/* vim: ts=4 sw=4 sts=4 et tw=78
 *
 * Copyright (c) 2009 James R. McKaskill
 *
 * This software is licensed under the stock MIT license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "common.h"
#include "char.h"
#include <stdint.h>

/* ------------------------------------------------------------------------- */

typedef struct d_map d_map;

struct d_map {
    uint32_t n_buckets, size, n_occupied, upper_bound;
    uint32_t *flags;
    size_t idx;
};

/* ------------------------------------------------------------------------- */

DMEM_API void dm_erase_base(d_map* h, size_t idx);
DMEM_API void dm_clear_base(d_map* h);
DMEM_API void dm_free_base(d_map* h);
DMEM_API bool dm_hasnext_base(const d_map* h, int* pidx);

/* ------------------------------------------------------------------------- */

DMEM_API bool dm_i32_get_base(const d_map* h, int32_t key);
DMEM_API bool dm_i64_get_base(const d_map* h, int64_t key);

DMEM_API bool dm_i32_add_base(d_map* h, int32_t key, size_t valsz);
DMEM_API bool dm_i64_add_base(d_map* h, int64_t key, size_t valsz);

/* ------------------------------------------------------------------------- */

DMEM_API bool dm_sget_base(const d_map* h, d_string key);
DMEM_API bool dm_sadd_base(d_map* h, d_string key, size_t valsz);

/* ------------------------------------------------------------------------- */

#define DMAP_INIT_INT(name, khval_t)                                          \
    typedef struct {                                                          \
        d_map base;                                                           \
        int *keys;                                                            \
        khval_t* vals;                                                        \
    } d_imap_##name

#define DMAP_INIT_INT64(name, khval_t)                                        \
    typedef struct {                                                          \
        d_map base;                                                           \
        int64_t *keys;                                                        \
        khval_t* vals;                                                        \
    } d_imap_##name

#define DMAP_INIT_STRING(name, khval_t)                                       \
    typedef struct {                                                          \
        d_map base;                                                           \
        d_string *keys;                                                       \
        khval_t* vals;                                                        \
    } d_smap_##name

#define d_imap(name) d_imap_##name
#define d_smap(name) d_smap_##name

/* ------------------------------------------------------------------------- */

/* Key type agnostic functions */
#define dm_size(h)              ((h)->base.size)
#define dm_clear(h)             dm_clear_base(&(h)->base)
#define dm_free(h)              dm_free_base(&(h)->base)
#define dm_erase(h, idx)        dm_erase_base(&(h)->base, idx)
/* To iterate over the elements in a map:
 * int idx = -1;
 * while (dm_hasnext(&table, &idx)) {
 *  // key is table.keys[idx]
 *  // value is table.vals[idx]
 * }
 */
#define dm_hasnext(h, pidx)     dm_hasnext_base(&(h)->base, pidx)

/* dhi_* are the integer key versions.
 * dhs_* are the string key versions.
 *
 * dh[is]_get: Looks up the key in h and stores the value in *(pval). Returns
 * true if the lookup was successful.
 *
 * dh[is]_find: Looks up the key in h and stores the index in *(pidx). Returns
 * true if the lookup was successful.
 *
 * dh[is]_remove: Removes h[key] from the table. Returns true if it removed a
 * value.
 *
 * dh[is]_add: Adds a slot for the value and returns the index in *pidx.
 * Returns true if a slot was added.
 *
 * dh[is]_set: Sets h[key] = val.
 */

#define dm_iget_base(h, key)    ((sizeof((h)->keys[0]) == sizeof(int64_t)) ? dm_i64_get_base(&(h)->base, (int64_t) (key)) : dm_i32_get_base(&(h)->base, (int32_t) (key)))
#define dm_iadd_base(h, key)    ((sizeof((h)->keys[0]) == sizeof(int64_t)) ? dm_i64_add_base(&(h)->base, (int64_t) (key), sizeof((h)->vals[0])) : dm_i32_add_base(&(h)->base, (int32_t) (key), sizeof((h)->vals[0])))

#define dm_iget(h, key, pval)   (dm_iget_base(h, key) && (*(pval) = (h)->vals[(h)->base.idx], true))
#define dm_ifind(h, key, pidx)  (dm_iget_base(h, key) && (*(pidx) = (h)->base.idx, true))
#define dm_iremove(h, key)      (dm_iget_base(h, key) && (dh_erase_base(&(h)->base, (h)->base.idx), true))
#define dm_iadd(h, key, pidx)   (dm_iadd_base(h, key) ? ((*(pidx) = (h)->base.idx), true) : ((*(pidx) = (h)->base.idx), false))
#define dm_iset(h, key, val)    (dm_iadd_base(h, key), ((h)->vals[(h)->base.idx] = (val)))

#define dm_sget(h, key, pval)   (dm_sget_base(&(h)->base, key) && (*(pval) = (h)->vals[(h)->base.idx], true))
#define dm_sfind(h, key, pidx)  (dm_sget_base(&(h)->base, key) && (*(pidx) = (h)->base.idx, true))
#define dm_sremove(h, key)      (dm_sget_base(&(h)->base, key) && (dh_erase_base(&(h)->base, (h)->base.idx), true))
#define dm_sadd(h, key, pidx)   (dm_sadd_base(&(h)->base, key, sizeof((h)->vals[0])) ? ((*(pidx) = (h)->base.idx), true) : ((*(pidx) = (h)->base.idx), false))
#define dm_sset(h, key, val)    (dm_sadd_base(&(h)->base, key, sizeof((h)->vals[0])), ((h)->vals[(h)->base.idx] = (val)))

