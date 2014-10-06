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

/* needed for NAN */
#define _ISOC99_SOURCE
#define _CRT_SECURE_NO_WARNINGS
#define DMEM_LIBRARY

#include <dmem/char.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <limits.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _MSC_VER
#include <malloc.h>
#define alloca _alloca
#else
#include <alloca.h>
#endif

#if defined _MSC_VER && !defined NAN
#include <ymath.h>
#define NAN _Nan._Double
#endif

#ifndef va_copy
#   ifdef _MSC_VER
#       define va_copy(d,s) d = s
#   elif defined __GNUC__
#       define va_copy(d,s)	__builtin_va_copy(d,s)
#   else
#       error
#   endif
#endif


/* ------------------------------------------------------------------------- */

#define dv_isspace(c) ((c) == '\n' || (c) == '\t' || (c) == ' ' || (c) == '\r')

/* ------------------------------------------------------------------------- */

int dv_vprint(d_vector(char)* v, const char* format, va_list ap)
{
    dv_reserve(v, v->size + 1);

    for (;;) {

        int ret;

        char* buf = (char*) v->data + v->size;
        int bufsz = (int) ((uint64_t*) v->data)[-1] - v->size;

        va_list aq;
        va_copy(aq, ap);

        /* We initialise buf[bufsz] to \0 to detect when snprintf runs out of
         * buffer by seeing whether it overwrites it.
         */
        buf[bufsz] = '\0';
        ret = vsnprintf(buf, bufsz + 1, format, aq);

        if (ret > bufsz) {
            /* snprintf has told us the size of buffer required (ISO C99
             * behavior)
             */
            dv_reserve(v, v->size + ret);

        } else if (ret >= 0) {
            /* success */
            v->size += ret;
            return ret;

        } else if (buf[bufsz] != '\0') {
            /* snprintf has returned an error but has written to the end of the
             * buffer (MSVC behavior). The buffer is not large enough so grow
             * and retry. This can also occur with a format error if it occurs
             * right on the boundary, but then we grow the buffer and can
             * figure out its an error next time around.
             */
            dv_reserve(v, v->size + bufsz + 1);

        } else {
            /* snprintf has returned an error but has not written to the last
             * character in the buffer. We have a format error.
             */
            return -1;
        }
    }
}

int dv_print(d_vector(char)* v, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    return dv_vprint(v, format, ap);
}

/* ------------------------------------------------------------------------- */

#define test(map, val) ((map).d[(val) >> 5] & (1 << ((val) & 31)))
#define set(map, val) (map).d[(val) >> 5] |= 1 << ((val) & 31)

dv_char_mask dv_create_mask(d_string sep)
{
    dv_char_mask mask = {{0, 0, 0, 0, 0, 0, 0, 0}};
    int i;

    for (i = 0; i < sep.size; i++) {
        set(mask, sep.data[i]);
    }

    return mask;
}

/* ------------------------------------------------------------------------- */

static const char g_base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void ToBase64(uint8_t src[3], char dest[4])
{
    /* Input:  xxxx xxxx yyyy yyyy zzzz zzzz
     * Output: 00xx xxxx 00xx yyyy 00yy yyzz 00zz zzzz
     */
    dest[0] = ((src[0] >> 2) & 0x3F);

    dest[1] = ((src[0] << 4) & 0x30)
            | ((src[1] >> 4) & 0x0F);

    dest[2] = ((src[1] << 2) & 0x3C)
            | ((src[2] >> 6) & 0x03);

    dest[3] = (src[2] & 0x3F);

    dest[0] = g_base64[(int) dest[0]];
    dest[1] = g_base64[(int) dest[1]];
    dest[2] = g_base64[(int) dest[2]];
    dest[3] = g_base64[(int) dest[3]];
}

void dv_base64_encode(d_vector(char)* v, d_string from)
{
    uint8_t* up = (uint8_t*) from.data;
    uint8_t* uend = up + from.size;
    char* dest;

    dest = dv_append_buffer(v, ((from.size + 2) / 3) * 4);

    while (up + 3 <= uend) {
        ToBase64(up, dest);
        up += 3;
        dest += 4;
    }

    if (up + 2 == uend) {
        uint8_t pad[3];
        pad[0] = up[0];
        pad[1] = up[1];
        pad[2] = 0;

        ToBase64(pad, dest);
        dest[3] = '=';

    } else if (up + 1 == uend) {
        uint8_t pad[3];
        pad[0] = up[0];
        pad[1] = 0;
        pad[2] = 0;

        ToBase64(pad, dest);
        dest[2] = '=';
        dest[3] = '=';
    }
}

