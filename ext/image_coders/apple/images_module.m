/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "picasso_image.h"
#include "picasso_image_plugin.h"

#include "psx_image_io.h"
#include "psx_color_convert.h"

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#elif TARGET_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

struct cg_image_ctx {
    CGImageSourceRef source;

    // write
    image_writer_fn writer;
    void* writer_param;
};

static int read_image_info(const ps_byte* data, size_t len, psx_image_header* header)
{
    CFDataRef cg_data = CFDataCreate(kCFAllocatorDefault, data, len);
    if (!cg_data) {
        return -1;
    }

    struct cg_image_ctx* ctx = (struct cg_image_ctx*)calloc(1, sizeof(struct cg_image_ctx));
    if (!ctx) {
        CFRelease(cg_data);
        return -1; // out of memory
    }

    ctx->source = CGImageSourceCreateWithData(cg_data, NULL);

    CGImageRef image = CGImageSourceCreateImageAtIndex(ctx->source, 0, NULL);
    if (!image) {
        CFRelease(ctx->source);
        CFRelease(cg_data);
        return -1; // out of memory
    }

    int bpp = 4; // 4 bytes per pixel
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    size_t rowbytes = width * bpp;

    header->priv = ctx;
    header->width = (int)width;
    header->height = (int)height;
    header->pitch = (int)rowbytes;
    header->depth = 32;
    header->bpp = bpp;
    header->format = 0;
    header->alpha = 1;
    header->frames = (int)CGImageSourceGetCount(ctx->source);

    CGImageRelease(image);
    CFRelease(cg_data);
    return 0;
}

static int release_read_image_info(psx_image_header* header)
{
    struct cg_image_ctx* ctx = (struct cg_image_ctx*)header->priv;
    CFRelease(ctx->source);
    free(ctx);
    return 0;
}

static int decode_image_data(psx_image_header* header, const psx_image* image, psx_image_frame* frame, int idx, ps_byte* buffer, size_t buffer_len)
{
    struct cg_image_ctx* ctx = (struct cg_image_ctx*)header->priv;

    int width = header->width;
    int height = header->height;

    size_t bitsPerComponent = 8;
    CGBitmapInfo cgInfo = kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipLast;

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef cg = CGBitmapContextCreate(buffer, width, height, bitsPerComponent, header->pitch, cs, cgInfo);
    if (!cg) {
        CGColorSpaceRelease(cs);
        return -1;
    }

    CGImageRef cg_image = CGImageSourceCreateImageAtIndex(ctx->source, idx, NULL);

    CGContextSetBlendMode(cg, kCGBlendModeCopy);
    CGContextDrawImage(cg, CGRectMake(0, 0, width, height), cg_image);

    CGImageRelease(cg_image);
    CGContextRelease(cg);
    CGColorSpaceRelease(cs);
    return 0;
}


static int write_image_info(const psx_image* image, image_writer_fn func, void* param, float quality, psx_image_header* header)
{
}

static int release_write_image_info(psx_image_header* header)
{
}

static int encode_image_data(psx_image_header* header, const psx_image* image, psx_image_frame* frame,
                                                                     int idx, const ps_byte* buffer, size_t buffer_len, int* ret)
{
}

static psx_image_operator * cg_coder = NULL;
static module_handle lib_image = INVALID_HANDLE;

typedef int (*register_func)(const char*, const ps_byte*, size_t, size_t, psx_priority_level, psx_image_operator*);
typedef int (*unregister_func)(psx_image_operator*);

void psx_image_module_init(void)
{
    register_func func = NULL;

    lib_image = _module_load("libpsx_image.dylib");
    if (lib_image == INVALID_HANDLE)
        return;

    func = _module_get_symbol(lib_image, "psx_image_register_operator");
    if (!func)
        return;

    cg_coder = (psx_image_operator*)calloc(1, sizeof(psx_image_operator));
    if (!cg_coder)
        return;

    cg_coder->read_header_info = read_image_info;
    cg_coder->decode_image_data = decode_image_data;
    cg_coder->release_read_header_info = release_read_image_info;

    cg_coder->write_header_info = write_image_info;
    cg_coder->encode_image_data = encode_image_data;
    cg_coder->release_write_header_info = release_write_image_info;

    func("jpg", (ps_byte*)"\xFF\xD8\xFF", 0, 3, PRIORITY_DEFAULT, cg_coder);
    func("jpeg", (ps_byte*)"\xFF\xD8\xFF", 0, 3, PRIORITY_DEFAULT, cg_coder);
    func("png", (ps_byte*)"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 0, 8, PRIORITY_DEFAULT, cg_coder);
    func("gif", (ps_byte*)"GIF", 0, 3, PRIORITY_DEFAULT, cg_coder);
}

void psx_image_module_shutdown(void)
{
    unregister_func func = NULL;

    func = _module_get_symbol(lib_image, "psx_image_unregister_operator");
    if (func) {
        if (cg_coder) {
            func(cg_coder); //jpg
            func(cg_coder); //jpeg
            func(cg_coder); //png
            func(cg_coder); //gif
            free(cg_coder);
        }
    }

    if (lib_image != INVALID_HANDLE)
        _module_unload(lib_image);
}

const char* psx_image_module_get_string(int idx)
{
    switch (idx) {
        case MODULE_NAME:
            return "apple";
        default:
            return "unknown";
    }
}

