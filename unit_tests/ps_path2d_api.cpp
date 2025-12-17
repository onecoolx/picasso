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

class PathTest : public ::testing::Test
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
        path = ps_path_create();
        ASSERT_TRUE(path);
        ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    }

    void TearDown() override
    {
        if (path) {
            ps_path_unref(path);
            path = NULL;
        }
    }

    ps_path* path;
};

// Path Creation and Reference Management Tests
TEST_F(PathTest, CreateAndDestroy)
{
    ps_path* newPath = ps_path_create();
    ASSERT_TRUE(newPath);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    ps_path_unref(newPath);
}

TEST_F(PathTest, CreateCopy)
{
    // Add some content to original path
    ps_point p1 = {10.0f, 10.0f};
    ps_point p2 = {20.0f, 20.0f};
    ps_path_move_to(path, &p1);
    ps_path_line_to(path, &p2);

    ps_path* copyPath = ps_path_create_copy(path);
    ASSERT_TRUE(copyPath);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    // Verify copy has same content
    ASSERT_EQ(ps_path_get_vertex_count(path), ps_path_get_vertex_count(copyPath));

    ps_path_unref(copyPath);
}

TEST_F(PathTest, ReferenceCounting)
{
    ps_path* refPath = ps_path_ref(path);
    ASSERT_EQ(refPath, path);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_path_unref(refPath); // Should not destroy the original
    // path will be cleaned up in TearDown
}

// Path Building Tests
TEST_F(PathTest, MoveTo)
{
    ps_point p = {100.0f, 200.0f};
    ps_path_move_to(path, &p);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_EQ(ps_path_get_vertex_count(path), 1);

    ps_point vertex;
    ps_path_cmd cmd = ps_path_get_vertex(path, 0, &vertex);
    ASSERT_EQ(cmd, PATH_CMD_MOVE_TO);
    ASSERT_FLOAT_EQ(vertex.x, 100.0f);
    ASSERT_FLOAT_EQ(vertex.y, 200.0f);
}

TEST_F(PathTest, LineTo)
{
    ps_point p1 = {10.0f, 10.0f};
    ps_point p2 = {50.0f, 60.0f};
    ps_path_move_to(path, &p1);
    ps_path_line_to(path, &p2);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_EQ(ps_path_get_vertex_count(path), 2);

    ps_point vertex;
    ps_path_cmd cmd = ps_path_get_vertex(path, 1, &vertex);
    ASSERT_EQ(cmd, PATH_CMD_LINE_TO);
    ASSERT_FLOAT_EQ(vertex.x, 50.0f);
    ASSERT_FLOAT_EQ(vertex.y, 60.0f);
}

TEST_F(PathTest, BezierTo)
{
    ps_point p1 = {0.0f, 0.0f};
    ps_point cp1 = {10.0f, 20.0f};
    ps_point cp2 = {30.0f, 40.0f};
    ps_point p2 = {50.0f, 50.0f};

    ps_path_move_to(path, &p1);
    ps_path_bezier_to(path, &cp1, &cp2, &p2);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

#if 0
    ASSERT_EQ(ps_path_get_vertex_count(path), 4);

    ps_point vertex;
    ps_path_get_vertex(path, 1, &vertex);
    ASSERT_FLOAT_EQ(vertex.x, 10.0f);
    ASSERT_FLOAT_EQ(vertex.y, 20.0f);

    ps_path_get_vertex(path, 2, &vertex);
    ASSERT_FLOAT_EQ(vertex.x, 30.0f);
    ASSERT_FLOAT_EQ(vertex.y, 40.0f);

    ps_path_get_vertex(path, 3, &vertex);
    ASSERT_FLOAT_EQ(vertex.x, 50.0f);
    ASSERT_FLOAT_EQ(vertex.y, 50.0f);
#endif
}

TEST_F(PathTest, QuadTo)
{
    ps_point p1 = {0.0f, 0.0f};
    ps_point cp = {25.0f, 25.0f};
    ps_point p2 = {50.0f, 50.0f};

    ps_path_move_to(path, &p1);
    ps_path_quad_to(path, &cp, &p2);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

#if 0
    ASSERT_EQ(ps_path_get_vertex_count(path), 3);

    ps_point vertex;
    ps_path_get_vertex(path, 1, &vertex);
    ASSERT_FLOAT_EQ(vertex.x, 25.0f);
    ASSERT_FLOAT_EQ(vertex.y, 25.0f);
#endif
}

