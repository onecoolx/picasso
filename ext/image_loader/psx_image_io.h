/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PSX_IMAGE_IO_H_
#define _PSX_IMAGE_IO_H_

#if defined(WIN32) && defined(_MSC_VER)
#define PATH_MAX  1024
#define strdup(s) _strdup(s)
typedef wchar_t pchar;
#else
typedef char pchar;
#endif

/* release pchar string create by pstring_create needed free() */
pchar* pstring_create(const char* str, size_t * rlen);

int _file_exists(const pchar* path);

size_t _file_size(const pchar* path);

int _file_read(const pchar* path, unsigned char* buffer, size_t buffer_size);

int _file_write(const pchar* path, unsigned char* buffer, size_t buffer_size);

int _file_remove(const pchar* path);


typedef void* module_handle;

#define INVALID_HANDLE  ((module_handle)0)

module_handle _module_load(const pchar* path);

void* _module_get_symbol(module_handle module, const char* name);

void _module_unload(module_handle module);

pchar* _module_get_modules_dir(pchar* path_buffer, size_t buffer_size);

/* if paths == NULL, return modules only, paths[] items will alloc by malloc. */
size_t _module_get_modules(const pchar* dir_path, pchar* paths[], size_t num);

#endif /*_PSX_IMAGE_IO_H_*/

