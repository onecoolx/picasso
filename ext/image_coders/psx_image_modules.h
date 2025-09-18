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

#ifndef _PSX_IMAGE_MODULE_MANAGER_H_
#define _PSX_IMAGE_MODULE_MANAGER_H_

#include "psx_list.h"
#include "psx_image_io.h"
#include "psx_image_plugin.h"

struct image_coder_node {
    struct list_hdr head;
    char* magic_hdr;
    size_t magic_offset;
    size_t magic_len;
    int32_t level;
    char* type_name;
    psx_image_operator* op;
};

struct image_module_node {
    module_handle handle;
    pchar* path;
};

struct image_modules_mgr {
    struct list_hdr coders;
    size_t num_coders;
    struct image_module_node* modules;
    size_t num_modules;
};

int32_t modules_init(struct image_modules_mgr* mgr);

void modules_destroy(struct image_modules_mgr* mgr);

struct image_coder_node* get_first_operator(struct image_modules_mgr* mgr, const ps_byte* data, size_t len);

struct image_coder_node* get_first_operator_by_name(struct image_modules_mgr* mgr, const char* type);

struct image_coder_node* get_next_operator(struct image_modules_mgr* mgr,
                                           struct image_coder_node* op, const ps_byte* data, size_t len);

struct image_coder_node* get_next_operator_by_name(struct image_modules_mgr* mgr,
                                                   struct image_coder_node* op, const char* type);

#endif /*_PSX_IMAGE_MODULE_MANAGER_H_*/
