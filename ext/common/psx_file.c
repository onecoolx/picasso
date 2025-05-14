/*
 * Copyright (c) 2025, Zhang Ji Peng
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

#include "psx_file.h"

#include <sys/stat.h>
#if defined(WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
    #include <dlfcn.h>
    #include <fnmatch.h>
    #include <dirent.h>
#endif

#if defined(WIN32) && defined(_MSC_VER)

wchar_t* psx_path_create(const char* str, size_t* rlen)
{
    wchar_t* ustr = NULL;
    size_t len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (!len) {
        return NULL;
    }

    ustr = (wchar_t*)mem_malloc((len + 1) * sizeof(wchar_t));
    memset(ustr, 0, sizeof(wchar_t) * (len + 1));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, ustr, (int)len);

    if (rlen) { *rlen = len; }
    return ustr;
}

void psx_path_destroy(wchar_t* pstr)
{
    if (pstr) {
        mem_free(pstr);
    }
}

#define STAT(p, i) _wstat(p, i)
#define FOPEN(p, m) _wfopen(p, L##m)

bool psx_file_exists(const wchar_t* path)
{
    struct _stat info;
    if (STAT(path, &info) != 0) {
        return false;
    }

    return S_IFREG & info.st_mode ? true : false;
}

size_t psx_file_size(const wchar_t* path)
{
    struct _stat info;
    if (STAT(path, &info) != 0) {
        return 0;
    }

    return info.st_size;
}

bool psx_file_remove(const wchar_t* path)
{
    return _wremove(path) == 0 ? true : false;
}

#else

char* psx_path_create(const char* str, size_t* rlen)
{
    if (rlen) { *rlen = strlen(str); }
    return (char*)str;
}

void psx_path_destroy(char* pstr)
{
}

bool psx_file_exists(const char* path)
{
    struct stat info;
    if (stat(path, &info) != 0) {
        return false;
    }

    return S_ISREG(info.st_mode) ? true : false;
}

size_t psx_file_size(const char* path)
{
    struct stat info;
    if (stat(path, &info) != 0) {
        return 0;
    }

    return info.st_size;
}

bool psx_file_remove(const char* path)
{
    return remove(path) == 0 ? true : false;
}

#define FOPEN(p, m) fopen(p, m)

#endif

bool psx_file_read(const pchar* path, uint8_t* buffer, size_t buffer_size)
{
    size_t read_bytes = 0;
    bool retval = true;

    FILE* fp = FOPEN(path, "rb");
    if (!fp) {
        return false;
    }

    read_bytes = fread(buffer, 1, buffer_size, fp);
    if (read_bytes != buffer_size) {
        retval = false;
    }

    fclose(fp);
    return retval;
}

bool psx_file_write(const pchar* path, const uint8_t* buffer, size_t buffer_size)
{
    size_t write_bytes = 0;
    bool retval = true;

    FILE* fp = FOPEN(path, "ab");
    if (!fp) {
        return false;
    }

    write_bytes = fwrite(buffer, 1, buffer_size, fp);
    if (write_bytes != buffer_size) {
        retval = false;
    }

    fclose(fp);
    return retval;
}
