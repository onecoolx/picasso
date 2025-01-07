#include "test.h"

volatile int tmp;
static int dummy[4096000];
void clear_dcache(void)
{
    int sum = 0;
    for(int i=0; i<4096000; i++)
        dummy[i] = 2;
    for(int i=0; i<4096000; i++)
        sum += dummy[i];

    tmp = sum;
}

TEST(SystemDeathTest, Init) 
{ 
    printf("picasso initialize\n");
    ASSERT_NE(False, ps_initialize()); 
    int v = ps_version(); 
    fprintf(stderr, "picasso version %d \n", v);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 
} 

TEST(System, Shutdown) 
{ 
    printf("picasso shutdown\n");
    ps_shutdown(); 
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 
}

