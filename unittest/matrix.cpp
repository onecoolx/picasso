#include "test.h"

TEST(Matrix, CreateAndDestory)
{
    ps_matrix* m = 0;
    m = ps_matrix_create();
    EXPECT_NE((ps_matrix*)0, ps_matrix_create());    
    EXPECT_NE((ps_matrix*)0, ps_matrix_create_init(1, 0, 0, 1, 0, 0));    
    EXPECT_NE((ps_matrix*)0, ps_matrix_create_copy(m));

    EXPECT_NE((ps_matrix*)0, ps_matrix_ref(m));

    ps_matrix_unref(m);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 

    ps_matrix_init(m, 1,0,0,1,0,0);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 

    ps_matrix_unref(m);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 
}




