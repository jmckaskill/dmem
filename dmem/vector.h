/* vim: ts=4 sw=4 sts=4 et
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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

/* ------------------------------------------------------------------------- */

#ifdef __GLIBC__
#define DV_HAVE_MEMMEM
#define DV_HAVE_MEMRCHR
#elif defined __APPLE__
#define DV_HAVE_MEMMEM
#endif

#ifdef DV_HAVE_MEMMEM
#define dv_memmem(h, hl, n, nl) memmem(h, hl, n, nl)
#else
DMEM_API void* dv_memmem(const void* hay, size_t haylen, const void* needle, size_t needlelen);
#endif

#ifdef DV_HAVE_MEMRCHR
#define dv_memrchr(s, c, n) memrchr(s, c, n)
#else
DMEM_API void* dv_memrchr(const void* s, int c, size_t n);
#endif

/* always available */
#define dv_memchr memchr

DMEM_API void* dv_memrmem(const void* hay, size_t haylen, const void* needle, size_t needlelen);

/* ------------------------------------------------------------------------- */

/* Declares a new vector d_vector(name) and slice d_slice(name) that holds
 * 'type' data values
 */
#define DVECTOR_INIT(name, type)                                            \
    typedef struct d_vector_##name d_vector_##name;                         \
    typedef struct d_vector_##name d_slice_##name;                          \
                                                                            \
    struct d_vector_##name {                                                \
        int size;                                                           \
        type* data;                                                         \
    }

/* Static initializer for vectors 'd_vector(name) foo = DV_INIT;' */
#define DV_INIT {0,NULL}
#define d_vector(name) d_vector_##name
#define d_slice(name) d_slice_##name

/* Declare some basic vectors for the built in types */
DVECTOR_INIT(int, int);

#ifdef __cplusplus
struct d_string {
    int size;
    const char* data;
};
struct d_vector_char {
    int size;
    char* data;
    operator d_string() const {
        d_string ret = {size, data};
        return ret;
    }
};
typedef d_vector_char d_slice_char;
#else
DVECTOR_INIT(char, char);
typedef d_slice(char) d_string;
#endif

/* ------------------------------------------------------------------------- */

#ifdef __cplusplus
template <class T> inline
T* dv_cast(T* p, void* u)
{ (void) p; return (T*) u; }
#else
#define dv_cast(DATA, PTR) (PTR)
#endif

#define dv_datasize(VEC) ((int) sizeof((VEC).data[(VEC).size]))
#define dv_pdatasize(PVEC) ((int) sizeof((PVEC)->data[(PVEC)->size]))

DMEM_API void* dv_resize_base(void* p, int newsz);
DMEM_API void dv_free_base(void* p);
DMEM_API void* dv_append_buffer_base(d_vector(char)* v, int num, int typesz);
DMEM_API void* dv_append_zeroed_base(d_vector(char)* v, int num, int typesz);
DMEM_API void* dv_insert_buffer_base(d_vector(char)* v, int idx, int num, int typesz);
DMEM_API void* dv_insert_zeroed_base(d_vector(char)* v, int idx, int num, int typesz);

/* ------------------------------------------------------------------------- */

/* Frees the data in 'VEC'*/
#define dv_free(VEC) dv_free_base((VEC).data)

/* ------------------------------------------------------------------------- */

/* Resets the vector 'PVEC' to size zero */
#define dv_clear(PVEC) ((PVEC)->size = 0)

/* ------------------------------------------------------------------------- */

/* Reserves enough space in the vector to hold 'newsz' values */
#define dv_reserve(PVEC, NEWSZ) ((PVEC)->data = dv_cast((PVEC)->data, dv_resize_base((PVEC)->data, (NEWSZ) * dv_pdatasize(PVEC))))
#define dv_reserved(VEC) ((VEC).data ? (int) ((uint64_t*) (VEC).data)[-1] : 0)

/* Resizes the vector to hold 'newsz' values */
#define dv_resize(PVEC, NEWSZ) ((PVEC)->size = NEWSZ, dv_reserve(PVEC, (PVEC)->size))

/* ------------------------------------------------------------------------- */

/* Adds space for NUM values at 'IDX' in the vector and returns a pointer to
 * the added space - IDX and NUM are evaluated only once before the resize
 * occurs. The _zeroed form also zeros the added buffer.
 */
#define dv_insert_buffer(PVEC, IDX, NUM) dv_cast((PVEC)->data, dv_insert_buffer_base((d_vector(char)*) (PVEC), IDX, NUM, dv_pdatasize(PVEC)))
#define dv_insert_zeroed(PVEC, IDX, NUM) dv_cast((PVEC)->data, dv_insert_zeroed_base((d_vector(char)*) (PVEC), IDX, NUM, dv_pdatasize(PVEC)))

/* Adds space for NUM values at the end of the vector and returns a pointer to
 * the beginning of the added space - NUM is only evaluated once before the
 * append occurs. The _zeroed form also zeros the added buffer.
 */
