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

#include "vector.h"

typedef struct d_kv_pair d_kv_pair;

struct d_kv_pair {
    d_string key;
    d_string val;
};

DVECTOR_INIT(char_vector, d_vector(char));
DVECTOR_INIT(string, d_string);
DVECTOR_INIT(kv_pair, d_kv_pair);

DMEM_INLINE void dkv_append(d_vector(kv_pair)* v, d_string key, d_string val)
{
    d_kv_pair kv = {key, val};
    dv_append1(v, kv);
}

/* ------------------------------------------------------------------------- */

DMEM_INLINE void dv_clear_string_vector(d_vector(char_vector)* vec)
{
    int i;
    for (i = 0; i < vec->size; i++) {
        dv_free(vec->data[i]);
    }
    dv_clear(vec);
}

/* Macro to wrap char slices/vectors for printing with printf - use "%.*s" in
 * the format string.
 */
#define DV_PRI(STR) (STR).size, (STR).data

/* ------------------------------------------------------------------------- */

DMEM_INLINE d_string dv_char2(const char* str, size_t size)
{ d_string ret = {(int) size, (char*) str}; return ret; }

#ifdef __cplusplus
DMEM_INLINE d_slice(char) dv_char2(char* str, size_t size)
{ d_slice(char) ret = {(int) size, str}; return ret; }
#endif

/* Macro to convert char string literals into a d_Slice(char) */
#if __STDC_VERSION__+0 == 199901L
#define C(STR) ((d_string) {sizeof(STR)-1, STR})
#define C2(P, LEN) ((d_string) {P, LEN})
#else
#define C(STR) dv_char2(STR, sizeof(STR) - 1)
#define C2(P, LEN) dv_char2(P, LEN)
#endif

/* Macro to convert a d_slice(char) to a std::string */
#define dv_to_string(slice) std::string((slice).data, (slice).data + (slice).size)

/* ------------------------------------------------------------------------- */

/* Appends a base64 encoded version of 'from' to 'to' */
DMEM_API void dv_base64_encode(d_vector(char)* to, d_string from);

/* Appends a hex decoded/encoded version of 'from' to 'to' */
DMEM_API void dv_hex_decode(d_vector(char)* to, d_string from);
DMEM_API void dv_hex_encode(d_vector(char)* to, d_string from);

/* Appends a quoted version of 'from' to 'to' */
DMEM_API void dv_quote(d_vector(char)* to, d_string from, char quote);

/* URL decodes val in place, updating the new slice bounds. Returns zero on
 * success. */
DMEM_API int dv_url_decode(d_slice(char)* val);

/* Appends a url encoded version of 'from' to 'to' */
DMEM_API void dv_url_encode(d_vector(char)* to, d_string from);

/* ------------------------------------------------------------------------- */

/* Searches in from for sep (or the bytes in sep). Updating from with the
 * remaining slice after the seperator and returns the slice before the
 * seperator. Will return the whole string if sep can not be found.
 */
DMEM_API d_string dv_split(d_string* from, int sep);
DMEM_API d_string dv_split2(d_string* from, d_string sep);

#ifdef __cplusplus
DMEM_INLINE d_slice(char) dv_spit(d_slice(char)* from, int sep)
{ d_string ret = dv_split((d_string*) from, sep); return *(d_slice(char)*) &ret; }

DMEM_INLINE d_slice(char) dv_spit2(d_slice(char)* from, int sep)
{ d_string ret = dv_split2((d_string*) from, sep); return *(d_slice(char)*) &ret; }
#endif


/* Splits from on the next newline. Returning the line without line endings,
 * and updates from to the remaining string. Will return a null slice if no
 * newline can be found (ret.data == NULL).
 */
DMEM_API d_string dv_split_line(d_string* from);

