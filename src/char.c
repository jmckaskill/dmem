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

void dv_hex_decoded(d_vector(char)* to, d_string from)
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

void dv_url_decoded(d_vector(char)* to, d_string from)
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

            dv_hex_decoded(to, dv_char2(p + 1, 2));

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

void dv_url_encode(d_vector(char)* to, d_string from)
{
    const char* b = from.data;
    const char* e = b + from.size;
    const char* p = b;

    while (p < e) {
        if (    ('a' <= *p && *p <= 'z')
             || ('A' <= *p && *p <= 'Z')
             || ('0' <= *p && *p <= '9')
             || *p == '-'
             || *p == '_'
             || *p == '~'
             || *p == '.') {
            p++;
        } else {
            dv_append2(to, b, (int) (p - b));
            dv_print(to, "%%%02x", *(unsigned char*) p);
            p++;
            b = p;
        }
    }

    dv_append2(to, b, (int) (p - b));
}

/* -------------------------------------------------------------------------- */

#ifndef min
#   define min(x, y) ((x < y) ? (x) : (y))
#endif

#define dv_isprint(ch) (' ' < ch && ch <= '~')

#define BYTES_PER_LINE ((24 /*header*/ + 8*20 /*hex*/ + 4*4/*hex spaces*/ + 6*16/*ascii*/ + 3/*ascii spaces*/ + 3/*round up for spaces and \n*/) & ~3)

static const char lookup_hex[] = "0123456789abcdef";
#define hex(p, v) ((p)[0] = lookup_hex[((v) & 0xFF) >> 4], (p)[1] = lookup_hex[v & 0x0F], (p) + 2)

#if __LITTLE_ENDIAN__ || __ARMEL__ || defined __i386__ || defined __amd64__ || defined _M_IX86 || defined _M_X64
#define u32(p, h, mh, ml, l) (*((uint32_t*) p) = ((uint32_t) (h)) | ((uint32_t) (mh) << 8) | ((uint32_t) (ml) << 16) | ((uint32_t) (l) << 24), (p) + 4)
#elif __BIG_ENDIAN__ || __ARMEB__
#define u32(p, h, mh, ml, l) (*((uint32_t*) p) = ((uint32_t) (h) << 24) | ((uint32_t) (mh) << 16) | ((uint32_t) (ml) << 8) | ((uint32_t) (l)), (p) + 4)
#else
#error
#endif

#define NORMAL 0
#define RED 1
#define YELLOW 2
#define CYAN 3

static char* set_normal(char* p, int* color)
{
    if (*color != NORMAL) {
        *color = NORMAL;
        p = u32(p, '\033', '[', '0', 'm');
    }
    return p;
}

static char* set_red(char* p, int* color)
{
    if (*color != RED) {
        *color = RED;
        p = u32(p, '\033', '[', 'm', '\033');
        p = u32(p, '[', '3', '1', 'm');
    }
    return p;
}

