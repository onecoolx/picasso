/*
 * Copyright (c) 2026, Zhang Ji Peng
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

#include "common.h"

#include "global.h"
#include "memory_manager.h"

void* operator new (size_t size)
{
    return mem_malloc(size);
}

void* operator new[] (size_t size)
{
    return mem_malloc(size);
}

void operator delete (void* p)
{
    mem_free(p);
}

void operator delete[] (void* p)
{
    mem_free(p);
}

void* operator new (size_t size, const std::nothrow_t&) noexcept
{
    return mem_malloc(size);
}

void* operator new[] (size_t size, const std::nothrow_t&) noexcept
{
    return mem_malloc(size);
}

void operator delete (void* p, const std::nothrow_t&) noexcept
{
    mem_free(p);
}

void operator delete[] (void* p, const std::nothrow_t&) noexcept
{
    mem_free(p);
}

#if !ENABLE(SYSTEM_MALLOC)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void* _internal_malloc(size_t size)
{
    return malloc(size);
}

static void _internal_free(void* ptr)
{
    free(ptr);
}

static void* _internal_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

namespace picasso {

struct global_data _global(_internal_malloc, _internal_free, _internal_calloc);

} // namespace picasso

#endif // ENABLE(SYSTEM_MALLOC)