#define dv_append_buffer(PVEC, NUM) dv_cast((PVEC)->data, dv_append_buffer_base((d_vector(char)*) (PVEC), NUM, dv_pdatasize(PVEC)))
#define dv_append_zeroed(PVEC, NUM) dv_cast((PVEC)->data, dv_append_zeroed_base((d_vector(char)*) (PVEC), NUM, dv_pdatasize(PVEC)))

/* ------------------------------------------------------------------------- */

/* Appends the data 'DATA' of size 'SZ' to the vector 'PTO'. 'DATA' and 'PTO'
 * must be the same type - DATA and SZ are only evaluated once.
 */
#define dv_append2(PTO, DATA, SZ)                                           \
    do {                                                                    \
        STATIC_ASSERT(sizeof((DATA)[0]) == dv_pdatasize(PTO));              \
        const void* _fromdata = DATA;                                       \
        int _fromsz = SZ;                                                   \
        int _tosz = (PTO)->size;                                            \
        (PTO)->size += _fromsz;                                             \
        dv_reserve(PTO, (PTO)->size);                                       \
        memcpy((PTO)->data + _tosz, _fromdata, _fromsz * dv_pdatasize(PTO));\
    } while(0)

/* ------------------------------------------------------------------------- */

/* Append a single value 'VALUE' to the vector 'PTO'. VALUE is only evaluated
 * once.
 */
#define dv_append1(PTO, VALUE)                                              \
    do {                                                                    \
        dv_reserve(PTO, (PTO)->size + 1);                                   \
        (PTO)->data[(PTO)->size++] = (VALUE);                               \
    } while (0)

/* ------------------------------------------------------------------------- */

/* Appends the vector FROM to PTO - they must be of the same type - FROM is
 * only evaluated once.
 */
#ifdef __cplusplus
template <class T, class F> inline
void dv_append(T* to, F from)
{ dv_append2(to, from.data, from.size); }
#else
#define dv_append(PTO, FROM)                                                \
    do {                                                                    \
        void* _todata, *_fromdata;                                          \
        int _tosz, _fromsz;                                                 \
        _todata = (PTO)->data;                                              \
        _tosz = (PTO)->size;                                                \
        *(PTO) = (FROM); /* only eval FROM once */                          \
        _fromdata = (PTO)->data;                                            \
        _fromsz = (PTO)->size;                                              \
        (PTO)->data = dv_cast((PTO)->data, _todata);                        \
        dv_resize(PTO, _fromsz + _tosz);                                    \
        memcpy((PTO)->data + _tosz, _fromdata, _fromsz * dv_pdatasize(PTO));\
    } while (0)
#endif

/* ------------------------------------------------------------------------- */

/* Sets 'PTO' to be a copy of 'DATA' of size 'SZ' - DATA and SZ are only
 * evaluated once.
 */
#define dv_set2(PTO, DATA, SZ)                                              \
    do {                                                                    \
        STATIC_ASSERT(sizeof((DATA)[0]) == dv_pdatasize(PTO));              \
        dv_resize(PTO, SZ);                                                 \
        memcpy(PTO, DATA, (PTO)->size * dv_pdatasize(PTO));                 \
    } while(0)

/* ------------------------------------------------------------------------- */

/* Sets 'PTO' to be a copy of 'FROM' */
#ifdef __cplusplus
template <class T, class F> inline
void dv_set(T* to, F from)
{ dv_set2(to, from.data, from.size); }
#else
#define dv_set(PTO, FROM)                                                   \
    do {                                                                    \
        void *_todata, *_fromdata;                                          \
        _todata = (PTO)->data;                                              \
        *(PTO) = (FROM); /* only eval FROM once */                          \
        _fromdata = (PTO)->data;                                            \
        (PTO)->data = _todata;                                              \
        dv_reserve(PTO, (PTO)->size);                                       \
        memcpy((PTO)->data, _fromdata, (PTO)->size * dv_pdatasize(PTO));    \
    } while(0)
#endif

/* ------------------------------------------------------------------------- */

/* Inserts a copy of 'DATA' of size 'SZ' at index 'INDEX' in 'PTO' */
#define dv_insert2(PTO, INDEX, DATA, SZ)                                    \
    do {                                                                    \
        STATIC_ASSERT(sizeof((DATA)[0]) == dv_pdatasize(PTO));              \
        const void* _fromdata = (DATA);                                     \
        int _fromsz = (SZ);                                                 \
        void* _todata = dv_insert_buffer(PTO, INDEX, _fromsz);              \
        memcpy(_todata, _fromdata, _fromsz * dv_pdatasize(PTO));            \
    } while(0)

/* ------------------------------------------------------------------------- */

