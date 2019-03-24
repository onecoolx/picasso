/* Picasso - a vector graphics library
 *
 * Copyright (C) 2009 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PICASSO_GLOBAL_H_
#define _PICASSO_GLOBAL_H_

#include "common.h"
#include "math_type.h"
#include "data_vector.h"
#include "fixedopt.h"
#include <string.h>

#include "picasso.h"

inline void* ZeroBufferAlloc(size_t bytes) {
	void *buffer = mem_malloc(bytes);
    return buffer?memset(buffer, 0, bytes):NULL;
}
inline void* ZeroBuffersAlloc(size_t num, size_t size) {
	void *buffer = mem_calloc(num, size);
    return buffer?memset(buffer, 0, bytes):NULL;
}

//this can be replace by hw buffer!
#define BufferAlloc(n)         ZeroBufferAlloc(n)
#define BuffersAlloc(n, s)     ZeroBuffersAlloc(n, s)
#define BufferFree(p)          mem_free(p)
#define BufferCopy(d, s, n)    mem_copy(d, s, n)

#define MAX(x, y)    (((x) > (y))?(x):(y))
#define MIN(x, y)    (((x) < (y))?(x):(y))
#define ABS(x)       (((x) < 0)?(-(x)):(x))

// global error code
extern "C" ps_status global_status;

#endif /*_PICASSO_GLOBAL_H_*/
