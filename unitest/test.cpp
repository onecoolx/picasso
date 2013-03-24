
#include "test.h"


TEST(SystemDeathTest, Init) 
{ 
    printf("picasso initialize\n");
	ASSERT_NE(False, ps_initialize()); 
	int v = ps_version(); 
	ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 
} 



TEST(System, Shutdown) 
{ 
    printf("picasso shutdown\n");
	ps_shutdown(); 
	ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 
}

