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

#ifndef _PICASSO_TEST_H_
#define _PICASSO_TEST_H_

#include "picasso.h"
#include "images/psx_image.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define TEST_WIDTH 640
#define TEST_HEIGHT 480

void PS_Init();
void PS_Shutdown();
void clear_dcache(void);
ps_canvas* get_test_canvas(void);
void clear_test_canvas(void);

#if defined(WIN32)
    #define SYSTEM "win32"
#elif defined(__APPLE__)
    #define SYSTEM "apple"
#else
    #define SYSTEM "linux"
#endif

#if defined(__i386__) \
    || defined(i386)     \
    || defined(_M_IX86)  \
    || defined(_X86_)    \
    || defined(__THW_INTEL) \
    || defined(__x86_64__) \
    || defined(_M_X64)
    #define ARCH "x86"
#elif defined(arm) \
    || defined(__arm__) \
    || defined(ARM) \
    || defined(_ARM_)
    #define ARCH "arm"
#else
    #define ARCH "unknown"
#endif

#endif