#ifdef __cplusplus
DMEM_INLINE d_slice(char) dv_split_line(d_slice(char)* from)
{ d_string ret = dv_split_line((d_string*) from); return *(d_slice(char)* &ret; }
#endif

/* Appends value to v using the seperator if v is non empty */
DMEM_API void dv_join(d_vector(char)* v, d_string value, int sep);

/* ------------------------------------------------------------------------- */

/* Appends a pretty printed hex data dump of the data in slice. This can be
 * optionally coloured using inline ansi terminal colour commands.
 */
DMEM_API void dv_hex_dump(d_vector(char)* s, d_string data, bool colors);

/* Logs the data as a hex to stderr with format and the variable arguments
 * specifying the header.
 */
DMEM_API void dv_log(d_string data, const char* format, ...);

/* ------------------------------------------------------------------------- */

/* Returns a slice for the null terminated string 'str' */
DMEM_INLINE d_string dv_char(const char* str)
{ d_string ret = {str ? (int) strlen(str) : 0, (char*) str}; return ret; }

#ifdef __cplusplus
DMEM_INLINE d_slice(char) dv_char(char* str)
{ d_slice(char) ret = {str ? (int) strlen(str) : 0, str}; return ret; }

/* Overload for std::string and compatible interfaces */
template <class T>
DMEM_INLINE d_string dv_char(const T& str)
{ d_string ret = {str.size(), str.c_str()}; return ret; }
#endif

/* ------------------------------------------------------------------------- */

/* Returns the slice left of index from */
DMEM_INLINE d_string dv_left(d_string str, int from)
{ d_string ret = {from, str.data}; return ret; }

/* Returns the slice right of index from */
DMEM_INLINE d_string dv_right(d_string str, int from)
{ d_string ret = {str.size - from, str.data + from}; return ret; }

/* Returns the slice starting at from and size long */
DMEM_INLINE d_string dv_slice(d_string str, int from, int size)
{ d_string ret = {size, str.data + from}; return ret; }

#ifdef __cplusplus
DMEM_INLINE d_slice(char) dv_left(d_slice(char) str, int from)
{ d_slice(char) ret = {from, str.data}; return ret; }

DMEM_INLINE d_slice(char) dv_right(d_slice(char) str, int from)
{ d_slice(char) ret = {str.size - from, str.data + from}; return ret; }

DMEM_INLINE d_slice(char) dv_slice(d_slice(char) str, int from, int size)
{ d_slice(char) ret = {size, str.data + from}; return ret; }
#endif

/* ------------------------------------------------------------------------- */

/* Converts the string slice to a number - see strtod for valid values */
DMEM_API double dv_to_number(d_string value);
DMEM_API int dv_to_integer(d_string value, int radix, int def);

/* Converts the boolean slice to a number - valid true values are "true",
 * "TRUE", "1", etc */
DMEM_API bool dv_to_boolean(d_string value);

DMEM_API d_string dv_strip_whitespace(d_string str);
#ifdef __cplusplus
DMEM_INLINE d_slice(char) dv_strip_whitespace(d_slice(char) str)
{ d_string ret = dv_strip_whitespace(d_string(str)); return *(d_slice(char)*) &ret; }
#endif

/* ------------------------------------------------------------------------- */

/* Appends a sprintf formatted string to 's' */
DMEM_API int dv_print(d_vector(char)* s, const char* format, ...) DMEM_PRINTF(2, 3);
DMEM_API int dv_vprint(d_vector(char)* s, const char* format, va_list ap) DMEM_PRINTF(2, 0);

/* ------------------------------------------------------------------------- */

/* Searches for the first occurrence of ch in str. Returing -1 if it can
 * not be found */
DMEM_INLINE int dv_find_char(d_string str, char ch)
{
    char* p = (char*) dv_memchr(str.data, ch, str.size);
    return p ? (int) (p - str.data) : -1;
}

DMEM_INLINE int dv_find_string(d_string str, d_string val)
{
    char* p = (char*) dv_memmem(str.data, str.size, val.data, val.size);
    return p ? (int) (p - str.data) : -1;
}

/* Searches for the last occurrence of ch in str. Returing -1 if it can not be
 * found */
DMEM_INLINE int dv_find_last_char(d_string str, char ch)
{
    char* p = (char*) dv_memrchr(str.data, ch, str.size);
    return p ? (int) (p - str.data) : -1;
}

DMEM_INLINE int dv_find_last_string(d_string str, d_string val)
{
    char* p = (char*) dv_memrmem(str.data, str.size, val.data, val.size);
    return p ? (int) (p - str.data) : -1;
}

/* ------------------------------------------------------------------------- */

/* Appends a cleaned version of path to out. */
DMEM_API void dv_clean_path(d_vector(char)* out, d_string path);

/* Appends the relative path component to the existing cleaned path starting
 * at off to the end of the vector. If rel is an absolute path, it replaces
 * the path in out. Finally the resulting path is cleaned.
 */
DMEM_API void dv_join_path(d_vector(char)* out, int off, d_string rel);