/* Inserts a copy of 'FROM' at index 'INDEX' in 'PTO' */
#ifdef __cplusplus
template <class T, class U> inline
void dv_insert(T* to, int idx, U from)
{ dv_insert2(to, idx, from.data, from.size); }
#else
#define dv_insert(PTO, INDEX, FROM)                                         \
    do {                                                                    \
        void* _todata, *_fromdata;                                          \
        int _tosz, _fromsz;                                                 \
        _todata = (PTO)->data;                                              \
        _tosz = (PTO)->size;                                                \
        *(PTO) = (FROM); /* only eval FROM once */                          \
        _fromdata = (PTO)->data;                                            \
        _fromsz = (PTO)->size;                                              \
        (PTO)->data = dv_cast((PTO)->data, _todata);                        \
        (PTO)->size = _tosz;                                                \
        _todata = dv_insert_buffer(PTO, INDEX, _fromsz);                    \
        memcpy(_todata, _fromdata, _fromsz * dv_pdatasize(PTO));            \
    } while (0)
#endif

/* ------------------------------------------------------------------------- */

/* Erases 'NUM' values beginning with data[INDEX] */
#define dv_erase(PVEC, INDEX, NUM)                                          \
    do {                                                                    \
        int _from = (int) (INDEX);                                          \
        int _num = (int) (NUM);                                             \
        int _to = _from + _num;                                             \
        memmove(&(PVEC)->data[_from], &(PVEC)->data[_to], ((PVEC)->size - _to) * dv_pdatasize(PVEC)); \
        dv_resize(PVEC, (PVEC)->size - _num);                               \
    } while(0)

/* Erases 'NUM' values FROM the end of the vector */
#define dv_erase_end(PVEC, NUM)  dv_resize(PVEC, (PVEC)->size - (NUM))

/* ------------------------------------------------------------------------- */

/* Removes all values that equal 'val' in the vector 'pvec' */
#define dv_remove(pvec, val) dv_remove2(pvec, (pvec)->data[INDEX] == (val))

/* Removes all items for which 'test' resolves to true - use INDEX to
 * reference the current index. eg test could be
 * "should_remove(&myvec.data[INDEX])"
 */
#define dv_remove2(pvec, test)                                              \
    do {                                                                    \
        int INDEX;                                                          \
        for (INDEX = 0; INDEX < (pvec)->size;) {                            \
            if (test) {                                                     \
                dv_erase(pvec, INDEX, 1);                                   \
            } else {                                                        \
                INDEX++;                                                    \
            }                                                               \
        }                                                                   \
    } while (0)                                                             \

/* ------------------------------------------------------------------------- */

/* Sets 'pidx' to the index of the value which equals 'val' in 'vec' or -1 if
 * none is found. 'pidx' should point to an int.
 */
#define dv_find(vec, val, pidx) dv_find2(vec, (vec).data[INDEX] == (val), pidx)

/* Iterates over the vector values until 'test' evaluates to true. 'test'
 * should use the local variable INDEX to reference the index. eg test could
 * be "test_item(&myvec.data[INDEX])". 'pidx' is then set to the found index
 * or -1 if none is found. 'pidx' should point to an int.
 */
#define dv_find2(vec, test, pidx)                                           \
    do {                                                                    \
        int INDEX;                                                          \
        *(pidx) = -1;                                                       \
        for (INDEX = 0; INDEX < (vec).size; INDEX++) {                      \
            if (test) {                                                     \
                *(pidx) = INDEX;                                            \
                break;                                                      \
            }                                                               \
        }                                                                   \
    } while (0)                                                             \

/* ------------------------------------------------------------------------- */

/* Does a memcmp between the data in VEC1 and VEC2. Evaluates VEC1 and VEC2
 * multiple times. */
#define dv_cmp(VEC1, VEC2) ((VEC1).size == (VEC2).size ? memcmp((VEC1).data, (VEC2).data, (VEC1).size * dv_datasize(VEC1)) : (VEC2).size - (VEC1).size)

/* Returns VEC1 == VEC2. Evaluats VEC1 and VEC2 multiple times. */
#define dv_equals(VEC1, VEC2) ((VEC1).size == (VEC2).size && 0 == memcmp((VEC1).data, (VEC2).data, (VEC1).size * dv_datasize(VEC1)))

/* Returns the last value in the vector - note the vector must be non-empty. */
#define dv_last(VEC) ((VEC).data[(VEC).size - 1])

/* Returns whether the char slice/vector 'VEC' begins with the slice 'TEST'.
 * Evaluates VEC and TEST multiple times. */
#define dv_begins_with(VEC, TEST) ((VEC).size > (TEST).size && 0 == memcmp((VEC).data, (TEST).data, (TEST).size * dv_datasize(VEC)))

/* Returns whether the char slice/vector 'VEC' ends with the slice 'TEST'.
 * Evaluates VEC and TEST multiple times. */
#define dv_ends_with(VEC, TEST) ((VEC).size > (TEST).size && 0 == memcmp((VEC).data + (VEC).size - (TEST).size, (TEST).data, (TEST).size * dv_datasize(VEC)))
