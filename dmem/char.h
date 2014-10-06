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

DVECTOR_INIT(char, char);
typedef d_slice(char) d_string;

/* ------------------------------------------------------------------------- */

/* Macro to wrap char slices/vectors for printing with printf - use "%.*s" in
 * the format string.
 */
#define DV_PRI(STR) (STR).size, (STR).data

/* ------------------------------------------------------------------------- */

/* Returns a slice for the null terminated string 'str' */
DMEM_INLINE d_string dv_char(const char* str)
{ d_string ret = {str ? (int) strlen(str) : 0, (char*) str}; return ret; }

#ifdef __cplusplus
DMEM_INLINE d_string dv_char(char* str)
{ d_string ret = {str ? (int) strlen(str) : 0, str}; return ret; }

/* Overload for std::string and compatible interfaces */
template <class T>
DMEM_INLINE d_string dv_char(const T& str)
{ d_string ret = {str.size(), str.c_str()}; return ret; }
#endif

DMEM_INLINE d_string dv_char2(const char* str, size_t size)
{ d_string ret = {(int) size, (char*) str}; return ret; }

#ifdef __cplusplus
DMEM_INLINE d_string dv_char2(char* str, size_t size)
{ d_string ret = {(int) size, str}; return ret; }
#endif

/* Macro to convert char string literals into a d_string */
#if __STDC_VERSION__+0 >= 199901L
#define C(STR) ((d_string) {sizeof(STR)-1, STR})
#define C2(P, LEN) ((d_string) {P, LEN})
#else
#define C(STR) dv_char2(STR, sizeof(STR) - 1)
#define C2(P, LEN) dv_char2(P, LEN)
#endif

/* Macro to convert a d_string to a std::string */
#define dv_to_string(slice) std::string((slice).data, (slice).data + (slice).size)

/* ------------------------------------------------------------------------- */

typedef struct { uint32_t d[8]; } dv_char_mask;

DMEM_API dv_char_mask dv_create_mask(d_string chars);

/* Appends a base64 encoded version of 'from' to 'to' */
DMEM_API void dv_base64_encode(d_vector(char)* to, d_string from);

/* Appends a hex decoded/encoded version of 'from' to 'to' */
DMEM_API void dv_hex_decode(d_vector(char)* to, d_string from);
DMEM_API void dv_hex_encode(d_vector(char)* to, d_string from);

/* Appends a quoted version of 'from' to 'to'. keep is optional giving the
 * mask of characters to keep. */
DMEM_API void dv_quote(d_vector(char)* to, d_string from, dv_char_mask* keep);

/* Appends a url decoded version of 'from' to 'to' */
DMEM_API void dv_url_decode(d_vector(char)* to, d_string from);

/* Appends a url encoded version of 'from' to 'to'. keep is optional giving
 * the mask of characters to keep. */
DMEM_API void dv_url_encode(d_vector(char)* to, d_string from, dv_char_mask* keep);

/* ------------------------------------------------------------------------- */

/* Searches in from for sep (or the bytes in sep). Updating from with the
 * remaining slice after the seperator and returns the slice before the
 * seperator. Will return the whole string if sep can not be found. Multiple
 * seperators in a row will return empty strings.
 */
DMEM_API d_string dv_split_char(d_string* from, int sep);
DMEM_API d_string dv_split_one_of(d_string* from, d_string sep);
DMEM_API d_string dv_split_string(d_string* from, d_string sep);

/* Searches for the first occurrence of ch in str. Returing -1 if it can
 * not be found */
DMEM_API int dv_find_char(d_string str, int ch);
DMEM_API int dv_find_one_of(d_string str, d_string sep);
DMEM_API int dv_find_string(d_string str, d_string val);

/* Searches for the last occurrence of ch in str. Returing -1 if it can not be
 * found */
DMEM_API int dv_find_last_char(d_string str, int ch);
DMEM_API int dv_find_last_one_of(d_string str, d_string sep);
DMEM_API int dv_find_last_string(d_string str, d_string val);

/* Splits from on the next newline. Returning the line without line endings,
 * and updates from to the remaining string. Will return a null slice if no
 * newline can be found (ret.data == NULL).
 */
DMEM_API d_string dv_split_line(d_string* from);

/* Returns a version of str with both leading and trailing whitespace removed
 */
DMEM_API d_string dv_strip_whitespace(d_string str);

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

/* ------------------------------------------------------------------------- */

/* Converts the string slice to a number - see strtod for valid values */
DMEM_API double dv_to_number(d_string value);
DMEM_API int dv_to_integer(d_string value, int radix, int def);

/* ------------------------------------------------------------------------- */

/* Appends a sprintf formatted string to 's' */
DMEM_API int dv_print(d_vector(char)* s, const char* format, ...) DMEM_PRINTF(2, 3);
DMEM_API int dv_vprint(d_vector(char)* s, const char* format, va_list ap) DMEM_PRINTF(2, 0);

/* ------------------------------------------------------------------------- */

/* The two path functions should only be used for unix style paths with /
 * seperators */

/* Appends a cleaned version of path to out. */
DMEM_API void dv_clean_path(d_vector(char)* out, d_string path);

/* Appends the relative path component to the existing cleaned path starting
 * at off to the end of the vector. If rel is an absolute path, it replaces
 * the path in out. Finally the resulting path is cleaned.
 */
DMEM_API void dv_join_path(d_vector(char)* out, int off, d_string rel);

