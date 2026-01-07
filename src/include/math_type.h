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

#ifndef _MATH_TYPES_H_
#define _MATH_TYPES_H_

#include "common.h"

#include <math.h>
typedef float scalar;
#define FLT_TO_SCALAR(v)  ((scalar)(v))
#define INT_TO_SCALAR(v)  ((scalar)(v))
#define SCALAR_TO_FLT(v)  ((float)(v))
#define SCALAR_TO_INT(v)  ((int32_t)(v))
//math PI value
#define PI      3.14159265358979323846f
#define _2PI    6.28318530717958647692f
#define _PIdiv2 1.57079632679489661923f
#define _1divPI 0.31830988618379069f
#define _1div2PI 0.1591549432737563f

#define Fabs(x) fabsf(x)
#define Fmod(x, y) fmodf((x), (y))
#define Sqrt(x) sqrtf(x)
#define Tan(x) tanf(x)
#define Atan2(x, y) atan2f((x), (y))
#define Sin(x) sinf(x)
#define Cos(x) cosf(x)
#define Acos(x) acosf(x)
#define Asin(x) asinf(x)
#define Floor(x) floorf(x)
#define Ceil(x) ceilf(x)
#define Exp(x) expf(x)
#define Pow(x, y) powf((x), (y))
#define Round(x) roundf(x)

#define SqrtD(x) sqrt(x)

// max min
#define Max(x, y)    (((x) > (y)) ? (x) : (y))
#define Min(x, y)    (((x) < (y)) ? (x) : (y))

// float type
inline uint32_t _uround(float v)
{
    return uint32_t(v + 0.5f);
}

inline int32_t _iround(float v)
{
    return int32_t((v < 0.0f) ? v - 0.5f : v + 0.5f);
}

#endif /*_MATH_TYPES_H_*/