static char* do_line(char* p, int off, const char* d, int n, int* color)
{
    uint8_t* u = (uint8_t*) d;
    int i;

    /* 8 bytes */
    if (color) {
        *color = CYAN;
        p = u32(p, '\t', '\033', '[', '3');
        p = u32(p, '6', 'm', '0', 'x');
    } else {
        p = u32(p, ' ', '\t', '0', 'x');
    }

    /* 8 bytes */
    p = hex(p, off >> 8);
    p = hex(p, off);

    /* 8 bytes */
    p = set_normal(p, color);
    p = u32(p, ' ', ' ', ' ', ' ');

    /* up to 20 bytes per loop */
    for (i = 0; i+2 <= n; i+= 2) {
        if ((i & 3) == 0 && i > 0) {
            p = u32(p, ' ', '\b', ' ', ' ');
        }

        if (!color) {
            /* abcd - 4 bytes */
            p = hex(p, u[i]);
            p = hex(p, u[i+1]);

        } else if (dv_isprint(d[i]) && dv_isprint(d[i+1])) {
            /* NORMAL abcd - 8 bytes */
            p = set_normal(p, color);
            p = hex(p, u[i]);
            p = hex(p, u[i+1]);

        } else if (!dv_isprint(d[i]) && !dv_isprint(d[i+1])) {
            /* RED abcd - 12 bytes */
            p = set_red(p, color);
            p = hex(p, u[i]);
            p = hex(p, u[i+1]);

        } else if (dv_isprint(u[i])) {
            /* NORMAL ab RED cd - 20 bytes */
            p = set_normal(p, color);
            p = hex(p, u[i]);
            *(p++) = ' ';
            *(p++) = '\b';
            p = set_red(p, color);
            p = hex(p, u[i+1]);
            *(p++) = ' ';
            *(p++) = '\b';

        } else {
            /* RED ab NORMAL cd - 20 bytes */
            p = set_red(p, color);
            p = hex(p, u[i]);
            *(p++) = ' ';
            *(p++) = '\b';
            p = set_normal(p, color);
            p = hex(p, u[i+1]);
            *(p++) = ' ';
            *(p++) = '\b';
        }
    }

    /* Pad out to an even number of bytes - not used in worst case */
    if (n & 1) {
        if ((i & 3) == 0) {
            p = u32(p, ' ', '\b', ' ', ' ');
        }

        if (color && dv_isprint(d[i])) {
            p = set_normal(p, color);
        } else if (color) {
            p = set_red(p, color);
        }

        p = hex(p, u[i]);
        *(p++) = ' ';
        *(p++) = ' ';
        i += 2;
    }

    /* Pad out to 16 bytes - not used in worst case */
    for (; i < 16; i += 2) {
        if ((i & 3) == 0) {
            p = u32(p, ' ', '\b', ' ', ' ');
        }
        p = u32(p, ' ', ' ', ' ', ' ');
    }

    p = u32(p, ' ', ' ', ' ', ' ');

    /* up to 6 bytes per loop */
    for (i = 0; i < n; i++) {
        if ((i & 3) == 0 && i > 0) {
            *(p++) = ' ';
        }

        if (!color) {
            *(p++) = dv_isprint(d[i]) ? d[i] : '.';

        } else if (dv_isprint(d[i])) {
            if (*color != NORMAL) {
                *color = NORMAL;
                *(p++) = '\033';
                *(p++) = '[';
                *(p++) = 'm';
            }
            *(p++) = d[i];

        } else if (d[i] == '\0' || d[i] == '\n') {
            if (*color != YELLOW) {
                *color = YELLOW;
                *(p++) = '\033';
                *(p++) = '[';
                *(p++) = '3';
                *(p++) = '3';
                *(p++) = 'm';
            }
            *(p++) = d[i] ? 'N' : '0';

        } else {
            if (*color != RED) {
                *color = RED;
                *(p++) = '\033';
                *(p++) = '[';
                *(p++) = '3';
                *(p++) = '1';
                *(p++) = 'm';
            }
            *(p++) = '.';
        }
    }

    /* realign up to 4 bytes */
    switch ((uintptr_t) p & 3) {
    case 3:
        *(p++) = '\n';
        break;

    case 2:
        *(p++) = ' ';
        *(p++) = '\n';
        break;

    case 1:
        *(p++) = ' ';
        *(p++) = '\b';
        *(p++) = '\n';
        break;

    case 0:
    default:
        if (color) {
            *color = NORMAL;
            p = u32(p, '\033', '[', 'm', '\n');
        } else {
            p = u32(p, ' ', '\b', ' ', '\n');
        }
        break;
    }

    return set_normal(p, color);
}

void dv_hex_dump(d_vector(char)* s, d_string data, bool colors)
{
    int i, color;

    if (!data.size) {
        return;
    }

    /* align to a 4 byte boundary with spaces that are consumed by the first
     * tab */
    switch (s->size & 3) {
    case 1:
        dv_append1(s, ' ');
    case 2:
        dv_append1(s, ' ');
    case 3:
        dv_append1(s, ' ');
    default:
        break;
    }

    for (i = 0; i < data.size; i += 16) {
        int n = min(16, data.size - i);
        char* p = dv_reserve(s, BYTES_PER_LINE);
        char* e = do_line(p, i, &data.data[i], n, colors ? &color : NULL);
        assert(e - p <= BYTES_PER_LINE);
        dv_resize(s, (int) (e - data.data));
    }
}

void dv_log(d_string data, const char* format, ...)
{
    /* use a uint32_t to ensure the buffer is 4 byte aligned */
    uint32_t buf[BYTES_PER_LINE/4];
    int i, color;
    va_list ap;
    static int colored = -1;

    if (colored == -1) {
#ifdef _WIN32
        colored = 0;
#else
        colored = isatty(fileno(stderr));
#endif
    }

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fputc('\n', stderr);

    for (i = 0; i < data.size; i += 16) {
        int n = min(16, data.size - i);
        char* e = do_line((char*)&buf[0], i, &data.data[i], n, colored ? &color : NULL);
        assert(e - (char*)&buf[0] <= BYTES_PER_LINE);
        fwrite(buf, 1, e - (char*)&buf[0], stderr);
    }
}

