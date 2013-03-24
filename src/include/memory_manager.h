
/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include <new>
#include <stdlib.h>

//common memory managers

#define mem_malloc(n) 		malloc(n)
#define mem_calloc(n, s) 	calloc(n, s)
#define mem_realloc(p, s) 	realloc(p, s)
#define mem_free(p)			free(p)

#ifdef __cplusplus
#undef new
#undef delete

inline void* operator new (size_t size) { return mem_malloc(size); }
inline void* operator new[] (size_t size) { return mem_malloc(size); }
inline void operator delete (void* p) { mem_free(p); }
inline void operator delete[] (void* p) { mem_free(p); }
#endif

#endif/*_MEMORY_MANAGER_H_*/
