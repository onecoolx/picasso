
/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
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

int modules_init(struct image_modules_mgr* mgr)
{
    pchar* dir_path = NULL;
    pchar** mod_paths = NULL;
    size_t nums = 0, i = 0, n = 0;
    mod_func func = NULL;
    memset(modules_dir, 0, sizeof(pchar) * PATH_MAX);

    //init coder list.
    if (list_init(&(mgr->coders)) != 0){
        fprintf(stderr, "internal error!\n");
        return -1;
    }

    dir_path = _module_get_modules_dir(modules_dir, PATH_MAX);
    if (!dir_path) {
        fprintf(stderr, "no image modules directory found!\n");
        return -1;
    }

    nums = _module_get_modules(modules_dir, NULL, 0);
    if (!nums) {
        fprintf(stderr, "no image modules found!\n");
        return -1;
    }

    mgr->modules = (struct image_module_node*)calloc(nums, sizeof(struct image_module_node));
    mod_paths = (pchar**)calloc(nums, sizeof(pchar*));
    _module_get_modules(modules_dir, mod_paths, nums);

    for (i = 0, n = 0; i < nums; i++) {
        pchar * ps = mod_paths[i];
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
        if (func)
            func(); // call module deinit
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
        if (memcmp(data, entry->magic_hdr, entry->magic_len) == 0)
            break;
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
        if (strncmp(type, entry->type_name, strlen(entry->type_name)) == 0)
            break;
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
        if (memcmp(data, entry->magic_hdr, entry->magic_len) == 0)
            break;
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
        if (strncmp(type, entry->type_name, strlen(entry->type_name)) == 0)
            break;
        entry = NULL;
    }

    return entry;
}

static char* copy_magic(const char* str, size_t len)
{
    char* dst = (char*)calloc(len+1, sizeof(char)); //malloc and clear
    memcpy(dst, str, len);
    return dst;
}

int psx_image_register_operator(const char* type, const ps_byte* header_magic, size_t magic_len,
        psx_priority_level level, psx_image_operator* coder)
{
    size_t len = 0;
    struct image_modules_mgr* mgr = NULL;
    struct list_hdr* ptr = NULL;
    struct image_coder_node* entry = NULL;

    if (!type || !header_magic || !magic_len || !coder)
        return S_BAD_PARAMS;

    mgr = _get_modules();
    if (!mgr)
        return S_INIT_FAILURE;

    list_for_each(&(mgr->coders), ptr) {
        entry = (struct image_coder_node*)ptr;
        len = entry->magic_len > magic_len ? entry->magic_len : magic_len;
        if (entry && (memcmp(entry->magic_hdr, header_magic, len) == 0))
            break;
        entry = NULL;
    }

    if (entry) {
        struct image_coder_node * new_entry = (struct image_coder_node*)calloc(1, sizeof(struct image_coder_node));
        new_entry->magic_hdr = copy_magic((const char*)header_magic, magic_len);
        new_entry->magic_len = magic_len;
        new_entry->level = (int)level;
        new_entry->type_name = strdup(type);
        new_entry->op = coder;

        if (level == PRIORITY_MASTER)
            list_add_tail(entry, new_entry);
        else if (level == PRIORITY_EXTENTED)
            list_add_tail(&(mgr->coders), new_entry);
        else
            list_add(entry, new_entry);

    } else {
        entry = (struct image_coder_node*)calloc(1, sizeof(struct image_coder_node));
        entry->magic_hdr = copy_magic((const char*)header_magic, magic_len);
        entry->magic_len = magic_len;
        entry->level = (int)level;
        entry->type_name = strdup(type);
        entry->op = coder;

        list_add(&(mgr->coders), entry);
    }

    mgr->num_coders++;
    return S_OK;
}


int psx_image_unregister_operator(psx_image_operator* coder)
{
    struct image_modules_mgr* mgr = NULL;
    struct list_hdr* ptr = NULL;
    struct image_coder_node* entry = NULL;

    if (!coder)
        return S_BAD_PARAMS;

    mgr = _get_modules();
    if (!mgr)
        return S_INIT_FAILURE;

    if (list_empty(&(mgr->coders)) == 0)
        return S_OK;

    list_for_each(&(mgr->coders), ptr) {
        entry = (struct image_coder_node*)ptr;
        if (entry && (entry->op == coder))
            break;
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


