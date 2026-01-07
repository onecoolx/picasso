

#ifndef _TU_H_
#define _TU_H_

#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#if defined(LINUX) || defined(UNIX)
#include "unistd.h"
#include "sys/time.h"
#endif
#if defined(WIN32)
#include <windows.h>
#endif

#if defined(WIN32)
typedef long long suseconds_t;
#define inline __inline
#endif

static inline suseconds_t get_time()
{
#if defined(WIN32)
    DWORD t1 = GetTickCount();
#else
    struct timeval t;
    gettimeofday(&t, 0);
    suseconds_t t1 = (suseconds_t)(t.tv_sec * 1000  + t.tv_usec/1000);
#endif
    return t1;
}

#if defined(WIN32)
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

#if defined(ARM)
static inline uint64_t arm_rdtsc(void)
{
    uint64_t tsc;
    asm volatile("mrs %0, cntvct_el0" : "=r"(tsc));
    return tsc;
}

static inline uint64_t
arm64_cntfrq(void)
{
   uint64_t freq;
   asm volatile("mrs %0, cntfrq_el0" : "=r" (freq));
   return freq;
}

static inline uint64_t
arm64_pmccntr(void)
{
   uint64_t tsc;
   asm volatile("mrs %0, pmccntr_el0" : "=r"(tsc));
   return tsc;
}

static inline uint64_t
rdtsc(void)
{
   return arm64_pmccntr();
}

static void enable_pmu_pmccntr(void)
{
   u64 val = 0;
   asm volatile("msr pmintenset_el1, %0" : : "r" ((u64)(0 << 31)));
   asm volatile("msr pmcntenset_el0, %0" :: "r" ((u64)(1 << 31)));
   asm volatile("msr pmuserenr_el0, %0" : : "r"((u64)(1 << 0) | (u64)(1 << 2)));
   asm volatile("mrs %0, pmcr_el0" : "=r" (val));
   val |= ((u64)(1 << 0) | (u64)(1 << 2));
   isb();
   asm volatile("msr pmcr_el0, %0" : : "r" (val));
}
#endif

#endif
