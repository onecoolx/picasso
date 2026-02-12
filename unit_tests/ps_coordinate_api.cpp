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

class CoordinateTest : public ::testing::Test
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

    void SetUp() override
    {
        canvas = get_test_canvas();
        ctx = ps_context_create(canvas, NULL);
        ASSERT_TRUE(ctx);
    }

    void TearDown() override
    {
        ps_context_unref(ctx);
    }

    ps_canvas* canvas;
    ps_context* ctx;
};

TEST_F(CoordinateTest, GetMatrixIdentity)
{
    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix);

    ps_bool result = ps_get_matrix(ctx, matrix);
    EXPECT_TRUE(result);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
    EXPECT_TRUE(ps_matrix_is_identity(matrix));

    ps_matrix_unref(matrix);
}

TEST_F(CoordinateTest, SetAndGetMatrix)
{
    ps_matrix* set_matrix = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 3.0f, 10.0f, 20.0f);
    ASSERT_TRUE(set_matrix);

    ps_set_matrix(ctx, set_matrix);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_matrix* get_matrix = ps_matrix_create();
    ASSERT_TRUE(get_matrix);

    ps_bool result = ps_get_matrix(ctx, get_matrix);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ps_matrix_is_equal(set_matrix, get_matrix));

    ps_matrix_unref(set_matrix);
    ps_matrix_unref(get_matrix);
}

TEST_F(CoordinateTest, TransformPointWithIdentity)
{
    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix);

    ps_point p = {100.0f, 200.0f};
    ps_matrix_transform_point(matrix, &p);

    EXPECT_FLOAT_EQ(100.0f, p.x);
    EXPECT_FLOAT_EQ(200.0f, p.y);

    ps_matrix_unref(matrix);
}

TEST_F(CoordinateTest, TransformPointWithScale)
{
    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix);
    ps_matrix_scale(matrix, 2.0f, 3.0f);

    ps_point p = {50.0f, 60.0f};
    ps_matrix_transform_point(matrix, &p);

    EXPECT_FLOAT_EQ(100.0f, p.x);
    EXPECT_FLOAT_EQ(180.0f, p.y);

    ps_matrix_unref(matrix);
}

TEST_F(CoordinateTest, TransformPointWithTranslate)
{
    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix);
    ps_matrix_translate(matrix, 10.0f, 20.0f);

    ps_point p = {50.0f, 60.0f};
    ps_matrix_transform_point(matrix, &p);

    EXPECT_FLOAT_EQ(60.0f, p.x);
    EXPECT_FLOAT_EQ(80.0f, p.y);

    ps_matrix_unref(matrix);
}

TEST_F(CoordinateTest, TransformPointWithRotate)
{
    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix);
    ps_matrix_rotate(matrix, 1.5708f); // 90 degrees in radians

    ps_point p = {100.0f, 0.0f};
    ps_matrix_transform_point(matrix, &p);

    EXPECT_NEAR(0.0f, p.x, 0.01f);
    EXPECT_NEAR(100.0f, p.y, 0.01f);

    ps_matrix_unref(matrix);
}

TEST_F(CoordinateTest, TransformRectangle)
{
    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix);
    ps_matrix_scale(matrix, 2.0f, 2.0f);

    ps_rect rc = {10.0f, 10.0f, 50.0f, 50.0f};
    ps_matrix_transform_rect(matrix, &rc);

    EXPECT_FLOAT_EQ(20.0f, rc.x);
    EXPECT_FLOAT_EQ(20.0f, rc.y);
    EXPECT_FLOAT_EQ(100.0f, rc.w);
    EXPECT_FLOAT_EQ(100.0f, rc.h);

    ps_matrix_unref(matrix);
}

TEST_F(CoordinateTest, GetMatrixNull)
{
    ps_bool result = ps_get_matrix(NULL, NULL);
    EXPECT_FALSE(result);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(CoordinateTest, GetPath)
{
    ps_rect rc = {10, 10, 100, 100};
    ps_rectangle(ctx, &rc);

    ps_path* path = ps_path_create();
    ps_bool result = ps_get_path(ctx, path);
    EXPECT_TRUE(result);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
    EXPECT_GT(ps_path_get_vertex_count(path), 0);

    ps_path_unref(path);
}

TEST_F(CoordinateTest, GetPathNull)
{
    ps_bool result = ps_get_path(NULL, NULL);
    EXPECT_FALSE(result);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(CoordinateTest, TransformPath)
{
    ps_path* path = ps_path_create();
    ps_point p0 = {0.0f, 0.0f};
    ps_point p1 = {100.0f, 0.0f};
    ps_point p2 = {100.0f, 100.0f};
    ps_point p3 = {0.0f, 100.0f};

    ps_path_move_to(path, &p0);
    ps_path_line_to(path, &p1);
    ps_path_line_to(path, &p2);
    ps_path_line_to(path, &p3);

    ps_matrix* matrix = ps_matrix_create();
    ASSERT_TRUE(matrix);
    ps_matrix_scale(matrix, 2.0f, 2.0f);
    ps_matrix_transform_path(matrix, path);

    EXPECT_GT(ps_path_get_vertex_count(path), 0);

    ps_matrix_unref(matrix);
    ps_path_unref(path);
}
