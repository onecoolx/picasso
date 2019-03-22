#include "test.h"

TEST(Matrix, CreateAndDestory)
{
    ps_matrix* m = NULL;
    m = ps_matrix_create();
    EXPECT_NE((ps_matrix*)NULL, m);    

    ps_matrix* m2 = NULL;
    m2 = ps_matrix_create_init(1, 0, 0, 1, 0, 0);
    EXPECT_NE((ps_matrix*)NULL, m2);

    ps_matrix* m3 = NULL;
    m3 = ps_matrix_create_copy(m);
    EXPECT_NE((ps_matrix*)NULL, m3);

    EXPECT_NE((ps_matrix*)NULL, ps_matrix_ref(m));

    ps_matrix_unref(m);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 

    ps_matrix_init(m, 1,0,0,1,0,0);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 

    ps_matrix_unref(m);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status()); 
}




