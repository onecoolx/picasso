

#ifndef _TU_H_
#define _TU_H_

#include "unistd.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "sys/time.h"
#ifdef WIN32
#include <windows.h>
#endif

#ifdef WIN32
typedef long suseconds_t;
#endif

static inline suseconds_t get_time()
{
    struct timeval t;
    gettimeofday(&t, 0);
    suseconds_t t1 = t.tv_usec;
    return t1;
}

#ifdef WIN32
typedef LARGE_INTEGER clocktime_t;

static inline clocktime_t get_clock()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t;
}

static inline double get_clock_used_ms(LARGE_INTEGER t1, LARGE_INTEGER t2)
{
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    return ((double)(t2.QuadPart-t1.QuadPart)/(double)f.QuadPart)*1000;
}

#endif

#endif
