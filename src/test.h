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

#include <stdlib.h>
#include <stdio.h>
#include "dmem/char.h"

static d_vector(char) g_log;

#define check(test) \
    do { \
        if (test) {\
            dv_print(&g_log, "%s:%d PASS %s\n", __FILE__, __LINE__, #test); \
        } else { \
            fwrite(g_log.data, 1, g_log.size, stderr); \
            fprintf(stderr, "%s:%d: FAIL %s\n", __FILE__, __LINE__, #test); \
            abort(); \
        } \
    } while(0)

#define check_int(u, v) \
    do { \
        int u2 = (int) (u); \
        int v2 = (int) (v); \
        if (u2 == v2) { \
            dv_print(&g_log, "%s:%d PASS %s == %s\n", \
                    __FILE__, __LINE__, #u, #v); \
        } else { \
            fwrite(g_log.data, 1, g_log.size, stderr); \
            fprintf(stderr, "%s:%d: FAIL\n\tHAVE %s = %#x\n\tEXP  %s = %#x\n", \
                    __FILE__, __LINE__, #u, u2, #v, v2); \
            abort(); \
        } \
    } while(0)

#define check_string(u, v) \
    do { \
        d_string u2 = (u); \
        d_string v2 = (v); \
        if (dv_equals(u2, v2)) { \
            dv_print(&g_log, "%s:%d PASS %s == %s\n", \
                    __FILE__, __LINE__, #u, #v); \
        } else { \
            d_vector(char) uq = DV_INIT; \
            d_vector(char) vq = DV_INIT; \
            dv_quote(&uq, u2, NULL); \
            dv_quote(&vq, v2, NULL); \
            fwrite(g_log.data, 1, g_log.size, stderr); \
            fprintf(stderr, "%s:%d: FAIL\n\tHAVE %s = %.*s\n\tEXP  %s = %.*s\n", \
                    __FILE__, __LINE__, #u, DV_PRI(uq), #v, DV_PRI(vq)); \
            abort(); \
        } \
    } while(0)
