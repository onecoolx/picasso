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

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "psx_linear_allocator.h"

#define LINEAR_DEFAULT_BLOCK_SIZE    (1024) // 1kb
#define MAX_BLOCK_SIZE   ((size_t) 65536) // 64kb

#define MEM_ALIGN(x, shift) \
    (((x) + ((shift) - 1)) & ~(shift - 1))

typedef struct _mem_block {
    struct _mem_block* next;
    uint32_t block_size;
    uint8_t data[1];
} mem_block_t;

typedef struct {
    psx_linear_allocator base;
    mem_block_t* blocks;
    mem_block_t* cur_block;
    void* next_ptr;
    psx_memory_align_t align;
} linear_allocator_impl;

static INLINE bool fits_block(linear_allocator_impl* mem, size_t size)
{
    return mem->next_ptr &&
           (((uint8_t*)mem->next_ptr) + size) <= (((uint8_t*)mem->cur_block) + mem->cur_block->block_size);
}

static INLINE void ensure_next(linear_allocator_impl* mem, size_t size)
{
    if (fits_block(mem, size)) {
        return;
    }

    size_t block_size = MEM_ALIGN(LINEAR_DEFAULT_BLOCK_SIZE, PSX_LINEAR_ALIGN_DEFAULT);
    size_t block_hdr_size = sizeof(mem_block_t*) + sizeof(uint32_t);

    while ((block_size - block_hdr_size) < size) {
        block_size *= 2;
    }

    mem->base.total_memory += block_size;

    mem_block_t* block = mem_malloc(block_size);
    if (!block) {
        LOG_ERROR("Out of memory!\n");
        ABORT();
        return;
    }
    block->next = NULL;
    block->block_size = (uint32_t)block_size;

    if (mem->cur_block) {
        mem->cur_block->next = block;
    }

    mem->cur_block = block;

    if (!mem->blocks) {
        mem->blocks = mem->cur_block;
    }

    mem->next_ptr = (void*)(&block->data);
    mem->next_ptr = (void*)MEM_ALIGN((uintptr_t)mem->next_ptr, (uintptr_t)mem->align);
}

static INLINE void* _linear_alloc(struct _linear_allocator* mem, size_t size)
{
    if (size > MAX_BLOCK_SIZE) {
        LOG_ERROR("Allocating more than 64kb of memory using the linear allocator is not allowed!\n");
        return NULL;
    }

    linear_allocator_impl* p = (linear_allocator_impl*)mem;
    size = MEM_ALIGN(size, p->align);
    ensure_next(p, size);
    void* ptr = p->next_ptr;
    p->next_ptr = ((uint8_t*)p->next_ptr) + size;
    return ptr;
}

psx_linear_allocator* psx_linear_allocator_create(psx_memory_align_t align)
{
    linear_allocator_impl* mem = (linear_allocator_impl*)mem_malloc(sizeof(linear_allocator_impl));
    ASSERT(mem != NULL);
    if (!mem) {
        return NULL;
    }
    memset(mem, 0, sizeof(linear_allocator_impl));

    mem->base.alloc = _linear_alloc;
    mem->base.total_memory = 0;
    mem->align = align;

    return (psx_linear_allocator*)mem;
}

void psx_linear_allocator_destroy(psx_linear_allocator* mem)
{
    ASSERT(mem != NULL);
    if (!mem) {
        return;
    }

    mem_block_t* b = ((linear_allocator_impl*)mem)->blocks;

    while (b) {
        mem_block_t* next = b->next;
        mem_free(b);
        b = next;
    }
    mem_free(mem);
}
