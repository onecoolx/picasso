/*
 * Copyright (c) 2025, Zhang Ji Peng
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

#include "test.h"

class MatrixTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        PS_Init();
    }

    static void TearDownTestSuite()
    {
        PS_Shutdown();
    }
};

TEST_F(MatrixTest, CreateAndDestory)
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

    ps_matrix_init(m, 1, 0, 0, 1, 0, 0);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_matrix_unref(m);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_matrix_unref(m2);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_matrix_unref(m3);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}
