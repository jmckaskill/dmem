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

#ifndef _WIN32
#include_next <stdint.h>
#else

#include <limits.h>

#if UCHAR_MAX == 0xff
    typedef signed char int8_t;
    typedef unsigned char uint8_t;
#else
#   error
#endif

#if USHRT_MAX == 0xffff
    typedef signed short int16_t;
    typedef unsigned short uint16_t;
#else
#   error
#endif

#if UINT_MAX == 0xffffffffUL
    typedef signed int int32_t;
    typedef unsigned int uint32_t;
#else
#   error
#endif

#if ULLONG_MAX == 0xffffffffffffffffui64
    typedef signed long long intmax_t;
    typedef unsigned long long uintmax_t;
    typedef signed long long int64_t;
    typedef unsigned long long uint64_t;
#else
#   error
#endif



#if defined(__STDC_CONSTANT_MACROS) || !defined(__cplusplus)
#   define INT8_C(value)     value##i8
#   define INT16_C(value)    value##i16
#   define INT32_C(value)    value##i32
#   define INT64_C(value)    value##i64
#   define UINT8_C(value)    value##ui8
#   define UINT16_C(value)   value##ui16
#   define UINT32_C(value)   value##ui32
#   define UINT64_C(value)   value##ui64
#   define INTMAX_C(value)   value##i64
#   define UINTMAX_C(value)  value##ui64
#endif

#if defined(__STDC_LIMIT_MACROS) || !defined(__cplusplus)
#   define INT8_MAX    SCHAR_MAX
#   define INT16_MAX   SHRT_MAX
#   define INT32_MAX   INT_MAX
#   define INT64_MAX   LLONG_MAX
#   define UINT8_MAX   UCHAR_MAX
#   define UINT16_MAX  USHRT_MAX
#   define UINT32_MAX  UINT_MAX
#   define UINT64_MAX  ULLONG_MAX
#   define UINTMAX_MAX UINT64_MAX
#   define INTMAX_MAX  INT64_MAX
#endif

#if defined _WIN64
#   define UINTPTR_MAX UINT64_MAX
    typedef uint64_t uintptr_t;
#elif defined _WIN32
#   define UINTPTR_MAX UINT32_MAX
    typedef uint32_t uintptr_t;
#else
#   error
#endif

#endif
