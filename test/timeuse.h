

#ifndef _TU_H_
#define _TU_H_

#include "unistd.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "sys/time.h"

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



#endif