/* -------------------------------------------------------------------------- */

void dv_hex_decode(d_vector(char)* to, d_string from)
{
    int i;
    uint8_t* ufrom = (uint8_t*) from.data;
    uint8_t* dest = (uint8_t*) dv_append_buffer(to, from.size / 2);

    if (from.size % 2 != 0) {
        goto err;
    }

    for (i = 0; i < from.size / 2; ++i) {
        uint8_t hi = ufrom[2*i];
        uint8_t lo = ufrom[2*i + 1];

        if ('0' <= hi && hi <= '9') {
            hi = hi - '0';
        } else if ('a' <= hi && hi <= 'f') {
            hi = hi - 'a' + 10;
        } else if ('A' <= hi && hi <= 'F') {
            hi = hi - 'A' + 10;
        } else {
            goto err;
        }

        if ('0' <= lo && lo <= '9') {
            lo = lo - '0';
        } else if ('a' <= lo && lo <= 'f') {
            lo = lo - 'a' + 10;
        } else if ('A' <= lo && lo <= 'F') {
            lo = lo - 'A' + 10;
        } else {
            goto err;
        }


        *(dest++) = (hi << 4) | lo;
    }

    return;

err:
    dv_erase_end(to, from.size / 2);
}

/* -------------------------------------------------------------------------- */

void dv_hex_encode(d_vector(char)* to, d_string from)
{
    int i;
    uint8_t* ufrom = (uint8_t*) from.data;
    uint8_t* dest = (uint8_t*) dv_append_buffer(to, from.size * 2);

    for (i = 0; i < from.size; ++i) {
        uint8_t hi = ufrom[i] >> 4;
        uint8_t lo = ufrom[i] & 0x0F;

        if (hi < 10) {
            hi += '0';
        } else {
            hi += 'a' - 10;
        }

        if (lo < 10) {
            lo += '0';
        } else {
            lo += 'a' - 10;
        }

        *(dest++) = hi;
        *(dest++) = lo;
    }
}

/* -------------------------------------------------------------------------- */

void dv_url_decode(d_vector(char)* to, d_string from)
{
    const char* b = from.data;
    const char* e = from.data + from.size;
    const char* p = b;

    while (p < e) {
        if (*p == '%') {
            dv_append2(to, b, (int) (p - b));

            /* if we have an incomplete or invalid espace then just ignore it */
            if (p + 3 > e) {
                return;
            }

            dv_hex_decode(to, dv_char2(p + 1, 2));

            p += 3;
            b = p;

        } else if (*p == '+') {
            dv_append2(to, b, (int) (p - b));
            dv_append(to, C(" "));
            p++;
            b = p;

        } else {
            p++;
        }
    }

    dv_append2(to, b, (int) (p - b));
}

/* -------------------------------------------------------------------------- */

/* See test in char_test.c to update */
dv_char_mask dv_url_mask = {{
    0x00000000, 0x03FF6000,
    0x87FFFFFE, 0x47FFFFFE,
    0x00000000, 0x00000000,
    0x00000000, 0x00000000
}};

void dv_url_encode(d_vector(char)* to, d_string from, dv_char_mask* keep)
{
    const unsigned char* b = (unsigned char*) from.data;
    const unsigned char* e = b + from.size;
    const unsigned char* p = b;

    if (keep == NULL) {
        keep = &dv_url_mask;
    }

    while (p < e) {
        if (test(*keep, *p)) {
            p++;
        } else {
            dv_append2(to, b, (int) (p - b));
            dv_print(to, "%%%02X", *(unsigned char*) p);
            p++;
            b = p;
        }
    }

    dv_append2(to, b, (int) (p - b));
}

/* -------------------------------------------------------------------------- */

/* See test in char_test.c to update */
dv_char_mask dv_quote_mask = {{
    0x00000000, 0xFFFFFFEB,
    0xEFFFFFFF, 0x7FFFFFFE,
    0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF
}};

