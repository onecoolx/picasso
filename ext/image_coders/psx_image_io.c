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

module_handle _module_load(const wchar_t* path)
{
    HMODULE dl = LoadLibraryW(path);
    if (!dl) {
        fwprintf(stderr, L"Load Module [%s] failed code: %x\n", path, (int32_t)GetLastError());
    }
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
    wchar_t* p = 0;
    *rlen = (size_t)GetModuleFileName(NULL, buffer, (DWORD)len);
    p = wcsrchr(buffer, '\\');
    p++;
    *p = 0;
    return buffer;
}

static int32_t _directory_is_exists(const wchar_t* path)
{
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(path, &fd);
    BOOL b = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? TRUE : FALSE;
    int32_t ret = ((h != INVALID_HANDLE_VALUE) && b) ? 0 : -1;
    FindClose(h);
    return ret;
}

wchar_t* _module_get_modules_dir(wchar_t* path_buffer, size_t buffer_size)
{
    size_t len = 0;
    wchar_t* current_path = _get_current_path(path_buffer, buffer_size, &len);
    if ((len + 24) >= buffer_size) {
        return NULL; //path is too long.
    }

    lstrcat (current_path, L"modules");

    if (_directory_is_exists(current_path) == 0) {
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
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            continue;
        }

        if (paths && (nums < size)) {
            size_t len = lstrlen(fd.cFileName);
            wchar_t* path = (wchar_t*)calloc((dir_path_len + len + 2), sizeof(wchar_t));
            if (path) {
                lstrcpy(path, dir_path);
                lstrcat(path, L"\\");
                lstrcat(path, fd.cFileName);
                paths[nums] = path;
            }
        }
        nums++;

        if (!FindNextFile(handle, &fd)) {
            break;
        }
    }
    FindClose(handle);
    return nums;
}

#else
#ifdef __APPLE__
    #define SO_EXT "dylib"
#else
    #define SO_EXT "so"
#endif

size_t _module_get_modules(const char* dir_path, char* paths[], size_t size)
{
    DIR* dir = NULL;
    struct dirent* de = NULL;
    size_t nums = 0;
    size_t dir_path_len = 0;

    dir = opendir(dir_path);
    if (!dir) {
        return 0;
    }

    dir_path_len = strlen(dir_path);
    while ((de = readdir(dir)) != NULL) {
        if (fnmatch("*."SO_EXT, de->d_name, 0) != 0) {
            continue;
        }

        if (paths && (nums < size)) {
            size_t len = strlen(de->d_name);
            char* path = (char*)malloc(dir_path_len + len + 1);
            if (path) {
                memcpy(path, dir_path, dir_path_len);
                memcpy(path + dir_path_len, de->d_name, len + 1);
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

    lib_paths = (char*)getenv("PS_IMAGE_MODULES_DIR");
    if (lib_paths) {
        return lib_paths;
    }

    if ((stat("/usr/lib/modules", &info) == 0) && S_ISDIR(info.st_mode)) {
        if (buffer_size > 24) {
            strncpy(path_buffer, "/usr/lib/modules/", buffer_size);
            return path_buffer;
        } else {
            return NULL;
        }
    }

    if ((stat("/usr/local/lib/modules", &info) == 0) && S_ISDIR(info.st_mode)) {
        if (buffer_size > 32) {
            strncpy(path_buffer, "/usr/local/lib/modules/", buffer_size);
            return path_buffer;
        } else {
            return NULL;
        }
    }

    lib_paths = (char*)getenv("LD_LIBRARY_PATH");
    if (!lib_paths) {
        return NULL;
    }

    length = strlen(lib_paths);
    if (!length) {
        return NULL;
    }

    while (length > 0) {
        memset(path_buffer, 0, buffer_size);

        path_seek = strchr(lib_paths, ':');
        if (!path_seek) { //end of string
            if ((length + 16) > buffer_size) {
                return NULL; //path_buffer too small.
            }

            strncpy(path_buffer, lib_paths, buffer_size);

            if (path_buffer[length - 1] != '/') {
                path_buffer[length] = '/';
            }

            strcat(path_buffer, "modules/");

            if ((stat(path_buffer, &info) == 0) && S_ISDIR(info.st_mode)) {
                return path_buffer;
            }

            length -= length;
        } else {
            size_t len = path_seek - lib_paths;
            if ((len + 16) > buffer_size) {
                return NULL; //path_buffer too small.
            }

            strncpy(path_buffer, lib_paths, len);

            if (path_buffer[len - 1] != '/') {
                path_buffer[len] = '/';
            }

            strcat(path_buffer, "modules/");

            if ((stat(path_buffer, &info) == 0) && S_ISDIR(info.st_mode)) {
                return path_buffer;
            }

            length -= len;
            lib_paths = path_seek + 1;
        }
    }
    return NULL;
}

module_handle _module_load(const char* path)
{
    void* dl = dlopen(path, RTLD_LAZY);
    if (!dl) {
        LOG_ERROR("Load Module [%s] failed: %s\n", path, dlerror());
    }
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

#endif
