/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include <new>
#include <stdlib.h>
#include <stdint.h>

#include "fastcopy.h"

// common memory managers

#define mem_malloc(n)         malloc(n)
#define mem_calloc(n, s)      calloc(n, s)
#define mem_realloc(p, s)     realloc(p, s)
#define mem_free(p)           free(p)

#define mem_deep_copy(d, s, l)    memmove(d, s, l)
#define mem_copy(d, s, l)         fastcopy(d, s, l)

#if !ENABLE(SYSTEM_MALLOC)
#undef new
#undef delete

MAYBE_INLINE void* operator new (size_t size) { return mem_malloc(size); }
MAYBE_INLINE void* operator new[] (size_t size) { return mem_malloc(size); }
MAYBE_INLINE void operator delete (void* p) { mem_free(p); }
MAYBE_INLINE void operator delete[] (void* p) { mem_free(p); }

#endif /*ENABLE(SYSTEM_MALLOC)*/

#endif/*_MEMORY_MANAGER_H_*/
