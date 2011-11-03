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

#define DMEM_LIBRARY
#include <dmem/vector.h>
#include <stdlib.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */

void dv_free_base(void* p)
{
    if (p) {
        free((char*) p - 8);
    }
}

/* ------------------------------------------------------------------------- */

void* dv_resize_base(void* p, int newsz)
{
    char* cp = (char*) p;
    size_t alloc = p ? (size_t) ((uint64_t*) cp)[-1] : 0;

    if (newsz > (int) alloc) {
        alloc = (alloc * 2) + 16;

        if (newsz > (int) alloc) {
            alloc = (newsz + 8) & ~7U;
        }

        assert((alloc / 8) * 8 == alloc);

        cp = (char*) realloc(cp ? cp - 8 : NULL, alloc + 16);
        if (cp) {
            *((uint64_t*) cp) = alloc;
            cp += 8;
        }
    }

    if (cp) {
        *((uint64_t*) &cp[newsz]) = 0;
    }

    return cp;
}

/* ------------------------------------------------------------------------- */

#ifndef DV_HAVE_MEMMEM
void* dv_memmem(const void* hay, size_t hlen, const void* needle, size_t nlen)
{
    uint8_t* p = (uint8_t*) hay;
    uint8_t* last_first = p + hlen - nlen;

    if (nlen == 0) {
        return p;
    }

    while (p < last_first) {
        p = (uint8_t*) dv_memchr(p, *(uint8_t*) needle, last_first - p);
        if (!p) {
            return NULL;
        }

        if (memcmp(p, needle, nlen) == 0) {
            return p;
        }

        p++;
    }

    return NULL;
}
#endif

/* ------------------------------------------------------------------------- */

#ifndef DV_HAVE_MEMRMEM
void* dv_memrmem(const void* hay, size_t hlen, const void* needle, size_t nlen)
{
    uint8_t* p = (uint8_t*) hay + hlen;
    uint8_t* last_first = p - nlen;

    if (nlen == 0) {
        return p;
    }

    while (p >= (uint8_t*) hay) {
        p = (uint8_t*) dv_memrchr(p, *(uint8_t*) needle, last_first - p);
        if (!p) {
            return NULL;
        }

        if (memcmp(p, needle, nlen) == 0) {
            return p;
        }

        p--;
    }

    return NULL;
}
#endif

/* ------------------------------------------------------------------------- */

#ifndef DV_HAVE_MEMRCHR
void* dv_memrchr(const void* s, int c, size_t n)
{
    uint8_t* p = (uint8_t*) s;
    while (p >= (uint8_t*) s) {
        if (*p == c) {
            return p;
        }
        p--;
    }
    return NULL;
}
#endif