void dv_quote(d_vector(char)* to, d_string s, dv_char_mask* keep)
{
    const unsigned char* b = (unsigned char*) s.data;
    const unsigned char* e = b + s.size;
    const unsigned char* p = b;

    if (keep == NULL) {
        keep = &dv_quote_mask;
    }

    dv_append1(to, '\"');

    while (p < e) {
        if (test(*keep, *p)) {
            p++;
        } else {
            dv_append2(to, b, (int) (p - b));
            switch (*p) {
            case '\n':
                dv_append(to, C("\\n"));
                break;
            case '\r':
                dv_append(to, C("\\r"));
                break;
            case '\t':
                dv_append(to, C("\\t"));
                break;
            case '\'':
                dv_append(to, C("\\'"));
                break;
            case '\"':
                dv_append(to, C("\\\""));
                break;
            case '\\':
                dv_append(to, C("\\\\"));
                break;
            default:
                dv_print(to, "\\x%02X", *p);
                break;
            }
            p++;
            b = p;
        }
    }

    dv_append2(to, b, (int) (p - b));
    dv_append1(to, '\"');
}

/* -------------------------------------------------------------------------- */

double dv_to_number(d_string value)
{
    char* end;
    double ret;
    char* buf = (char*) alloca(value.size + 1);

    if (value.size == 0) return NAN;

    memcpy(buf, value.data, value.size);
    buf[value.size] = '\0';
    ret = strtod(buf, &end);

    if (*end == '\0' || dv_isspace(*end))
      return ret;
    else
      return 0;
}

/* ------------------------------------------------------------------------- */

int dv_to_integer(d_string value, int radix, int defvalue)
{
    char buf[sizeof(int)*CHAR_BIT+2]; /* worst case is binary including the -ve */
    char* p = (char*) value.data;
    char* e = p + value.size;
    int copy, ret;

    while (dv_isspace(*p) && p < e) {
        p++;
    }

    copy = (int) (e - p);

    if (copy < (int) sizeof(buf)-1) {
        copy = (int) sizeof(buf)-1;
    }

    memcpy(buf, p, copy);
    buf[copy] = '\0';

    ret = (int) strtol(buf, &e, radix);

    if (*e && !dv_isspace(*e)) {
        return defvalue;
    }

    return ret;
}

/* ------------------------------------------------------------------------- */

d_string dv_strip_whitespace(d_string s)
{
    char* p = (char*) s.data + s.size;

    while (p >= s.data && dv_isspace(p[-1])) {
        p--;
    }

    s.size = (int) (p - s.data);
    p = s.data;
    while (p < s.data + s.size && dv_isspace(p[0])) {
        p++;
    }

    s.size -= p - s.data;
    s.data = p;
    return s;
}

/* ------------------------------------------------------------------------- */

d_string dv_split_line(d_string* from)
{
    d_string ret = DV_INIT;
    char* p = (char*) memchr(from->data, '\n', from->size);

    if (!p) {
        return ret;
    }

    ret.data = from->data;
    ret.size = (int) (p - from->data);

    if (ret.size && ret.data[ret.size-1] == '\r') {
        ret.size--;
    }

    from->size = (int) (from->data + from->size - (p + 1));
    from->data = p + 1;
    return ret;
}

/* ------------------------------------------------------------------------- */

/* Ported from go path.Clean by go authors
 *
 * Clean returns the shortest path name equivalent to path
 * by purely lexical processing.  It applies the following rules
 * iteratively until no further processing can be done:
 *
 *	1. Replace multiple slashes with a single slash.
 *	2. Eliminate each . path name element (the current directory).
 *	3. Eliminate each inner .. path name element (the parent directory)
 *	   along with the non-.. element that precedes it.
 *	4. Eliminate .. elements that begin a rooted path:
 *	   that is, replace "/.." by "/" at the beginning of a path.
 *
 * If the result of this process is an empty string, Clean
 * returns the string ".".
 *
 * See also Rob Pike, ``Lexical File Names in Plan 9 or
 * Getting Dot-Dot right,''
 * http://plan9.bell-labs.com/sys/doc/lexnames.html
 */
