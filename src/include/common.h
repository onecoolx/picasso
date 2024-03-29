/* Picasso - a vector graphics library
 *
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _COMMON_H_
#define _COMMON_H_

// all source file must include this file.

#include "pconfig.h"

#include "platform.h"
#include "memory_manager.h"

#if !COMPILER(MSVC)
    #include <inttypes.h>
#else
    typedef signed   char int8_t;
    typedef unsigned char uint8_t;
    typedef signed   short int16_t;
    typedef unsigned short uint16_t;
    typedef signed   int int32_t;
    typedef unsigned int uint32_t;
    typedef unsigned long long int uint64_t;
    typedef long long int int64_t;
    // snprintf
    #define snprintf _snprintf
#endif  /* _MSC_VER */

// special types
typedef uint8_t byte;

#endif /*_COMMON_H_*/
