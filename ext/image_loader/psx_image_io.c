/* Picasso - a vector graphics library
 *
 * Copyright (C) 2016 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/stat.h>
#if defined(WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#include <fnmatch.h>
#include <dirent.h>
#endif

#include "psx_image_io.h"

#if defined(WIN32) && defined(_MSC_VER)

wchar_t* pstring_create(const char* str, size_t * rlen)
{
    wchar_t* ustr = NULL;
    size_t len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (!len)
        return NULL;

    ustr = (wchar_t*)malloc((len+1) * sizeof(wchar_t));
    memset(ustr, 0, sizeof(wchar_t) * (len+1));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, ustr, len);

    if (rlen) *rlen = len;
    return ustr;
}

#define STAT(p, i) _wstat(p, i)
#define FOPEN(p, m) _wfopen(p, L##m)

int _file_exists(const wchar_t* path)
{
    struct _stat info;
    if (STAT(path, &info) != 0)
        return -1;

    return S_IFREG &info.st_mode ? 0 : -1;
}

size_t _file_size(const wchar_t* path)
{
    struct _stat info;
    if (STAT(path, &info) != 0)
        return 0;

    return info.st_size;
}

int _file_remove(const wchar_t* path)
{
    return _wremove(path);
}

module_handle _module_load(const wchar_t* path)
{
    HMODULE dl = LoadLibraryW(path);
    if (!dl)
        fwprintf(stderr, L"Load Module [%s] failed code: %x\n", path, (int)GetLastError());
    return (module_handle)dl;
}

void* _module_get_symbol(module_handle module, const char* name)
{
    return GetProcAddress((HMODULE)module, name);
}

void _module_unload(module_handle module)
{
    FreeLibrary((HMODULE)module);
}

static wchar_t* _get_current_path(wchar_t* buffer, size_t len, size_t* rlen)
{
    wchar_t *p = 0;
    *rlen = GetModuleFileName(NULL, buffer, len);
    p = wcsrchr(buffer, '\\');
    p++; *p = 0;
    return buffer;
}

static int _directory_is_exists(const wchar_t* path)
{
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(path, &fd);
    BOOL b = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? TRUE : FALSE;
    int ret = ((h != INVALID_HANDLE_VALUE) && b) ? 0 : -1;
    FindClose(h);
    return ret;
}

wchar_t* _module_get_modules_dir(wchar_t* path_buffer, size_t buffer_size)
{
    size_t len = 0;
    wchar_t * current_path = _get_current_path(path_buffer, buffer_size, &len);
    if ((len + 24) >= buffer_size)
        return NULL; //path is too long.

    lstrcat (current_path, L"modules");

    if (_directory_is_exists(current_path) == 0){
        return path_buffer;
    } else {
        return NULL;
    }
}

size_t _module_get_modules(const wchar_t* dir_path, wchar_t* paths[], size_t size)
{
    WIN32_FIND_DATA fd;
    HANDLE handle;
    size_t nums = 0;
    size_t dir_path_len = 0;

    wchar_t find_path[PATH_MAX] = {0};
    lstrcpy(find_path, dir_path);
    lstrcat(find_path, L"\\*.dll");

    handle = FindFirstFile(find_path, &fd);
    if (handle == INVALID_HANDLE_VALUE) {
        FindClose(handle);
        return 0;
    }

    dir_path_len = lstrlen(dir_path);
    while (1) {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (paths && (nums < size)) {
            size_t len = lstrlen(fd.cFileName);
            wchar_t* path = (wchar_t*)calloc((dir_path_len+len+2), sizeof(wchar_t));
            if (path) {
                lstrcpy(path, dir_path);
                lstrcat(path, L"\\");
                lstrcat(path, fd.cFileName);
                paths[nums] = path;
            }
        }
        nums++;

        if (!FindNextFile(handle, &fd))
           break;
    }
    FindClose(handle);
    return nums;
}
#else
int _file_exists(const char* path)
{
    struct stat info;
    if (stat(path, &info) != 0)
        return -1;

    return S_ISREG(info.st_mode) ? 0 : -1;
}

size_t _file_size(const char* path)
{
    struct stat info;
    if (stat(path, &info) != 0)
        return 0;

    return info.st_size;
}

int _file_remove(const char* path)
{
    return remove(path);
}

size_t _module_get_modules(const char* dir_path, char* paths[], size_t size)
{
    DIR* dir = NULL;
    struct dirent* de = NULL;
    size_t nums = 0;
    size_t dir_path_len = 0;

    dir = opendir(dir_path);
    if (!dir)
        return 0;

    dir_path_len = strlen(dir_path);
    while ((de = readdir(dir)) != NULL) {
        if (fnmatch("*.so", de->d_name, 0) != 0)
            continue;

        if (paths && (nums < size)) {
            size_t len = strlen(de->d_name);
            char* path = (char*)malloc(dir_path_len+len+1);
            if (path) {
                memcpy(path, dir_path, dir_path_len);
                memcpy(path+dir_path_len, de->d_name, len+1);
                paths[nums] = path;
            }
        }
        nums++;
    }
    closedir(dir);
    return nums;
}

char* _module_get_modules_dir(char* path_buffer, size_t buffer_size)
{
    struct stat info;
    char* lib_paths = NULL;
    char* path_seek = NULL;
    size_t length = 0;

    if ((stat("/usr/lib/modules", &info) == 0) && S_ISDIR(info.st_mode)){
        if (buffer_size > 24) {
            strncpy(path_buffer, "/usr/lib/modules/", buffer_size);
            return path_buffer;
        } else
            return NULL;
    }

    if ((stat("/usr/local/lib/modules", &info) == 0) && S_ISDIR(info.st_mode)){
        if (buffer_size > 32) {
            strncpy(path_buffer, "/usr/local/lib/modules/", buffer_size);
            return path_buffer;
        } else
            return NULL;
    }

    lib_paths = (char*)getenv("LD_LIBRARY_PATH");
    if (!lib_paths)
        return NULL;

    length = strlen(lib_paths);
    if (!length)
        return NULL;

    while (length > 0) {
        memset(path_buffer, 0, buffer_size);

        path_seek = strchr(lib_paths, ':');
        if (!path_seek) { //end of string
            if ((length + 16) > buffer_size)
                return NULL; //path_buffer too small.

            strncpy(path_buffer, lib_paths, buffer_size);

            if (path_buffer[length-1] != '/')
                path_buffer[length] = '/';

            strcat(path_buffer, "modules/");

            if ((stat(path_buffer, &info) == 0) && S_ISDIR(info.st_mode))
                return path_buffer;

            length -= length;
        } else {
            size_t len = path_seek - lib_paths;
            if ((len + 16) > buffer_size)
                return NULL; //path_buffer too small.

            strncpy(path_buffer, lib_paths, len);

            if (path_buffer[len-1] != '/')
                path_buffer[len] = '/';

            strcat(path_buffer, "modules/");

            if ((stat(path_buffer, &info) == 0) && S_ISDIR(info.st_mode))
                return path_buffer;

            length -= len;
            lib_paths = path_seek + 1;
        }
    }
    return NULL;
}

module_handle _module_load(const char* path)
{
    void* dl = dlopen(path, RTLD_LAZY);
    if (!dl)
        fprintf(stderr, "Load Module [%s] failed: %s\n", path, dlerror());
    return dl;
}

void* _module_get_symbol(module_handle module, const char* name)
{
    return dlsym(module, name);
}

void _module_unload(module_handle module)
{
    dlclose(module);
}

char* pstring_create(const char* str, size_t * rlen)
{
    if (rlen) *rlen = strlen(str);
    return strdup(str);
}

#define FOPEN(p, m) fopen(p, m)

#endif

int _file_read(const pchar* path, unsigned char* buffer, size_t buffer_size)
{
    size_t read_bytes = 0;
    int retval = 0;

    FILE* fp = FOPEN(path, "rb");
    if (!fp)
        return -1;

    read_bytes = fread(buffer, 1, buffer_size, fp);
    if (read_bytes != buffer_size){
        retval = -1;
    }

    fclose(fp);
    return retval;
}

int _file_write(const pchar* path, unsigned char* buffer, size_t buffer_size)
{
    size_t write_bytes = 0;
    int retval = 0;

    FILE* fp = FOPEN(path, "ab");
    if (!fp)
        return -1;

    write_bytes = fwrite(buffer, 1, buffer_size, fp);
    if (write_bytes != buffer_size){
        retval = -1;
    }

    fclose(fp);
    return retval;
}