TEST_F(PathTest, ArcTo)
{
    ps_point p1 = {0.0f, 0.0f};
    ps_point p2 = {100.0f, 0.0f};

    ps_path_move_to(path, &p1);
    ps_path_arc_to(path, 50.0f, 50.0f, 0.0f, False, False, &p2);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(ps_path_get_vertex_count(path), 1);
}

TEST_F(PathTest, TangentArcTo)
{
    ps_point p1 = {0.0f, 0.0f};
    ps_point tp = {50.0f, 50.0f};
    ps_point ep = {100.0f, 0.0f};

    ps_path_move_to(path, &p1);
    ps_path_tangent_arc_to(path, 25.0f, &tp, &ep);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(ps_path_get_vertex_count(path), 1);
}

TEST_F(PathTest, SubClose)
{
    ps_point p1 = {0.0f, 0.0f};
    ps_point p2 = {100.0f, 0.0f};
    ps_point p3 = {100.0f, 100.0f};
    ps_point p4 = {0.0f, 100.0f};

    ps_path_move_to(path, &p1);
    ps_path_line_to(path, &p2);
    ps_path_line_to(path, &p3);
    ps_path_line_to(path, &p4);
    ps_path_sub_close(path);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Path Utility Tests
TEST_F(PathTest, IsEmpty)
{
    ASSERT_TRUE(ps_path_is_empty(path));
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_point p = {10.0f, 10.0f};
    ps_path_move_to(path, &p);
    ASSERT_FALSE(ps_path_is_empty(path));
}

TEST_F(PathTest, Clear)
{
    ps_point p = {10.0f, 10.0f};
    ps_path_move_to(path, &p);
    ASSERT_FALSE(ps_path_is_empty(path));

    ps_path_clear(path);
    ASSERT_TRUE(ps_path_is_empty(path));
    ASSERT_EQ(0, ps_path_get_vertex_count(path));
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathTest, GetLength)
{
    ps_point p1 = {0.0f, 0.0f};
    ps_point p2 = {100.0f, 0.0f};

    ps_path_move_to(path, &p1);
    ps_path_line_to(path, &p2);

    float length = ps_path_get_length(path);
    ASSERT_FLOAT_EQ(length, 100.0f);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

// Path Add Operations Tests
TEST_F(PathTest, AddLine)
{
    ps_point p1 = {10.0f, 10.0f};
    ps_point p2 = {50.0f, 60.0f};

    ps_path_add_line(path, &p1, &p2);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_EQ(ps_path_get_vertex_count(path), 2);

    ps_point vertex;
    ps_path_cmd cmd = ps_path_get_vertex(path, 0, &vertex);
    ASSERT_EQ(cmd, PATH_CMD_MOVE_TO);
    ASSERT_FLOAT_EQ(vertex.x, 10.0f);
    ASSERT_FLOAT_EQ(vertex.y, 10.0f);
}

TEST_F(PathTest, AddArc)
{
    ps_point cp = {50.0f, 50.0f};

    ps_path_add_arc(path, &cp, 25.0f, 0.0f, 3.14159f, False);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(ps_path_get_vertex_count(path), 0);
}

TEST_F(PathTest, AddRect)
{
    ps_rect rect = {10.0f, 10.0f, 100.0f, 50.0f};

    ps_path_add_rect(path, &rect);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(ps_path_get_vertex_count(path), 0);
}

TEST_F(PathTest, AddEllipse)
{
    ps_rect rect = {10.0f, 10.0f, 100.0f, 50.0f};

    ps_path_add_ellipse(path, &rect);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(ps_path_get_vertex_count(path), 0);
}

TEST_F(PathTest, AddRoundedRect)
{
    ps_rect rect = {10.0f, 10.0f, 100.0f, 50.0f};

    ps_path_add_rounded_rect(path, &rect, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(ps_path_get_vertex_count(path), 0);
}

TEST_F(PathTest, AddSubPath)
{
    ps_path* subPath = ps_path_create();
    ASSERT_TRUE(subPath);

    ps_point p1 = {0.0f, 0.0f};
    ps_point p2 = {50.0f, 50.0f};
    ps_path_move_to(subPath, &p1);
    ps_path_line_to(subPath, &p2);

    ps_path_add_sub_path(path, subPath);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_GT(ps_path_get_vertex_count(path), 0);

    ps_path_unref(subPath);
}

// Path Query Tests
TEST_F(PathTest, GetVertexCount)
{
    ASSERT_EQ(0, ps_path_get_vertex_count(path));

    ps_point p = {10.0f, 10.0f};
    ps_path_move_to(path, &p);
    ASSERT_EQ(1, ps_path_get_vertex_count(path));

    ps_point p2 = {20.0f, 20.0f};
    ps_path_line_to(path, &p2);
    ASSERT_EQ(2, ps_path_get_vertex_count(path));
}

TEST_F(PathTest, GetVertex)
{
    ps_point p1 = {10.0f, 10.0f};
    ps_point p2 = {20.0f, 20.0f};
    ps_path_move_to(path, &p1);
    ps_path_line_to(path, &p2);

    ps_point vertex;
    ps_path_cmd cmd = ps_path_get_vertex(path, 0, &vertex);
    ASSERT_EQ(cmd, PATH_CMD_MOVE_TO);
    ASSERT_FLOAT_EQ(vertex.x, 10.0f);
    ASSERT_FLOAT_EQ(vertex.y, 10.0f);

    cmd = ps_path_get_vertex(path, 1, &vertex);
    ASSERT_EQ(cmd, PATH_CMD_LINE_TO);
    ASSERT_FLOAT_EQ(vertex.x, 20.0f);
    ASSERT_FLOAT_EQ(vertex.y, 20.0f);
}

TEST_F(PathTest, BoundingRect)
{
    ps_point p1 = {10.0f, 20.0f};
    ps_point p2 = {50.0f, 80.0f};
    ps_path_move_to(path, &p1);
    ps_path_line_to(path, &p2);

    ps_rect rect;
    ps_bool result = ps_path_bounding_rect(path, &rect);
    ASSERT_TRUE(result);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ASSERT_FLOAT_EQ(rect.x, 10.0f);
    ASSERT_FLOAT_EQ(rect.y, 20.0f);
    ASSERT_FLOAT_EQ(rect.w, 40.0f);
    ASSERT_FLOAT_EQ(rect.h, 60.0f);
}

TEST_F(PathTest, Contains)
{
    // Create a simple rectangle
    ps_rect rect = {0.0f, 0.0f, 100.0f, 100.0f};
    ps_path_add_rect(path, &rect);

    ps_point insidePoint = {50.0f, 50.0f};
    ps_point outsidePoint = {150.0f, 150.0f};

    ASSERT_TRUE(ps_path_contains(path, &insidePoint, FILL_RULE_WINDING));
    ASSERT_FALSE(ps_path_contains(path, &outsidePoint, FILL_RULE_WINDING));
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathTest, StrokeContains)
{
    // Create a line
    ps_point p1 = {0.0f, 0.0f};
    ps_point p2 = {100.0f, 0.0f};
    ps_path_move_to(path, &p1);
    ps_path_line_to(path, &p2);

    ps_point onLinePoint = {50.0f, 0.0f};
    ps_point nearLinePoint = {50.0f, 2.0f};
    ps_point farPoint = {50.0f, 10.0f};

    ASSERT_TRUE(ps_path_stroke_contains(path, &onLinePoint, 5.0f));
    ASSERT_TRUE(ps_path_stroke_contains(path, &nearLinePoint, 5.0f));
    ASSERT_FALSE(ps_path_stroke_contains(path, &farPoint, 5.0f));
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

TEST_F(PathTest, PathClippingTest)
{
    ps_rect rectA = {0.0f, 0.0f, 50.0f, 50.0f};
    ps_rect rectB = {25.0f, 25.0f, 50.0f, 50.0f};

    ps_path* pathA = ps_path_create();
    ps_path* pathB = ps_path_create();

    ps_path_add_rect(pathA, &rectA);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_path_add_rect(pathB, &rectB);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_path_clipping(path, PATH_OP_UNION, pathA, pathB);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    EXPECT_GT(ps_path_get_vertex_count(path), 0);
    EXPECT_FALSE(ps_path_is_empty(path));

    ps_path_clipping(path, PATH_OP_INTERSECT, pathA, pathB);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    EXPECT_GT(ps_path_get_vertex_count(path), 0);
    EXPECT_FALSE(ps_path_is_empty(path));

    ps_path_clipping(path, PATH_OP_XOR, pathA, pathB);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    EXPECT_GT(ps_path_get_vertex_count(path), 0);
    EXPECT_FALSE(ps_path_is_empty(path));

    ps_path_clipping(path, PATH_OP_DIFF, pathA, pathB);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
    EXPECT_GT(ps_path_get_vertex_count(path), 0);
    EXPECT_FALSE(ps_path_is_empty(path));

    ps_path_unref(pathA);
    ps_path_unref(pathB);
}

// Bad Case Tests
TEST_F(PathTest, BadCase_NullPath)
{
    ps_point p = {10.0f, 10.0f};
    ps_rect r = {10.0f, 10.0f, 100.0f, 50.0f};

    ps_path_move_to(NULL, &p);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_line_to(NULL, &p);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_bezier_to(NULL, &p, &p, &p);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_quad_to(NULL, &p, &p);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_arc_to(NULL, 10.0f, 10.0f, 0.0f, False, False, &p);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_tangent_arc_to(NULL, 10.0f, &p, &p);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_sub_close(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_clear(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_is_empty(NULL);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    float length = ps_path_get_length(NULL);
    ASSERT_EQ(0.0f, length);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    uint32_t count = ps_path_get_vertex_count(NULL);
    ASSERT_EQ(0, count);
    ASSERT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_get_vertex(NULL, 0, &p);
    EXPECT_EQ(PATH_CMD_STOP, ps_path_get_vertex(NULL, 0, &p));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_ref(NULL);
    EXPECT_EQ(NULL, ps_path_ref(NULL));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_unref(NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_create_copy(NULL);
    EXPECT_EQ(NULL, ps_path_create_copy(NULL));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_bounding_rect(NULL, &r);
    EXPECT_EQ(False, ps_path_bounding_rect(NULL, &r));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_contains(NULL, &p, FILL_RULE_WINDING);
    EXPECT_EQ(False, ps_path_contains(NULL, &p, FILL_RULE_WINDING));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_stroke_contains(NULL, &p, 5.0f);
    EXPECT_EQ(False, ps_path_stroke_contains(NULL, &p, 5.0f));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_line(NULL, &p, &p);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_arc(NULL, &p, 25.0f, 0.0f, 3.14159f, False);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_rect(NULL, &r);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_ellipse(NULL, &r);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_rounded_rect(NULL, &r, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_sub_path(NULL, path);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_clipping(NULL, PATH_OP_UNION, path, path);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_move_to(path, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_line_to(path, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_bezier_to(path, NULL, &p, &p);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_bezier_to(path, &p, NULL, &p);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_bezier_to(path, &p, &p, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_quad_to(path, NULL, &p);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_quad_to(path, &p, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_arc_to(path, 10.0f, 10.0f, 0.0f, False, False, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_tangent_arc_to(path, 10.0f, NULL, &p);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_tangent_arc_to(path, 10.0f, &p, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_get_vertex(path, 0, NULL);
    EXPECT_EQ(PATH_CMD_STOP, ps_path_get_vertex(path, 0, NULL));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_contains(path, NULL, FILL_RULE_WINDING);
    EXPECT_EQ(False, ps_path_contains(path, NULL, FILL_RULE_WINDING));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_stroke_contains(path, NULL, 5.0f);
    EXPECT_EQ(False, ps_path_stroke_contains(path, NULL, 5.0f));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_line(path, NULL, &p);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_line(path, &p, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_arc(path, NULL, 25.0f, 0.0f, 3.14159f, False);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_rect(path, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_ellipse(path, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_rounded_rect(path, NULL, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_add_sub_path(path, NULL);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_path_bounding_rect(path, NULL);
    EXPECT_EQ(False, ps_path_bounding_rect(path, NULL));
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

}
