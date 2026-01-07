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

#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include <new>
#include <stdlib.h>
#include <stdint.h>

#include "global.h"
#include "fastcopy.h"

// common memory managers

#define mem_malloc(n)         (picasso::_global._malloc((n)))
#define mem_calloc(n, s)      (picasso::_global._calloc((n), (s)))
#define mem_free(p)           (picasso::_global._free((p)))

#define mem_deep_copy(d, s, l)    memmove(d, s, l)
#define mem_copy(d, s, l)         fastcopy(d, s, l)

// this can be replace by hw buffer!
#define BufferAlloc(n)         mem_calloc(n, 1)
#define BuffersAlloc(n, s)     mem_calloc(n, s)
#define BufferFree(p)          mem_free(p)
#define BufferCopy(d, s, n)    mem_copy(d, s, n)

#if !ENABLE(SYSTEM_MALLOC) && !COMPILER(CLANG)
#undef new
#undef delete

MAYBE_INLINE void* operator new (size_t size) { return mem_malloc(size); }
MAYBE_INLINE void* operator new[] (size_t size) { return mem_malloc(size); }
MAYBE_INLINE void operator delete (void* p) { mem_free(p); }
MAYBE_INLINE void operator delete[] (void* p) { mem_free(p); }

#endif /*ENABLE(SYSTEM_MALLOC)*/

#endif/*_MEMORY_MANAGER_H_*/
