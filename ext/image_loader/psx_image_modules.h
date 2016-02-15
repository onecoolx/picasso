/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PSX_IMAGE_MODULE_MANAGER_H_
#define _PSX_IMAGE_MODULE_MANAGER_H_

#include "psx_list.h"
#include "psx_image_io.h"
#include "picasso_image_plugin.h"

struct image_coder_node {
    struct list_hdr head;
    char*       magic_hdr;
    size_t      magic_len;
    int         level;
    char*       type_name;
    psx_image_operator* op;
};

struct image_module_node {
    module_handle handle;
    pchar*         path;
};

struct image_modules_mgr {
    struct list_hdr     coders;
    size_t          num_coders;
    struct image_module_node* modules;
    size_t          num_modules;
};


int modules_init(struct image_modules_mgr* mgr);

void modules_destroy(struct image_modules_mgr* mgr);

struct image_coder_node* get_first_operator(struct image_modules_mgr* mgr, const ps_byte* data, size_t len);

struct image_coder_node* get_first_operator_by_name(struct image_modules_mgr* mgr, const char* type);

struct image_coder_node* get_next_operator(struct image_modules_mgr* mgr,
                                           struct image_coder_node* op, const ps_byte* data, size_t len);

struct image_coder_node* get_next_operator_by_name(struct image_modules_mgr* mgr,
                                                   struct image_coder_node* op, const char* type);

#endif /*_PSX_IMAGE_MODULE_MANAGER_H_*/

