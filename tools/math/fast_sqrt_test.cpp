
#include "test.h"
#include "timeuse.h"
#include <math.h>

#include "fast_sqrt.h"

TEST(FastSqrt, Performance)
{
    suseconds_t t1, t2, r1, r2;
    uint32_t i = 133;
    uint32_t x1 = 0, x2 = 0;

    clear_dcache();
    t1 = get_time();
    for (uint32_t t = 10000000; t > 0; t--)
    {
        x1 += fast_sqrt(i++);
    }
    t2 = get_time();

    r1 = t2-t1;

    i = 133;

    clear_dcache();

    t1 = get_time();
    for (uint32_t t = 10000000; t > 0; t--)
    {
        x2 += (int32_t)sqrt(i++);
    }
    t2 = get_time();
    r2 = t2-t1;

    printf("time %.4f   -  %.4f \n", r1/1000.0, r2/1000.0);
    EXPECT_EQ(x1, x2); 
}
