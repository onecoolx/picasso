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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "psx_image_io.h"
#include "psx_image_loader.h"
#include "psx_image_modules.h"

typedef void (*mod_func)(void);

static pchar modules_dir[PATH_MAX];

int32_t modules_init(struct image_modules_mgr* mgr)
{
    pchar* dir_path = NULL;
    pchar** mod_paths = NULL;
    size_t nums = 0, i = 0, n = 0;
    mod_func func = NULL;
    memset(modules_dir, 0, sizeof(pchar) * PATH_MAX);

    //init coder list.
    list_init(&(mgr->coders));

    dir_path = _module_get_modules_dir(modules_dir, PATH_MAX);
    if (!dir_path) {
        LOG_ERROR("No image modules directory found! you can set the `PS_IMAGE_MODULES_DIR` environment variable to the modules path.\n");
        return -1;
    }

    nums = _module_get_modules(dir_path, NULL, 0);
    if (!nums) {
        LOG_ERROR("No image modules found! you can set the `PS_IMAGE_MODULES_DIR` environment variable to the modules path.\n");
        return -1;
    }

    mgr->modules = (struct image_module_node*)calloc(nums, sizeof(struct image_module_node));
    mod_paths = (pchar**)calloc(nums, sizeof(pchar*));
    _module_get_modules(dir_path, mod_paths, nums);

    for (i = 0, n = 0; i < nums; i++) {
        pchar* ps = mod_paths[i];
        module_handle h = _module_load(ps);
        // init module
        func = _module_get_symbol(h, "psx_image_module_init");
        if (func) {
            func(); // call module init
            mgr->modules[n].path = ps;
            mgr->modules[n].handle = h;
            n++;
        } else {
            // unload other module and free path.
            _module_unload(h);
            free(ps);
        }
    }
    mgr->num_modules = n;

    free(mod_paths);
    return 0;
}

void modules_destroy(struct image_modules_mgr* mgr)
{
    size_t i = 0;

    for (i = 0; i < mgr->num_modules; i++) {
        mod_func func = _module_get_symbol(mgr->modules[i].handle, "psx_image_module_shutdown");
        if (func) {
            func(); // call module deinit
        }
        _module_unload(mgr->modules[i].handle);
        free(mgr->modules[i].path);
    }
    free(mgr->modules);
}

struct image_coder_node* get_first_operator(struct image_modules_mgr* mgr, const ps_byte* data, size_t len)
{
    struct list_hdr* ptr = NULL;
    struct image_coder_node* entry = NULL;

    list_for_each(&(mgr->coders), ptr) {
        entry = (struct image_coder_node*)ptr;
        if (entry && (len > (entry->magic_offset + entry->magic_len))
            && memcmp(data + entry->magic_offset, entry->magic_hdr, entry->magic_len) == 0) {
            break;
        }
        entry = NULL;
    }

    return entry;
}

struct image_coder_node* get_first_operator_by_name(struct image_modules_mgr* mgr, const char* type)
{
    struct list_hdr* ptr = NULL;
    struct image_coder_node* entry = NULL;

    list_for_each(&(mgr->coders), ptr) {
        entry = (struct image_coder_node*)ptr;
        if (strncmp(type, entry->type_name, strlen(entry->type_name)) == 0) {
            break;
        }
        entry = NULL;
    }

    return entry;
}

struct image_coder_node* get_next_operator(struct image_modules_mgr* mgr, struct image_coder_node* node, const ps_byte* data, size_t len)
{
    struct list_hdr* ptr = NULL;
    struct image_coder_node* entry = NULL;

    list_for_each_start_with(&(mgr->coders), node, ptr) {
        entry = (struct image_coder_node*)ptr;
        if (memcmp(data + entry->magic_offset, entry->magic_hdr, entry->magic_len) == 0) {
            break;
        }
        entry = NULL;
    }

    return entry;
}

struct image_coder_node* get_next_operator_by_name(struct image_modules_mgr* mgr, struct image_coder_node* node, const char* type)
{
    struct list_hdr* ptr = NULL;
    struct image_coder_node* entry = NULL;

    list_for_each_start_with(&(mgr->coders), node, ptr) {
        entry = (struct image_coder_node*)ptr;
        if (strncmp(type, entry->type_name, strlen(entry->type_name)) == 0) {
            break;
        }
        entry = NULL;
    }

    return entry;
}

static char* copy_magic(const char* str, size_t len)
{
    char* dst = (char*)calloc(len + 1, sizeof(char)); //malloc and clear
    memcpy(dst, str, len);
    return dst;
}

int32_t psx_image_register_operator(const char* type, const ps_byte* header_magic,
                                    size_t magic_offset, size_t magic_len, psx_priority_level level, psx_image_operator* coder)
{
    struct image_modules_mgr* mgr = NULL;
    struct list_hdr* ptr = NULL;
    struct image_coder_node* entry = NULL;

    if (!type || !header_magic || !magic_len || !coder) {
        return S_BAD_PARAMS;
    }

    mgr = _get_modules();
    if (!mgr) {
        return S_INIT_FAILURE;
    }

    list_for_each(&(mgr->coders), ptr) {
        entry = (struct image_coder_node*)ptr;
        if (entry && entry->magic_len == magic_len) {
            if ((memcmp(entry->magic_hdr, header_magic, magic_len) == 0) && (magic_offset == entry->magic_offset)) {
                break;
            }
        }
        entry = NULL;
    }

    if (entry) {
        struct image_coder_node* new_entry = (struct image_coder_node*)calloc(1, sizeof(struct image_coder_node));
        new_entry->magic_hdr = copy_magic((const char*)header_magic, magic_len);
        new_entry->magic_offset = magic_offset;
        new_entry->magic_len = magic_len;
        new_entry->level = (int32_t)level;
        new_entry->type_name = strdup(type);
        new_entry->op = coder;

        if (level == PRIORITY_MASTER) {
            list_add_tail(entry, new_entry);
        } else if (level == PRIORITY_EXTENTED) {
            list_add_tail(&(mgr->coders), new_entry);
        } else {
            list_add(entry, new_entry);
        }

    } else {
        entry = (struct image_coder_node*)calloc(1, sizeof(struct image_coder_node));
        entry->magic_hdr = copy_magic((const char*)header_magic, magic_len);
        entry->magic_offset = magic_offset;
        entry->magic_len = magic_len;
        entry->level = (int32_t)level;
        entry->type_name = strdup(type);
        entry->op = coder;

        list_add(&(mgr->coders), entry);
    }

    mgr->num_coders++;
    return S_OK;
}

int32_t psx_image_unregister_operator(psx_image_operator* coder)
{
    struct image_modules_mgr* mgr = NULL;
    struct list_hdr* ptr = NULL;
    struct image_coder_node* entry = NULL;

    if (!coder) {
        return S_BAD_PARAMS;
    }

    mgr = _get_modules();
    if (!mgr) {
        return S_INIT_FAILURE;
    }

    if (list_empty(&(mgr->coders))) {
        return S_OK;
    }

    list_for_each(&(mgr->coders), ptr) {
        entry = (struct image_coder_node*)ptr;
        if (entry && (entry->op == coder)) {
            break;
        }
        entry = NULL;
    }

    if (entry) {
        list_remove(entry);
        // free node
        free(entry->magic_hdr);
        free(entry->type_name);
        free(entry);
        mgr->num_coders--;
    }
    return S_OK;
}
