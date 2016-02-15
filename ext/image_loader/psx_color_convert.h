/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PSX_COLOR_CONVERT_H_
#define _PSX_COLOR_CONVERT_H_

#include <stdlib.h>

#if defined(__GNUC__)
#define INLINE inline
#elif defined(_MSC_VER)
#define INLINE __inline
#else
#define INLINE
#endif

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
