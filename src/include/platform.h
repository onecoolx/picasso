/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2010 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define ENABLE(FEATURE) (defined ENABLE_##FEATURE && ENABLE_##FEATURE)

#if defined(WINCE) 
// math.h
#define sinf(x) ((float)sin(x))
#define cosf(x) ((float)cos(x))
#define acosf(x) ((float)acos(x))
#define asinf(x) ((float)asin(x))
#define atan2f(x, y) ((float)atan2((x), (y)))
#endif

#endif /*_PLATFORM_H_*/
