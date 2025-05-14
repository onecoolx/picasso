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

#ifndef _PSX_LINEAR_ALLOCATOR_H_
#define _PSX_LINEAR_ALLOCATOR_H_

#include "psx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PSX_LINEAR_ALIGN_1 = 1,
    PSX_LINEAR_ALIGN_2 = (1 << 1),
    PSX_LINEAR_ALIGN_4 = (1 << 2),
    PSX_LINEAR_ALIGN_8 = (1 << 3),
    PSX_LINEAR_ALIGN_16 = (1 << 4),
    PSX_LINEAR_ALIGN_32 = (1 << 5),
    PSX_LINEAR_ALIGN_64 = (1 << 6),
} psx_memory_align_t;

#if defined(_M_X64) || defined(__x86_64__) || defined(__aarch64__)
#define PSX_LINEAR_ALIGN_DEFAULT PSX_LINEAR_ALIGN_8
#else // 32bit system
#define PSX_LINEAR_ALIGN_DEFAULT PSX_LINEAR_ALIGN_4
#endif

typedef struct _linear_allocator {
    size_t total_memory;
    void* (* alloc)(struct _linear_allocator* allocator, size_t size);
} psx_linear_allocator;

psx_linear_allocator* psx_linear_allocator_create(psx_memory_align_t align);

void psx_linear_allocator_destroy(psx_linear_allocator* allocator);

#ifdef __cplusplus
}
#endif

#endif /* _PSX_LINEAR_ALLOCATOR_H_ */
