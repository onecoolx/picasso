/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <png.h>

#include "picasso_image.h"
#include "picasso_image_plugin.h"

#include "psx_image_io.h"
#include "psx_color_convert.h"
#if defined(WIN32) && defined(_MSC_VER)
#include <windows.h>
#endif

struct png_image_ctx {
    png_structp png_ptr;
    png_infop info_ptr;
    // read
    uint8_t* pos;
    uint8_t* end;
    // write
    image_writer_fn writer;
    void* writer_param;
};

static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct png_image_ctx* ctx = (struct png_image_ctx*)png_get_io_ptr(png_ptr);
    if (ctx->pos + length > ctx->end)
        png_error(png_ptr, "PNG read error!");

    memcpy(data, ctx->pos, length);
    ctx->pos += length;
}

static int read_png_info(const ps_byte* data, size_t len, psx_image_header* header)
{
    png_uint_32 width,height;
    int bit_depth, color_type, interlace_type, rowbytes, bpp;
    double screen_gamma = 2.2; // typical value
    double gamma;
    png_color_16p dib_background;

    struct png_image_ctx* ctx = (struct png_image_ctx*)calloc(1, sizeof(struct png_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    ctx->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!ctx->png_ptr){
        free(ctx);
        return -1;
    }

    ctx->info_ptr = png_create_info_struct(ctx->png_ptr);
    if (!ctx->info_ptr) {
        png_destroy_read_struct(&ctx->png_ptr, NULL, NULL);
        free(ctx);
        return -1;
    }

    if (setjmp(png_jmpbuf(ctx->png_ptr))) { // error occurred
        png_destroy_read_struct(&ctx->png_ptr, &ctx->info_ptr, NULL);
        free(ctx);
        return -1;
    }

    ctx->pos = (uint8_t*)data;
    ctx->end = (uint8_t*)data + len;

    png_set_read_fn(ctx->png_ptr, (void*)ctx, png_read_data);

    png_read_info(ctx->png_ptr, ctx->info_ptr);

    png_get_IHDR(ctx->png_ptr, ctx->info_ptr, &width, &height,
                    &bit_depth, &color_type, &interlace_type, NULL, NULL);

   // configure transformations, we always want RGB data in the end
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(ctx->png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(ctx->png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
#if PNG_LIBPNG_VER >= 10209
      png_set_expand_gray_1_2_4_to_8(ctx->png_ptr);
#else
      png_set_gray_1_2_4_to_8(ctx->png_ptr);
#endif
    if (png_get_valid(ctx->png_ptr, ctx->info_ptr,PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(ctx->png_ptr);
    if (bit_depth == 16)
      png_set_strip_16(ctx->png_ptr);
    if (bit_depth < 8)
      png_set_packing(ctx->png_ptr);

    if (png_get_bKGD(ctx->png_ptr, ctx->info_ptr, &dib_background))
        png_set_background(ctx->png_ptr, dib_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);

    if (png_get_gAMA(ctx->png_ptr, ctx->info_ptr, &gamma))
        png_set_gamma(ctx->png_ptr, screen_gamma, gamma);
    else
        png_set_gamma(ctx->png_ptr, screen_gamma, 0.45455);

    // update info after applying transformations
    png_read_update_info(ctx->png_ptr, ctx->info_ptr);
    rowbytes = png_get_rowbytes(ctx->png_ptr, ctx->info_ptr);
    bpp = rowbytes / width;

    rowbytes = png_get_rowbytes(ctx->png_ptr, ctx->info_ptr);

    header->priv = ctx;
    header->width = width;
    header->height = height;
    header->pitch = rowbytes;
    header->depth = bit_depth;
    header->bpp = bpp;
    header->format = 0;
    header->alpha = (bpp == 4) ? 1 : 0;
    header->frames = 1;
    return 0;
}

static int release_read_png_info(psx_image_header* header)
{
    struct png_image_ctx* ctx = (struct png_image_ctx*)header->priv;
    png_destroy_read_struct(&ctx->png_ptr, &ctx->info_ptr, NULL);
    free(ctx);
    return 0;
}

static int decode_png_data(psx_image_header* header, psx_image_frame* frame, int idx, ps_byte* buffer, size_t buffer_len)
{
    int y;
    struct png_image_ctx* ctx = (struct png_image_ctx*)header->priv;

    for (y = 0; y < header->height; y++) {
        ps_byte* row = buffer + header->pitch * y;
        png_read_row(ctx->png_ptr, row, NULL);
    }

    png_read_end(ctx->png_ptr, ctx->info_ptr);
    return 0;
}

static void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct png_image_ctx* ctx = (struct png_image_ctx*)png_get_io_ptr(png_ptr);
    ctx->writer(ctx->writer_param, data, length);
}

static int get_bpp(ps_color_format fmt)
{
    switch (fmt) {
    case COLOR_FORMAT_RGBA:
    case COLOR_FORMAT_BGRA:
    case COLOR_FORMAT_ARGB:
    case COLOR_FORMAT_ABGR:
        return 4;
    case COLOR_FORMAT_RGB:
    case COLOR_FORMAT_BGR:
        return 3;
    case COLOR_FORMAT_RGB565:
    case COLOR_FORMAT_RGB555:
        return 2;
    default:
        return 4;
    }
}

static int get_depth(ps_color_format fmt)
{
    switch (fmt) {
    case COLOR_FORMAT_RGBA:
    case COLOR_FORMAT_BGRA:
    case COLOR_FORMAT_ARGB:
    case COLOR_FORMAT_ABGR:
        return 32;
    case COLOR_FORMAT_RGB:
    case COLOR_FORMAT_BGR:
        return 24;
    case COLOR_FORMAT_RGB565:
    case COLOR_FORMAT_RGB555:
        return 16;
    default:
        return 32;
    }
}

static int write_png_info(const psx_image* image, image_writer_fn func, void* param,
                                                     float quality, psx_image_header* header)
{
    unsigned int fmt;
    struct png_image_ctx* ctx = (struct png_image_ctx*)calloc(1, sizeof(struct png_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    ctx->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!ctx->png_ptr){
        free(ctx);
        return -1;
    }

    ctx->info_ptr = png_create_info_struct(ctx->png_ptr);
    if (!ctx->info_ptr) {
        png_destroy_write_struct(&ctx->png_ptr, NULL);
        free(ctx);
        return -1;
    }

    if (setjmp(png_jmpbuf(ctx->png_ptr))) { // error occurred
        png_destroy_write_struct(&ctx->png_ptr, &ctx->info_ptr);
        free(ctx);
        return -1;
    }

    ctx->writer = func;
    ctx->writer_param = param;

    png_set_write_fn(ctx->png_ptr, (void*)ctx, png_write_data, NULL);

    switch (image->format)
    {
    case COLOR_FORMAT_RGBA:
    case COLOR_FORMAT_BGRA:
    case COLOR_FORMAT_ARGB:
    case COLOR_FORMAT_ABGR:
        fmt = PNG_COLOR_TYPE_RGB_ALPHA;
        break;
    case COLOR_FORMAT_RGB:
    case COLOR_FORMAT_BGR:
    case COLOR_FORMAT_RGB565: // rgb565 will convert to rgb
    case COLOR_FORMAT_RGB555: // rgb555 will convert to rgb
        fmt = PNG_COLOR_TYPE_RGB;
        break;
    default:
        fmt = PNG_COLOR_TYPE_RGB_ALPHA; // impossible here.
        break;
    }

    png_set_IHDR(ctx->png_ptr, ctx->info_ptr, image->width, image->height, 8, fmt,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(ctx->png_ptr, ctx->info_ptr);

    header->priv = ctx;
    header->width = image->width;
    header->height = image->height;
    header->pitch = image->pitch;
    header->depth = get_depth(image->format);
    header->bpp = get_bpp(image->format);
    header->format = (int)image->format;
    header->alpha = (fmt == PNG_COLOR_TYPE_RGB_ALPHA) ? 1 : 0;
    header->frames = 1;
    return 0;
}

static int release_write_png_info(psx_image_header* header)
{
    struct png_image_ctx* ctx = (struct png_image_ctx*)header->priv;
    png_destroy_write_struct(&ctx->png_ptr, &ctx->info_ptr);
    free(ctx);
    return 0;
}

static void png_convert_32bit(psx_image_header* header, const ps_byte* buffer, size_t buffer_len)
{
    int y;
    struct png_image_ctx* ctx = (struct png_image_ctx*)header->priv;

    ps_byte* cbuf = (ps_byte*)calloc(1, header->pitch);

    for (y = 0; y < header->height; y++) {
        ps_byte* row = (ps_byte*)(buffer + header->pitch * y);
        if (header->format == (int)COLOR_FORMAT_BGRA)
            _bgra_to_rgba(cbuf, row, header->width);
        else if (header->format == (int)COLOR_FORMAT_ABGR)
            _abgr_to_rgba(cbuf, row, header->width);
        else if (header->format == (int)COLOR_FORMAT_ARGB)
            _argb_to_rgba(cbuf, row, header->width);

        png_write_row(ctx->png_ptr, cbuf);
    }

    free(cbuf);
}

static void png_convert_24bit(psx_image_header* header, const ps_byte* buffer, size_t buffer_len)
{
    int y;
    struct png_image_ctx* ctx = (struct png_image_ctx*)header->priv;

    ps_byte* cbuf = (ps_byte*)calloc(1, header->width * 3);

    for (y = 0; y < header->height; y++) {
        ps_byte* row = (ps_byte*)(buffer + header->pitch * y);
        if (header->format == (int)COLOR_FORMAT_BGR)
            _bgr_to_rgb(cbuf, row, header->width);
        else if (header->format == (int)COLOR_FORMAT_RGB565)
            _rgb565_to_rgb(cbuf, row, header->width);
        else if (header->format == (int)COLOR_FORMAT_RGB555)
            _rgb555_to_rgb(cbuf, row, header->width);

        png_write_row(ctx->png_ptr, cbuf);
    }

    free(cbuf);
}

static int encode_png_data(psx_image_header* header, psx_image_frame* frame, int idx, const ps_byte* buffer, size_t buffer_len, int* ret)
{
    int y;
    struct png_image_ctx* ctx = (struct png_image_ctx*)header->priv;

    if (header->format == (int)COLOR_FORMAT_RGBA || header->format == (int)COLOR_FORMAT_RGB) {
        for (y = 0; y < header->height; y++) {
            ps_byte* row = (ps_byte*)(buffer + header->pitch * y);
            png_write_row(ctx->png_ptr, row);
        }
    } else {
        if (header->format == (int)COLOR_FORMAT_BGRA
          || header->format == (int)COLOR_FORMAT_ABGR
          || header->format == (int)COLOR_FORMAT_ARGB) {
          // convert to 32bit
          png_convert_32bit(header, buffer, buffer_len);
        } else {
          // convert to 24bit
          png_convert_24bit(header, buffer, buffer_len);
        }
    }

    png_write_end(ctx->png_ptr, ctx->info_ptr);
    return 0;
}

psx_image_operator * png_coder = NULL;
static module_handle lib_image = INVALID_HANDLE;

typedef int (*register_func)(const char*, const ps_byte*, size_t, psx_priority_level, psx_image_operator*);
typedef int (*unregister_func)(psx_image_operator*);

#if defined(WIN32) && defined(_MSC_VER)
static wchar_t g_path[MAX_PATH];
static wchar_t* get_library_path(void)
{
    wchar_t *p = 0;
    memset(g_path, 0, sizeof(wchar_t) * MAX_PATH);
    GetModuleFileName(NULL, g_path, MAX_PATH);
    p = wcsrchr(g_path, '\\');
    p++; *p = 0;
    lstrcat(g_path, L"psx_image.dll");
    return g_path;
}
#endif

void psx_image_module_init(void)
{
    register_func func = NULL;

#if defined(WIN32) && defined(_MSC_VER)
    lib_image = _module_load(get_library_path());
#else
    lib_image = _module_load("libpsx_image.so");
#endif
    if (lib_image == INVALID_HANDLE)
        return;

    func = _module_get_symbol(lib_image, "psx_image_register_operator");
    if (!func)
        return;

    png_coder = (psx_image_operator*)calloc(1, sizeof(psx_image_operator));
    if (!png_coder)
        return;

    png_coder->read_header_info = read_png_info;
    png_coder->decode_image_data = decode_png_data;
    png_coder->release_read_header_info = release_read_png_info;

    png_coder->write_header_info = write_png_info;
    png_coder->encode_image_data = encode_png_data;
    png_coder->release_write_header_info = release_write_png_info;

    func("png", (ps_byte*)"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8, PRIORITY_DEFAULT, png_coder);
}

void psx_image_module_shutdown(void)
{
    unregister_func func = NULL;

    func = _module_get_symbol(lib_image, "psx_image_unregister_operator");
    if (func) {
        if (png_coder) {
            func(png_coder);
            free(png_coder);
        }
    }

    if (lib_image != INVALID_HANDLE)
        _module_unload(lib_image);
}

const char* psx_image_module_get_string(int idx)
{
    switch (idx) {
        case MODULE_NAME:
            return "png";
        default:
            return "unknow";
    }
}

