/*
 * Copyright (c) 2025, Zhang Ji Peng
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

#ifndef _TU_H_
#define _TU_H_

#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#if defined(LINUX) || defined(UNIX)
    #include "unistd.h"
    #include "sys/time.h"
#endif
#if defined(WIN32) || defined(WINCE)
    #include <windows.h>
#endif

#if defined(WIN32) || defined(WINCE)
    typedef long long suseconds_t;
    #define inline __inline
#endif

static inline suseconds_t get_time()
{
#if defined(WIN32) || defined(WINCE)
    DWORD t1 = GetTickCount();
#else
    struct timeval t;
    gettimeofday(&t, 0);
    suseconds_t t1 = (suseconds_t)(t.tv_sec * 1000 + t.tv_usec / 1000);
#endif
    return t1;
}

#if defined(WIN32) || defined(WINCE)
typedef LARGE_INTEGER clocktime_t;

static inline clocktime_t get_clock()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t;
}

static inline double get_clock_used_ms(LARGE_INTEGER t1, LARGE_INTEGER t2)
{
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    return ((double)(t2.QuadPart - t1.QuadPart) / (double)f.QuadPart) * 1000;
}

#endif

#endif
