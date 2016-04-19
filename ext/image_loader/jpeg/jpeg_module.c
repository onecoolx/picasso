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
#if defined(WIN32) && defined(_MSC_VER)
#include <windows.h>
#endif
#include <jpeglib.h>
#include <setjmp.h>

struct jpeg_error_pub {
    struct jpeg_error_mgr err;
    jmp_buf jmp_buffer;
};

struct jpeg_write_pub {
    struct jpeg_destination_mgr dst;
    unsigned char buffer[256];
    image_writer_fn writer;
    void* writer_param;
};

struct jpeg_image_ctx {
    struct jpeg_error_pub err;
    JSAMPROW rowp[1];
    // read
    struct jpeg_decompress_struct dinfo;
    int graycolor;

    // write
    struct jpeg_compress_struct cinfo;
    struct jpeg_write_pub dst;
};

static void _exit_error(j_common_ptr info)
{
    struct jpeg_error_pub* err = (struct jpeg_error_pub*)info->err;
    longjmp(err->jmp_buffer, -1);
}

static int read_jpg_info(const ps_byte* data, size_t len, psx_image_header* header)
{
    struct jpeg_image_ctx* ctx = (struct jpeg_image_ctx*)calloc(1, sizeof(struct jpeg_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    ctx->dinfo.err = jpeg_std_error(&ctx->err.err);
    ctx->err.err.error_exit = _exit_error;

    if (setjmp(ctx->err.jmp_buffer)) {
        jpeg_destroy_decompress(&ctx->dinfo);
        free(ctx);
        return -1;
    }

    jpeg_create_decompress(&ctx->dinfo);
    // set decode source.
    jpeg_mem_src(&ctx->dinfo, (unsigned char*)data, (unsigned long)len);
    jpeg_read_header(&ctx->dinfo, (unsigned char)TRUE);

    if (ctx->dinfo.num_components == 3 && ctx->dinfo.out_color_space == JCS_RGB) {
        // color space RGB
        ctx->graycolor = 0;
    } else if (ctx->dinfo.num_components == 1 && ctx->dinfo.out_color_space == JCS_GRAYSCALE) {
        // color space gray
        ctx->graycolor = 1;
    } else {
        jpeg_destroy_decompress(&ctx->dinfo);
        free(ctx);
        return -1;
    }

    header->priv = ctx;
    header->width = ctx->dinfo.image_width;
    header->height = ctx->dinfo.image_height;
    header->pitch = ctx->dinfo.image_width * 3;
    header->depth = 24;
    header->bpp = 3;
    header->format = 0;
    header->alpha = 0;
    header->frames = 1;
    return 0;
}

static int decode_jpg_data(psx_image_header* header, psx_image_frame* frame, int idx, ps_byte* buffer, size_t buffer_len)
{
    int y; size_t i;
    struct jpeg_image_ctx* ctx = (struct jpeg_image_ctx*)header->priv;

    unsigned char * row_buffer = (unsigned char*)malloc(ctx->dinfo.image_width * 3);
    if (!row_buffer)
        return -1;

    jpeg_start_decompress(&ctx->dinfo);

    if (ctx->graycolor == 1) { // gray color
        ctx->rowp[0] = &row_buffer[0];
        for (y = 0; y < header->height; y++) {
            ps_byte* row = buffer + header->pitch * y;
            jpeg_read_scanlines(&ctx->dinfo, ctx->rowp, 1);
            for (i = 0; i < ctx->dinfo.image_width; i++, row += 3) {
                row[0] = row[1] = row[2] = row_buffer[i];
            }
        }
    } else { // rgb color
        ctx->rowp[0] = &row_buffer[0];
        for (y = 0; y < header->height; y++) {
            ps_byte* row = buffer + header->pitch * y;
            unsigned char* src = row_buffer;
            jpeg_read_scanlines(&ctx->dinfo, ctx->rowp, 1);
            for (i = 0; i < ctx->dinfo.image_width; i++, src += 3, row += 3) {
                row[0] = src[0];
                row[1] = src[1];
                row[2] = src[2];
            }
        }
    }

    jpeg_finish_decompress(&ctx->dinfo);
    free(row_buffer);
    return 0;
}

static int release_read_jpg_info(psx_image_header* header)
{
    struct jpeg_image_ctx* ctx = (struct jpeg_image_ctx*)header->priv;
    jpeg_destroy_decompress(&ctx->dinfo);
    free(ctx);
    return 0;
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
    case COLOR_FORMAT_RGB565: // rgb565 will convert to rgb
    case COLOR_FORMAT_RGB555: // rgb555 will convert to rgb
        return 3;
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

static int get_color_input(int depth)
{
    switch (depth) {
        case 32:
            return JCS_EXT_RGBA;
        case 24:
        case 16: // 16 bit convert to rgb
            return JCS_RGB;
        default:
            return JCS_EXT_RGBA;
    }
}

static void _init_destination(j_compress_ptr cinfo)
{
    struct jpeg_write_pub* pub = (struct jpeg_write_pub*)cinfo->dest;
    pub->dst.next_output_byte = pub->buffer;
    memset(pub->buffer, 0, 256);
    pub->dst.free_in_buffer = 256;
}

static boolean _empty_output_buffer(j_compress_ptr cinfo)
{
    struct jpeg_write_pub* pub = (struct jpeg_write_pub*)cinfo->dest;
    pub->writer(pub->writer_param, pub->buffer, 256);
    memset(pub->buffer, 0, 256);
    pub->dst.next_output_byte = pub->buffer;
    pub->dst.free_in_buffer = 256;
    return TRUE;
}

static void _term_destination(j_compress_ptr cinfo)
{
    struct jpeg_write_pub* pub = (struct jpeg_write_pub*)cinfo->dest;
    size_t n = 256 - (size_t)(pub->dst.free_in_buffer);
    pub->writer(pub->writer_param, pub->buffer, n);
}

static int write_jpg_info(const psx_image* image, image_writer_fn func, void* param,
                                                     float quality, psx_image_header* header)
{
    struct jpeg_image_ctx* ctx = (struct jpeg_image_ctx*)calloc(1, sizeof(struct jpeg_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    ctx->cinfo.err = jpeg_std_error(&ctx->err.err);
    ctx->err.err.error_exit = _exit_error;

    if (setjmp(ctx->err.jmp_buffer)) {
        jpeg_destroy_compress(&ctx->cinfo);
        free(ctx);
        return -1;
    }

    jpeg_create_compress(&ctx->cinfo);

    ctx->cinfo.dest = (struct jpeg_destination_mgr *)&ctx->dst;
    ctx->dst.writer = func;
    ctx->dst.writer_param = param;

    ctx->dst.dst.init_destination = _init_destination;
    ctx->dst.dst.empty_output_buffer = _empty_output_buffer;
    ctx->dst.dst.term_destination = _term_destination;

    ctx->cinfo.image_width = image->width;
    ctx->cinfo.image_height = image->height;
    ctx->cinfo.input_components = get_bpp(image->format);
    ctx->cinfo.in_color_space = get_color_input(get_depth(image->format));
    jpeg_set_defaults(&ctx->cinfo);
    jpeg_set_quality(&ctx->cinfo, (int)(quality * 100), TRUE);

    header->priv = ctx;
    header->width = image->width;
    header->height = image->height;
    header->pitch = image->pitch;
    header->depth = get_depth(image->format);
    header->bpp = get_bpp(image->format);
    header->format = (int)image->format;
    header->alpha = 0;
    header->frames = 1;
    return 0;
}

static void jpeg_convert_32bit(psx_image_header* header, const ps_byte* buffer, size_t buffer_len)
{
    int y;
    struct jpeg_image_ctx* ctx = (struct jpeg_image_ctx*)header->priv;

    ps_byte* cbuf = (ps_byte*)calloc(1, header->pitch);

    for (y = 0; y < header->height; y++) {
        ps_byte* row = (ps_byte*)(buffer + header->pitch * y);
        if (header->format == (int)COLOR_FORMAT_BGRA)
            _bgra_to_rgba(cbuf, row, header->width);
        else if (header->format == (int)COLOR_FORMAT_ABGR)
            _abgr_to_rgba(cbuf, row, header->width);
        else if (header->format == (int)COLOR_FORMAT_ARGB)
            _argb_to_rgba(cbuf, row, header->width);

        ctx->rowp[0] = cbuf;
        jpeg_write_scanlines(&ctx->cinfo, ctx->rowp, 1);
    }

    free(cbuf);
}

static void jpeg_convert_24bit(psx_image_header* header, const ps_byte* buffer, size_t buffer_len)
{
    int y;
    struct jpeg_image_ctx* ctx = (struct jpeg_image_ctx*)header->priv;

    ps_byte* cbuf = (ps_byte*)calloc(1, header->width * 3);

    for (y = 0; y < header->height; y++) {
        ps_byte* row = (ps_byte*)(buffer + header->pitch * y);
        if (header->format == (int)COLOR_FORMAT_BGR)
            _bgr_to_rgb(cbuf, row, header->width);
        else if (header->format == (int)COLOR_FORMAT_RGB565)
            _rgb565_to_rgb(cbuf, row, header->width);
        else if (header->format == (int)COLOR_FORMAT_RGB555)
            _rgb555_to_rgb(cbuf, row, header->width);

        ctx->rowp[0] = cbuf;
        jpeg_write_scanlines(&ctx->cinfo, ctx->rowp, 1);
    }

    free(cbuf);
}

static int encode_jpg_data(psx_image_header* header, psx_image_frame* frame, int idx, const ps_byte* buffer, size_t buffer_len, int* ret)
{
    int y;
    struct jpeg_image_ctx* ctx = (struct jpeg_image_ctx*)header->priv;

    jpeg_start_compress(&ctx->cinfo, TRUE);

    if (header->format == (int)COLOR_FORMAT_RGBA || header->format == (int)COLOR_FORMAT_RGB) {
        for (y = 0; y < header->height; y++) {
            ps_byte* row = (ps_byte*)(buffer + header->pitch * y);
            ctx->rowp[0] = row;
            jpeg_write_scanlines(&ctx->cinfo, ctx->rowp, 1);
        }
    } else {
        if (header->format == (int)COLOR_FORMAT_BGRA
          || header->format == (int)COLOR_FORMAT_ABGR
          || header->format == (int)COLOR_FORMAT_ARGB) {
          // convert to 32bit
          jpeg_convert_32bit(header, buffer, buffer_len);
        } else {
          // convert to 24bit
          jpeg_convert_24bit(header, buffer, buffer_len);
        }
    }

    jpeg_finish_compress(&ctx->cinfo);
    return 0;
}

static int release_write_jpg_info(psx_image_header* header)
{
    struct jpeg_image_ctx* ctx = (struct jpeg_image_ctx*)header->priv;
    jpeg_destroy_compress(&ctx->cinfo);
    free(ctx);
    return 0;
}

psx_image_operator * jpg_coder = NULL;
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

    jpg_coder = (psx_image_operator*)calloc(1, sizeof(psx_image_operator));
    if (!jpg_coder)
        return;

    jpg_coder->read_header_info = read_jpg_info;
    jpg_coder->decode_image_data = decode_jpg_data;
    jpg_coder->release_read_header_info = release_read_jpg_info;

    jpg_coder->write_header_info = write_jpg_info;
    jpg_coder->encode_image_data = encode_jpg_data;
    jpg_coder->release_write_header_info = release_write_jpg_info;

    func("jpg", (ps_byte*)"\xFF\xD8\xFF", 3, PRIORITY_DEFAULT, jpg_coder);
    func("jpeg", (ps_byte*)"\xFF\xD8\xFF", 3, PRIORITY_DEFAULT, jpg_coder);
}

void psx_image_module_shutdown(void)
{
    unregister_func func = NULL;

    func = _module_get_symbol(lib_image, "psx_image_unregister_operator");
    if (func) {
        if (jpg_coder) {
            func(jpg_coder); // free jpg type coder.
            func(jpg_coder); // free jpeg type coder.
            free(jpg_coder);
        }
    }

    if (lib_image != INVALID_HANDLE)
        _module_unload(lib_image);
}

const char* psx_image_module_get_string(int idx)
{
    switch (idx) {
        case MODULE_NAME:
            return "jpeg";
        default:
            return "unknow";
    }
}

