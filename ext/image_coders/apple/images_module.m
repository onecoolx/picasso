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

#if 0
// Represents raw, premultiplied, RGBA image data with tightly packed rows
// (width * 4 bytes).
struct PlatformCGImage
{
    uint32_t width = 0;
    uint32_t height = 0;
    bool opaque = false;
    std::unique_ptr<uint8_t[]> pixels;
};

bool cg_image_decode(const uint8_t* encodedBytes,
                     size_t encodedSizeInBytes,
                     PlatformCGImage* platformImage)
{
    AutoCF data =
        CFDataCreate(kCFAllocatorDefault, encodedBytes, encodedSizeInBytes);
    if (!data)
    {
        return false;
    }

    AutoCF source = CGImageSourceCreateWithData(data, nullptr);
    if (!source)
    {
        return false;
    }

    AutoCF image = CGImageSourceCreateImageAtIndex(source, 0, nullptr);
    if (!image)
    {
        return false;
    }

    bool isOpaque = false;
    switch (CGImageGetAlphaInfo(image.get()))
    {
        case kCGImageAlphaNone:
        case kCGImageAlphaNoneSkipFirst:
        case kCGImageAlphaNoneSkipLast:
            isOpaque = true;
            break;
        default:
            break;
    }

    const size_t width = CGImageGetWidth(image);
    const size_t height = CGImageGetHeight(image);
    const size_t rowBytes = width * 4; // 4 bytes per pixel
    const size_t size = rowBytes * height;

    const size_t bitsPerComponent = 8;
    CGBitmapInfo cgInfo = kCGBitmapByteOrder32Big; // rgba
    if (isOpaque)
    {
        cgInfo |= kCGImageAlphaNoneSkipLast;
    }
    else
    {
        cgInfo |= kCGImageAlphaPremultipliedLast;
    }

    std::unique_ptr<uint8_t[]> pixels(new uint8_t[size]);

    AutoCF cs = CGColorSpaceCreateDeviceRGB();
    AutoCF cg = CGBitmapContextCreate(
        pixels.get(), width, height, bitsPerComponent, rowBytes, cs, cgInfo);
    if (!cg)
    {
        return false;
    }

    CGContextSetBlendMode(cg, kCGBlendModeCopy);
    CGContextDrawImage(cg, CGRectMake(0, 0, width, height), image);

    platformImage->width = rive::castTo<uint32_t>(width);
    platformImage->height = rive::castTo<uint32_t>(height);
    platformImage->opaque = isOpaque;
    platformImage->pixels = std::move(pixels);

    return true;
}

std::unique_ptr<Bitmap> Bitmap::decode(const uint8_t bytes[], size_t byteCount)
{
    PlatformCGImage image;
    if (!cg_image_decode(bytes, byteCount, &image))
    {
        return nullptr;
    }

    // CG only supports premultiplied alpha. Unmultiply now.
    size_t imageNumPixels = image.height * image.width;
    size_t imageSizeInBytes = imageNumPixels * 4;
    // Process 2 pixels at once, deal with odd number of pixels
    if (imageNumPixels & 1)
    {
        imageSizeInBytes -= 4;
    }
    size_t i;
    for (i = 0; i < imageSizeInBytes; i += 8)
    {
        // Load 2 pixels into 64 bits
        auto twoPixels = rive::simd::load<uint8_t, 8>(&image.pixels[i]);
        auto a0 = twoPixels[3];
        auto a1 = twoPixels[7];
        // Avoid computation if both pixels are either fully transparent or
        // opaque pixels
        if ((a0 > 0 && a0 < 255) || (a1 > 0 && a1 < 255))
        {
            // Avoid potential division by zero
            a0 = std::max<uint8_t>(a0, 1);
            a1 = std::max<uint8_t>(a1, 1);
            // Cast to 16 bits to avoid overflow
            rive::uint16x8 rgbaWidex2 = rive::simd::cast<uint16_t>(twoPixels);
            // Unpremult: multiply by RGB by "255.0 / alpha"
            rgbaWidex2 *= rive::uint16x8{255, 255, 255, 1, 255, 255, 255, 1};
            rgbaWidex2 /= rive::uint16x8{a0, a0, a0, 1, a1, a1, a1, 1};
            // Cast back to 8 bits and store
            twoPixels = rive::simd::cast<uint8_t>(rgbaWidex2);
            rive::simd::store(&image.pixels[i], twoPixels);
        }
    }
    // Process last odd pixel if needed
    if (imageNumPixels & 1)
    {
        // Load 1 pixel into 32 bits
        auto rgba = rive::simd::load<uint8_t, 4>(&image.pixels[i]);
        // Avoid computation for fully transparent or opaque pixels
        if (rgba.a > 0 && rgba.a < 255)
        {
            // Cast to 16 bits to avoid overflow
            rive::uint16x4 rgbaWide = rive::simd::cast<uint16_t>(rgba);
            // Unpremult: multiply by RGB by "255.0 / alpha"
            rgbaWide *= rive::uint16x4{255, 255, 255, 1};
            rgbaWide /= rive::uint16x4{rgba.a, rgba.a, rgba.a, 1};
            // Cast back to 8 bits and store
            rgba = rive::simd::cast<uint8_t>(rgbaWide);
            rive::simd::store(&image.pixels[i], rgba);
        }
    }

    return std::make_unique<Bitmap>(
        image.width, image.height, PixelFormat::RGBA, std::move(image.pixels));
}

#endif

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
    header->width = width;
    header->height = height;
    header->pitch = rowbytes;
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
    CGContextRef cg = CGBitmapContextCreate(buffer, width, height, bitsPerComponent, header->rowbytes, cs, cgInfo);
    if (!cg) {
        CGColorSpaceRelease(cs);
        return -1;
    }

    CGImageRef image = CGImageSourceCreateImageAtIndex(ctx->source, idx, NULL);

    CGContextSetBlendMode(cg, kCGBlendModeCopy);
    CGContextDrawImage(cg, CGRectMake(0, 0, width, height), image);

    CGImageRelease(image);
    CGContextRelease(cg)
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

    lib_image = _module_load("libpsx_image.so");
    if (lib_image == INVALID_HANDLE)
        return;

    func = _module_get_symbol(lib_image, "psx_image_register_operator");
    if (!func)
        return;

    cg_coder = (psx_image_operator*)calloc(1, sizeof(psx_image_operator));
    if (!jpg_coder)
        return;

    cg_coder->read_header_info = read_image_info;
    cg_coder->decode_image_data = decode_image_data;
    cg_coder->release_read_header_info = release_read_image_info;

    cg_coder->write_header_info = write_image_info;
    cg_coder->encode_image_data = encode_image_data;
    cg_coder->release_write_header_info = release_write_image_info;

    func("jpg", (ps_byte*)"\xFF\xD8\xFF", 0, 3, PRIORITY_DEFAULT, cg_coder);
    func("jpeg", (ps_byte*)"\xFF\xD8\xFF", 0, 3, PRIORITY_DEFAULT, cg_coder);
    //func("jpeg2k", (ps_byte*)"\xFF\xD8\xFF", 0, 3, PRIORITY_DEFAULT, cg_coder);
    //func("tiff", (ps_byte*)"\xFF\xD8\xFF", 0, 3, PRIORITY_DEFAULT, cg_coder);
    //func("pict", (ps_byte*)"\xFF\xD8\xFF", 0, 3, PRIORITY_DEFAULT, cg_coder);
    //func("qtif", (ps_byte*)"\xFF\xD8\xFF", 0, 3, PRIORITY_DEFAULT, cg_coder);
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