void dv_join_path(d_vector(char)* v, int off, d_string rel)
{
    char *r, *w, *e, *d, *ve;
    bool rooted;

	/* Invariants:
     *  r is the reading point in rel
     *  e is the end of rel
     *  w is where to write the next byte
     *  d is where in v .. must stop, either because
	 *		it is the leading slash or it is a leading ../../.. prefix.
     *	rooted indicates whether the final path in v will be absolute or
     *	relative
     *
     *	The writing part always has a trailing / or is empty
     */

    /* Remove the existing path if rel is absolute or if the existing
     * path is just "." */
    if ((rel.size && rel.data[0] == '/') || (off+1 == v->size && v->data[off] == '.')) {
        dv_resize(v, off);
    }

    w = dv_append_buffer(v, rel.size + 2);
    r = rel.data;
    e = r + rel.size;
    d = &v->data[off];
    rooted = off < v->size && v->data[off] == '/';

    if (r[0] == '/') {
        *(w++) = *(r++);
        rooted = true;
    }

    /* From now on the write buffer should have a trailing '/' or be
     * empty */
    if (w > &v->data[off] && w[-1] != '/') {
        *(w++) = '/';
    }

    if (rooted) {
        d++;
    }

    ve = v->data + v->size;
    while (d+1 < ve && d[0] == '.' && d[1] == '.' && (d+2 == ve || d[2] == '/')) {
        d += 3;
    }

    while (r < e) {
        if (r[0] == '/') {
            /* empty path element */
            r++;
        } else if (r[0] == '.' && (r+1 == e || r[1] == '/')) {
            /* . element */
            r++;
        } else if (r[0] == '.' && r[1] == '.' && (r+2 == e || r[2] == '/')) {
            /* .. element: remove to last / */
            r += 2;
            if (w > d) {
                /* can backtrack */
                w--;
                while (w > d && w[-1] != '/') {
                    w--;
                }
            } else if (!rooted) {
                /* cannot backtrack, but not rooted, so append .. element */
                *(w++) = '.';
                *(w++) = '.';
                *(w++) = '/';
                d = w;
            }
        } else {
			/* copy element */
            while (r < e && *r != '/') {
                *(w++) = *(r++);
			}
            *(w++) = '/';
        }
    }

    if (w == v->data + off) {
        *(w++) = '.';
    } else if (w > v->data + off + 1) {
        w--; /* remove the trailing '/' */
    }

    dv_resize(v, w - v->data);
}

void dv_clean_path(d_vector(char)* v, d_string path)
{ dv_join_path(v, v->size, path); }

/* -------------------------------------------------------------------------- */

void *find_chars(d_string from, d_string sep)
{
    dv_char_mask mask = dv_create_mask(sep);
    uint8_t *u = (uint8_t*) from.data;
    int i;

    for (i = 0; i < from.size; i++) {
        if (test(mask, u[i])) {
            return &u[i];
        }
    }

    return NULL;
}

static void *find_last_chars(d_string from, d_string sep)
{
    dv_char_mask mask = dv_create_mask(sep);
    uint8_t *u = (uint8_t*) from.data;
    int i;

    for (i = from.size - 1; i >= 0; i--) {
        if (test(mask, u[i])) {
            return &u[i];
        }
    }

    return NULL;
}

static d_string do_split(d_string* from, char *p, int sepsz)
{
    d_string ret;

    if (p) {
        ret.data = from->data;
        ret.size = (int) (p - from->data);
        from->data = p + sepsz;
        from->size -= ret.size + sepsz;
    } else {
        ret = *from;
        from->data += from->size;
        from->size = 0;
    }

    return ret;
}

d_string dv_split_char(d_string* from, int sep)
{ return do_split(from, (char*) memchr(from->data, sep, from->size), 1); }

d_string dv_split_one_of(d_string* from, d_string sep)
{ return do_split(from, (char*) find_chars(*from, sep), 1); }

d_string dv_split_string(d_string* from, d_string sep)
{ return do_split(from, (char*) dv_memmem(from->data, from->size, sep.data, sep.size), sep.size); }

/* -------------------------------------------------------------------------- */

int dv_find_char(d_string str, int ch)
{
    char* p = (char*) dv_memchr(str.data, ch, str.size);
    return p ? p - str.data : -1;
}

int dv_find_one_of(d_string str, d_string chars)
{
    char* p = (char*) find_chars(str, chars);
    return p ? p - str.data : -1;
}

int dv_find_last_char(d_string str, int ch)
{
    char* p = (char*) dv_memrchr(str.data, ch, str.size);
    return p ? p - str.data : -1;
}

int dv_find_last_one_of(d_string str, d_string chars)
{
    char* p = (char*) find_last_chars(str, chars);
    return p ? p - str.data : -1;
}

int dv_find_string(d_string str, d_string val)
{
    char* p = (char*) dv_memmem(str.data, str.size, val.data, val.size);
    return p ? p - str.data : -1;
}

int dv_find_last_string(d_string str, d_string val)
{
    char* p = (char*) dv_memrmem(str.data, str.size, val.data, val.size);
    return p ? p - str.data : -1;
}

