/*
 * Copyright (c) 2024, Zhang Ji Peng
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PSX_COMMON_H_
#define _PSX_COMMON_H_

#include "pconfig.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(WIN32) && defined(_MSC_VER)
    #define strdup(s) _strdup(s)
#endif

#ifdef HAVE_STDBOOL_H
    #include <stdbool.h>
#else
    #if !defined(_MSC_VER)
        #include <stddef.h>
        typedef int32_t bool;
        #define true 1;
        #define false 0;
    #endif
#endif

#if defined(__GNUC__)
    #define INLINE inline
#elif defined(_MSC_VER)
    #define INLINE __inline
#else
    #define INLINE
#endif

#if __cplusplus >= 201103L
    #define REGISTER
#else
    #define REGISTER register
#endif

/* memory management */
#ifndef mem_malloc
    #define mem_malloc malloc
#endif

#ifndef mem_calloc
    #define mem_calloc calloc
#endif

#ifndef mem_realloc
    #define mem_realloc realloc
#endif

#ifndef mem_free
    #define mem_free free
#endif

#ifndef mem_copy
    #define mem_copy memcpy
#endif

/* abort */
#define ABORT() abort()

/* assert */
#define ASSERT(cond) assert((cond))

#define LOG_ERROR(...) \
    psx_log(__VA_ARGS__)

/* c++ class utils */
#define NON_COPYABLE_CLASS(type) \
    private: \
    type(const type& o); \
    type& operator=(const type& o); \

#ifdef __cplusplus
extern "C" {
#endif

/* logs*/
void psx_log(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /*_PSX_COMMON_H_*/
