/*
 * Copyright (c) 2024, Zhang Ji Peng
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

#ifndef _PSX_ARRAY_H_
#define _PSX_ARRAY_H_

#include "psx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef mem_calloc
#define mem_calloc calloc
#endif
#ifndef mem_free
#define mem_free free
#endif

#define PSX_ARRAY_DEFAULT_CAPACITY  4

typedef struct {
    uint8_t* data;
    uint32_t size;
    uint32_t capacity;
    uint32_t element_size;
} psx_array_t;

static INLINE void psx_array_capacity_init(psx_array_t* array, uint32_t capacity, uint32_t element_size)
{
    array->size = 0;
    array->capacity = capacity;
    array->element_size = element_size;
    array->data = (uint8_t*)mem_calloc(capacity, element_size);
}

static INLINE void psx_array_destroy(psx_array_t* array)
{
    if (array->data) {
        mem_free(array->data);
        array->data = NULL;
    }
    array->size = 0;
    array->capacity = 0;
}

static INLINE uint32_t psx_array_size(const psx_array_t* array)
{
    return array->size;
}

static INLINE uint32_t psx_array_capacity(const psx_array_t* array)
{
    return array->capacity;
}

static INLINE void psx_array_clear(psx_array_t* array)
{
    array->size = 0;
}

static INLINE bool psx_array_empty(const psx_array_t* array)
{
    return array->size == 0;
}

static INLINE void* psx_array_at(const psx_array_t* array, uint32_t index)
{
    if (index >= array->size) {
        return NULL;
    }
    return array->data + index * array->element_size;
}

static INLINE bool psx_array_resize(psx_array_t* array, uint32_t capacity)
{
    uint8_t* data = (uint8_t*)mem_calloc(capacity, array->element_size);
    if (!data) {
        return false;
    }

    array->capacity = capacity;
    if (array->size > capacity) {
        array->size = capacity;
    }

    memcpy(data, array->data, array->size * array->element_size);
    mem_free(array->data);
    array->data = data;
    return true;
}

static INLINE void psx_array_shrink(psx_array_t* array)
{
    if (array->size <= (array->capacity >> 1)) {
        psx_array_resize(array, array->size);
    }
}

static INLINE bool psx_array_remove(psx_array_t* array, uint8_t index)
{
    if (index >= array->size) {
        return false;
    }

    if (index == array->size - 1) {
        array->size--;
        return true;
    }

    uint8_t* target = (uint8_t*)psx_array_at(array, index);
    uint8_t* start = target + array->element_size;
    uint32_t copy_size = (array->size - index - 1) * array->element_size;
    memmove(target, start, copy_size);
    array->size--;
    return true;
}

static INLINE bool psx_array_append(psx_array_t* array, const void* value)
{
    if (array->size == array->capacity) {
        if (!psx_array_resize(array, (array->capacity << 1))) {
            return false;
        }
    }

    if (value) {
        memcpy(array->data + array->size * array->element_size, value, array->element_size);
    }
    array->size++;
    return true;
}

#define psx_array_init(array, element_size) \
    psx_array_capacity_init((array), PSX_ARRAY_DEFAULT_CAPACITY, (element_size))

#define psx_array_init_type(array, type) \
    psx_array_init((array), sizeof(type))

#define psx_array_get(array, i, type) \
    (type*)psx_array_at((array), (i))

#define psx_array_get_last(array, type) \
    psx_array_get((array), psx_array_size((array)) - 1, type)

#define psx_array_push_back(array, value) \
    psx_array_append((array), (void*)&(value))

#define psx_array_remove_last(array) \
    psx_array_remove((array), psx_array_size(array) - 1)

#ifdef __cplusplus
}
#endif

#endif /*_PSX_ARRAY_H_*/
