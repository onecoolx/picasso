/*
 * Copyright (c) 2016, Zhang Ji Peng
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

#ifndef _PSX_COLOR_CONVERT_H_
#define _PSX_COLOR_CONVERT_H_

#include "psx_common.h"
#include <stdlib.h>

/* 32 bit color convert */
static INLINE void _argb_to_rgba(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        *dst++ = src[1];
        *dst++ = src[2];
        *dst++ = src[3];
        *dst++ = src[0];
        src += 4;
    } while (--len);
}

static INLINE void _abgr_to_rgba(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        *dst++ = src[3];
        *dst++ = src[2];
        *dst++ = src[1];
        *dst++ = src[0];
        src += 4;
    } while (--len);
}

static INLINE void _bgra_to_rgba(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        *dst++ = src[2];
        *dst++ = src[1];
        *dst++ = src[0];
        *dst++ = src[3];
        src += 4;
    } while (--len);
}

/* 24 bit color convert */
static INLINE void _argb_to_rgb(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        *dst++ = src[1];
        *dst++ = src[2];
        *dst++ = src[3];
        src += 4;
    } while (--len);
}

static INLINE void _abgr_to_rgb(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        *dst++ = src[3];
        *dst++ = src[2];
        *dst++ = src[1];
        src += 4;
    } while (--len);
}

static INLINE void _bgra_to_rgb(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        *dst++ = src[2];
        *dst++ = src[1];
        *dst++ = src[0];
        src += 4;
    } while (--len);
}

static INLINE void _rgba_to_rgb(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        *dst++ = src[0];
        *dst++ = src[1];
        *dst++ = src[2];
        src += 4;
    } while (--len);
}

static INLINE void _bgr_to_rgb(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        *dst++ = src[2];
        *dst++ = src[1];
        *dst++ = src[0];
        src += 3;
    } while (--len);
}

/* 16 bit color convert */
static INLINE void _rgb565_to_rgb(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        uint32_t rgb = *(uint16_t*)src;
        dst[0] = (rgb >> 8) & 0xF8;
        dst[1] = (rgb >> 3) & 0xFC;
        dst[2] = (rgb << 3) & 0xF8;
        src += 2;
        dst += 3;
    } while (--len);
}

static INLINE void _rgb555_to_rgb(uint8_t* dst, const uint8_t* src, size_t len)
{
    do {
        uint32_t rgb = *(uint16_t*)src;
        dst[0] = (rgb >> 7) & 0xF8;
        dst[1] = (rgb >> 2) & 0xF8;
        dst[2] = (rgb << 3) & 0xF8;
        src += 2;
        dst += 3;
    } while (--len);
}

#endif /*_PSX_COLOR_CONVERT_H_*/