/* ------------------------------------------------------------------------- */

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

    copy = min((int) sizeof(buf)-1, (int) (e - p));
    memcpy(buf, p, copy);
    buf[copy] = '\0';

    ret = (int) strtol(buf, &e, radix);

    if (*e && !dv_isspace(*e)) {
        return defvalue;
    }

    return ret;
}

/* ------------------------------------------------------------------------- */

bool dv_to_boolean(d_string value)
{
    if (dv_equals(value, C("TRUE"))) {
        return true;
    } else if (dv_equals(value, C("true"))) {
        return true;
    } else if (dv_equals(value, C("1"))) {
        return true;
    } else {
        return false;
    }
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

d_string dv_split(d_string* from, int sep)
{
    d_string ret;
    char* p = (char*) memchr(from->data, sep, from->size);

    if (p) {
        ret.data = from->data;
        ret.size = (int) (p - from->data);
        from->data = p + 1;
        from->size -= ret.size + 1;
    } else {
        ret = *from;
        from->data += from->size;
        from->size = 0;
    }

    return ret;
}

/* -------------------------------------------------------------------------- */

#define test(map, val) (map[(val) >> 5] & (1 << ((val) & 31)))
#define set(map, val) map[(val) >> 5] |= 1 << ((val) & 31)

d_string dv_split2(d_string* from, d_string sep)
{
    uint32_t map[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t *u = (uint8_t*) from->data;
    d_string ret;
    int i;

    for (i = 0; i < sep.size; i++) {
        set(map, sep.data[i]);
    }

    for (i = 0; i < from->size; i++) {
        if (test(map, u[i])) {
            ret = dv_left(*from, i);
            *from = dv_right(*from, i + 1);
            return ret;
        }
    }

    ret = *from;
    from->data += from->size;
    from->size = 0;

    return ret;
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
static int clean_path(char* buf, int existing, d_string pstr)
{
    int r, w, dotdot, n;
    bool rooted;
    const char *path;

    if (!pstr.size) {
        return 0;
    }

    path = pstr.data;
    n = pstr.size;
    rooted = existing ? buf[0] == '/' : path[0] == '/';

	/* Invariants:
	 *	reading from path; r is index of next byte to process.
	 *	writing to buf; w is index of next byte to write.
	 *	dotdot is index in p where .. must stop, either because
	 *		it is the leading slash or it is a leading ../../.. prefix.
     */
    r = 0;
    w = existing;
    dotdot = 0;
	if (rooted) {
        if (w == 0) {
            w++;
        }
        r++;
        dotdot++;
	}

    while (r < n) {
        if (path[r] == '/') {
            /* empty path element */
            r++;
        } else if (path[r] == '.' && (r+1 == n || path[r+1] == '/')) {
            /* . element */
            r++;
        } else if (path[r] == '.' && path[r+1] == '.' && (r+2 == n || path[r+2] == '/')) {
            /* .. element: remove to last / */
            r += 2;
            if (w > dotdot) {
                /* can backtrack */
                w--;
                while (w > dotdot && buf[w] != '/') {
                    w--;
                }
            } else if (!rooted) {
                /* cannot backtrack, but not rooted, so append .. element */
                if (w > 0) {
                    buf[w++] = '/';
                }
                buf[w++] = '.';
                buf[w++] = '.';
                dotdot = w;
            }
        } else {
			/* real path element.
			 * add slash if needed
             */
			if ((rooted && w != 1) || (!rooted && w != 0)) {
                buf[w++] = '/';
			}
			/* copy element */
            for (; r < n && path[r] != '/'; r++) {
                buf[w++] = path[r];
			}
        }
    }

    return w;
}

void dv_join_path(d_vector(char)* v, int off, d_string rel)
{
    int ret;

    if (!rel.size) {
        return;
    }

    /* Remove the existing path if rel is absolute */
    if (rel.size && rel.data[0] == '/') {
        dv_resize(v, off);
    }

    /* Append a path seperator */
    if (v->size > off) {
        dv_append1(v, '/');
    }

    ret = clean_path(v->data + off, v->size - off, rel);
    dv_resize(v, off + ret);

    if (ret == 0) {
        dv_append(v, C("."));
    }
}

void dv_clean_path(d_vector(char)* v, d_string path)
{ dv_join_path(v, v->size, path); }

