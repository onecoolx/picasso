/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "picasso.h"
#include "picasso_image.h"
#include "picasso_image_plugin.h"

#include "psx_image_io.h"
#include "psx_image_modules.h"
#include "psx_image_loader.h"

static struct image_modules_mgr* g_modules = NULL;

static ps_color_format get_format(const psx_image_header* hdr)
{
    switch (hdr->bpp)
    {
    case 4:
        return COLOR_FORMAT_RGBA;
    case 3:
        return COLOR_FORMAT_RGB;
    case 2:
        return COLOR_FORMAT_RGB565;
    default:
        return COLOR_FORMAT_UNKNOWN;
    }
}

static int get_bpp(ps_color_format fmt)
{
    switch (fmt)
    {
    case COLOR_FORMAT_RGBA:
    case COLOR_FORMAT_ARGB:
    case COLOR_FORMAT_ABGR:
    case COLOR_FORMAT_BGRA:
        return 4;
    case COLOR_FORMAT_RGB:
    case COLOR_FORMAT_BGR:
        return 3;
    case COLOR_FORMAT_RGB565:
    case COLOR_FORMAT_RGB555:
        return 2;
    default:
        return 0;
    }
}

struct image_modules_mgr* _get_modules(void)
{
    return g_modules;
}

int PICAPI psx_image_init(void)
{
    ps_initialize();

    if (!g_modules) {
        g_modules = (struct image_modules_mgr*)calloc(1, sizeof(struct image_modules_mgr));
        if (!g_modules)
            return S_OUT_OF_MEMORY;

        if (modules_init(g_modules) != 0) {
            free(g_modules);
            g_modules = NULL;
            return S_INIT_FAILURE;
        }
    }
    return S_OK;
}

int PICAPI psx_image_shutdown(void)
{
    if (g_modules) {
        modules_destroy(g_modules);
        free(g_modules);
        g_modules = NULL;
    }

    ps_shutdown();
    return S_OK;
}

psx_image* PICAPI psx_image_create_from_data(ps_byte* data, ps_color_format fmt,
                                             int width, int height, int pitch, int* err_code)
{
    int i;
    size_t size;
    psx_image * image;

    if (!data || width <= 0 || height <= 0 || pitch <= 0) {
        if (err_code)
            *err_code = S_BAD_PARAMS;
        return NULL;
    }

    if (fmt < 0 || fmt >= COLOR_FORMAT_UNKNOWN) {
        if (err_code)
            *err_code = S_BAD_PARAMS;
        return NULL;
    }

    image = (psx_image*)calloc(1, sizeof(psx_image));
    if (image) {
        //FIXME: add more image attribute here
        image->width = width;
        image->height = height;
        image->pitch = get_bpp(fmt) * width;
        image->format = fmt;
        image->num_frames = 1;

        image->frames = (psx_image_frame*)calloc(image->num_frames, sizeof(psx_image_frame));
        if (!image->frames) {
            if (err_code)
                *err_code = S_OUT_OF_MEMORY;
            free(image);
            return NULL;
        }

        size = image->pitch * image->height;
        image->frames[0].size = size;
        image->frames[0].data = (ps_byte*)calloc(1, size);
        image->frames[0].duration = 0;

        if (!image->frames[0].data) {
            if (err_code)
                *err_code = S_OUT_OF_MEMORY;
            free(image->frames);
            free(image);
            return NULL;
        }

        for (i = 0; i < image->height; i++) {
            memcpy(image->frames[0].data + (image->pitch * i), data + (i * pitch), image->pitch);
        }

        // create ps_image object.
        image->frames[0].img = ps_image_create_with_data(image->frames[0].data, image->format,
                                                             image->width, image->height, image->pitch);
        if (!image->frames[0].img) {
            if (err_code)
                *err_code = S_OUT_OF_MEMORY;
            free(image->frames[0].data);
            free(image->frames);
            free(image);
            return NULL;
        }

        if (get_bpp(fmt) == 4) // has alpha
            ps_image_set_allow_transparent(image->frames[0].img, True);

    } else {
        if (err_code)
            *err_code = S_OUT_OF_MEMORY;
		return NULL;
    }
	if (err_code)
        *err_code = S_OK;
    return image;
}

psx_image* PICAPI psx_image_load(const char* name, int* err_code)
{
    size_t size;
    ps_byte* file_data;
    psx_image * image;
    pchar* file_name;

    if (!name) {
        if (err_code)
            *err_code = S_BAD_PARAMS;
        return NULL;
    }

    file_name = pstring_create(name, NULL);
    if (!file_name) {
        if (err_code)
            *err_code = S_FAILURE;
        return NULL;
    }

    if (_file_exists(file_name) != 0) {
        if (err_code)
            *err_code = S_BAD_PARAMS;
        free(file_name);
        return NULL;
    }

    size = _file_size(file_name);
    if (!size) {
        if (err_code)
            *err_code = S_BAD_PARAMS;
        free(file_name);
        return NULL;
    }

    file_data = (ps_byte*)malloc(size);
    if (!file_data) {
        if (err_code)
            *err_code = S_OUT_OF_MEMORY;
        free(file_name);
        return NULL;
    }
    // read file data.
    if (_file_read(file_name, file_data, size) != 0){
        free(file_data);
        if (err_code)
            *err_code = S_FAILURE;
        free(file_name);
        return NULL;
    }

    image = psx_image_load_from_memory(file_data, size, err_code);
    free(file_name);
    free(file_data);
	if (err_code)
        *err_code = S_OK;
    return image;
}

