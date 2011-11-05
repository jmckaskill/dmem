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

DVECTOR_INIT(char_vector, d_Vector(char));
DVECTOR_INIT(char_slice, d_Slice(char));

/* ------------------------------------------------------------------------- */

DMEM_INLINE void dv_clear_string_vector(d_Vector(char_vector)* vec)
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

DMEM_INLINE d_Slice(char) dv_char2(const char* str, size_t size)
{ d_Slice(char) ret = {(int) size, (char*) str}; return ret; }

/* Macro to convert char string literals into a d_Slice(char) */
#if __STDC_VERSION__+0 == 199901L
#define C(STR) ((d_Slice(char)) {sizeof(STR)-1, STR})
#define C2(P, LEN) ((d_Slice(char)) {P, LEN})
#else
#define C(STR) dv_char2((char*) STR, sizeof(STR) - 1)
#define C2(P, LEN) dv_char2((char*) P, LEN)
#endif

/* Macro to convert a d_Slice(char) to a std::string */
#define dv_to_string(slice) std::string((slice).data, (slice).data + (slice).size)

/* ------------------------------------------------------------------------- */

/* Appends a base64 encoded version of 'from' to 'to' */
DMEM_API void dv_base64_encode(d_Vector(char)* to, d_Slice(char) from);

/* Appends a hex decodec/encoded version of 'from' to 'to' */
DMEM_API void dv_hex_decode(d_Vector(char)* to, d_Slice(char) from);
DMEM_API void dv_hex_encode(d_Vector(char)* to, d_Slice(char) from);

/* Appends a quoted version of 'from' to 'to' */
DMEM_API void dv_quote(d_Vector(char)* to, d_Slice(char) from, char quote);

/* Appends a url decoded/encoded version of 'from' to 'to' */
DMEM_API void dv_url_decode(d_Vector(char)* to, d_Slice(char) from);
DMEM_API void dv_url_encode(d_Vector(char)* to, d_Slice(char) from);

/* ------------------------------------------------------------------------- */

/* Searches in from for sep (or the bytes in sep). Updating from with the
 * remaining slice after the seperator and returns the slice before the
 * seperator. Will return the whole string if sep can not be found.
 */
DMEM_API d_Slice(char) dv_split(d_Slice(char)* from, int sep);
DMEM_API d_Slice(char) dv_split2(d_Slice(char)* from, d_Slice(char) sep);

/* Splits from on the next newline. Returning the line without line endings,
 * and updates from to the remaining string. Will return the current string if
 * no newline can be found.
 */
DMEM_API d_Slice(char) dv_split_line(d_Slice(char)* from);

/* ------------------------------------------------------------------------- */

/* Appends a pretty printed hex data dump of the data in slice. This can be
 * optionally coloured using inline ansi terminal colour commands.
 */
DMEM_API void dv_hex_dump(d_Vector(char)* s, d_Slice(char) data, bool colors);

/* Logs the data as a hex to stderr with format and the variable arguments
 * specifying the header.
 */
DMEM_API void dv_log(d_Slice(char) data, const char* format, ...);

/* ------------------------------------------------------------------------- */

/* Returns a slice for the null terminated string 'str' */
DMEM_INLINE d_Slice(char) dv_char(const char* str)
{ d_Slice(char) ret = {str ? (int) strlen(str) : 0, (char*) str}; return ret; }

#ifdef __cplusplus
DMEM_INLINE d_Slice(char) dv_char(char* str)
{ d_Slice(char) ret = {str ? (int) strlen(str) : 0, str}; return ret; }

/* Overload for std::string and compatible interfaces */
template <class T>
DMEM_INLINE d_Slice(char) dv_char(const T& str)
{ d_Slice(char) ret = {str.size(), str.c_str()}; return ret; }
#endif

/* ------------------------------------------------------------------------- */

/* Returns the slice left of index from */
DMEM_INLINE d_Slice(char) dv_left(d_Slice(char) str, int from)
{ d_Slice(char) ret = {from, str.data}; return ret; }

/* Returns the slice right of index from */
DMEM_INLINE d_Slice(char) dv_right(d_Slice(char) str, int from)
{ d_Slice(char) ret = {str.size - from, str.data + from}; return ret; }

/* Returns the slice starting at from and size long */
DMEM_INLINE d_Slice(char) dv_slice(d_Slice(char) str, int from, int size)
{ d_Slice(char) ret = {size, str.data + from}; return ret; }

/* ------------------------------------------------------------------------- */

/* Converts the string slice to a number - see strtod for valid values */
DMEM_API double dv_to_number(d_Slice(char) value);
DMEM_API int dv_to_integer(d_Slice(char) value, int radix, int def);

/* Converts the boolean slice to a number - valid true values are "true",
 * "TRUE", "1", etc */
DMEM_API bool dv_to_boolean(d_Slice(char) value);

DMEM_API d_Slice(char) dv_strip_whitespace(d_Slice(char) str);

/* ------------------------------------------------------------------------- */

/* Appends a sprintf formatted string to 's' */
DMEM_API int dv_print(d_Vector(char)* s, const char* format, ...) DMEM_PRINTF(2, 3);
DMEM_API int dv_vprint(d_Vector(char)* s, const char* format, va_list ap) DMEM_PRINTF(2, 0);

/* ------------------------------------------------------------------------- */

/* Searches for the first occurrence of ch in str. Returing str.size if it can
 * not be found */
DMEM_INLINE int dv_find_char(d_Slice(char) str, char ch)
{
    char* p = (char*) dv_memchr(str.data, ch, str.size);
    return p ? p - str.data : str.size;
}

DMEM_INLINE int dv_find_string(d_Slice(char) str, d_Slice(char) val)
{
    char* p = (char*) dv_memmem(str.data, str.size, val.data, val.size);
    return p ? p - str.data : str.size;
}

/* Searches for the last occurrence of ch in str. Returing -1 if it can not be
 * found */
DMEM_INLINE int dv_find_last_char(d_Slice(char) str, char ch)
{
    char* p = (char*) dv_memrchr(str.data, ch, str.size);
    return p ? p - str.data : -1;
}

DMEM_INLINE int dv_find_last_string(d_Slice(char) str, d_Slice(char) val)
{
    char* p = (char*) dv_memrmem(str.data, str.size, val.data, val.size);
    return p ? p - str.data : -1;
}

/* ------------------------------------------------------------------------- */

/* Appends a cleaned version of path to out. */
DMEM_API void dv_clean_path(d_Vector(char)* out, d_Slice(char) path);

/* Appends the relative path component to the existing cleaned path starting
 * at off to the end of the vector. If rel is an absolute path, it replaces
 * the path in out. Finally the resulting path is cleaned.
 */
DMEM_API void dv_join_path(d_Vector(char)* out, int off, d_Slice(char) rel);