static psx_image* load_psx_image(psx_image_operator* op, const ps_byte* data, size_t len)
{
    size_t i;
    psx_image_header header;
    psx_image* image = NULL;
    if (!op->read_header_info || !op->decode_image_data)
        return NULL;

    if (op->read_header_info(data, len, &header) != 0)
        return NULL;

    image = (psx_image*)calloc(1, sizeof(psx_image));
    if (image) {
        //FIXME: add more image attribute here
        image->width = header.width;
        image->height = header.height;
        image->pitch = header.pitch;
        image->format = get_format(&header);
        image->num_frames = header.frames;

        image->frames = (psx_image_frame*)calloc(image->num_frames, sizeof(psx_image_frame));
        if (!image->frames)
            goto error;

        for (i = 0; i < image->num_frames; i++) {
            size_t size = image->pitch * image->height;
            image->frames[i].size = size;
            image->frames[i].data = (ps_byte*)calloc(1, size);
            image->frames[i].duration = 0;
            if (!image->frames[i].data)
                goto error;

            if (op->decode_image_data(&header, &image->frames[i], i, image->frames[i].data, size) != 0)
                goto error;

            // create ps_image object.
            image->frames[i].img = ps_image_create_with_data(image->frames[i].data, image->format,
                                                             image->width, image->height, image->pitch);
            if (!image->frames[i].img)
                goto error;

            if (header.alpha == 1) // has alpha
                ps_image_set_allow_transparent(image->frames[i].img, True);
        }
    }

    if (op->release_read_header_info)
        op->release_read_header_info(&header);
    return image;

error:
    if (image->frames) {
        for (i = 0; i < image->num_frames; i++) {
            // free buffer pixels data and ps_image object.
            if (image->frames[i].data) {
                if (image->frames[i].img)
                    ps_image_unref(image->frames[i].img);
                free(image->frames[i].data);
            }
        }
        free(image->frames);
    }
    free(image);

    if (op->release_read_header_info)
        op->release_read_header_info(&header);
    return NULL;
}

static int save_psx_image(psx_image_operator* op, const psx_image* image,
                                                  image_writer_fn func, void* param, float quality)
{
    int i;
    int ret = S_OK;
    psx_image_header header;
    if (!op->write_header_info || !op->encode_image_data)
        return S_NOT_SUPPORT;

    if (op->write_header_info(image, func, param, quality, &header) != 0)
        return S_NOT_SUPPORT;

    for (i = 0; i < header.frames; i++) {
        if (op->encode_image_data(&header, &image->frames[i], i, image->frames[i].data, image->frames[i].size, &ret) != 0)
            break;
    }

    if (op->release_write_header_info)
        op->release_write_header_info(&header);
    return ret;
}

psx_image* PICAPI psx_image_load_from_memory(const ps_byte* data, size_t length, int* err_code)
{
    struct image_coder_node* node = NULL;
    psx_image * image = NULL;

    if (!data || !length) {
        if (err_code)
            *err_code = S_BAD_PARAMS;
        return NULL;
    }

    if (!g_modules) {
        if (err_code)
            *err_code = S_INIT_FAILURE;
        return NULL;
    }

    node = get_first_operator(g_modules, data, length);
    if (!node || !node->op) {
        if (err_code)
            *err_code = S_NOT_SUPPORT;
        return NULL;
    }

    while ((image = load_psx_image(node->op, data, length)) == NULL) {
        node = get_next_operator(g_modules, node, data, length);
        if (!node || !node->op) {
            if (err_code)
                *err_code = S_NOT_SUPPORT;
            return NULL;
        }
    }
	if (err_code)
        *err_code = S_OK;
    return image;
}

int PICAPI psx_image_save(const psx_image* image, image_writer_fn func, void* param, const char* type, float quality)
{
    struct image_coder_node* node = NULL;
    int ret = S_OK;
    if (!image || !func || !type || quality <= 0.0f)
        return S_BAD_PARAMS;

    if (!g_modules)
        return S_INIT_FAILURE;

    node = get_first_operator_by_name(g_modules, type);
    if (!node || !node->op)
        return S_NOT_SUPPORT;

    while ((ret = save_psx_image(node->op, image, func, param, quality)) != S_OK) {
        node = get_next_operator_by_name(g_modules, node, type);
        if (!node || !node->op)
            return S_NOT_SUPPORT;
    }
    return ret;
}

static int file_writer(void* param, const ps_byte* data, size_t len)
{
    const pchar* path = (const pchar*)param;
    if (_file_write(path, (unsigned char*)data, len) == 0)
        return S_OK;
    else
        return S_FAILURE;
}

int PICAPI psx_image_save_to_file(const psx_image* image, const char* name, const char* type, float quality)
{
    int ret;
    pchar* file_name;

    if (!image || !name || !type || quality <= 0.0f)
        return S_BAD_PARAMS;
    file_name = pstring_create(name, NULL);
    if (_file_exists(file_name) == 0)
        _file_remove(file_name); // remove old file.
    ret = psx_image_save(image, file_writer, (void*)file_name, type, quality);
    free(file_name);
    return ret;
}

int PICAPI psx_image_destroy(psx_image* image)
{
    size_t i;
    if (!image)
        return S_BAD_PARAMS;

    for (i = 0; i < image->num_frames; i++) {
        // free buffer pixels data and ps_image object.
        if (image->frames[i].img)
            ps_image_unref(image->frames[i].img);
        free(image->frames[i].data);
    }

    free(image->frames);
    free(image);
    return S_OK;
}

