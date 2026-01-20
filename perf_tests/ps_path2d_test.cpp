/*
 * Copyright (c) 2026, Zhang Ji Peng
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

PERF_TEST_DEFINE(Path);

// Test 1: Basic path creation and destruction
PERF_TEST(Path, PathCreateDestroy)
{
    for (int i = 0; i < 10000; i++) {
        ps_path* path = ps_path_create();
        ps_path_unref(path);
    }
}

// Test 2: Path creation with copy
PERF_TEST_RUN(Path, PathCreateCopy)
{
    ps_path* source = ps_path_create();
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_path_move_to(source, &start);
    ps_path_line_to(source, &end);

    auto result = RunBenchmark(Path_PathCreateCopy, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_path* copy = ps_path_create_copy(source);
            ps_path_unref(copy);
        }
    });

    CompareToBenchmark(Path_PathCreateCopy, result);
    ps_path_unref(source);
}

// Test 3: Path reference counting
PERF_TEST_RUN(Path, PathReferenceOperations)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_path_ref(path);
            ps_path_unref(path);
        }
    });

    CompareToBenchmark(Path_PathReferenceOperations, result);
    ps_path_unref(path);
}

// Test 4: Path building operations - move_to and line_to
PERF_TEST_RUN(Path, PathBuildingMoveLine)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathBuildingMoveLine, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_point start = {i * 10.0f, i * 10.0f};
            ps_point end = {(i + 1) * 10.0f, (i + 1) * 10.0f};

            ps_path_move_to(path, &start);
            ps_path_line_to(path, &end);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathBuildingMoveLine, result);
    ps_path_unref(path);
}

// Test 5: Bezier curve operations
PERF_TEST_RUN(Path, PathBezierOperations)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathBezierOperations, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_point start = {0, 0};
            ps_point fcp = {25, 0};
            ps_point scp = {75, 100};
            ps_point end = {100, 100};

            ps_path_move_to(path, &start);
            ps_path_bezier_to(path, &fcp, &scp, &end);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathBezierOperations, result);
    ps_path_unref(path);
}

// Test 6: Quadratic curve operations
PERF_TEST_RUN(Path, PathQuadOperations)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathQuadOperations, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_point start = {0, 0};
            ps_point cp = {50, 0};
            ps_point end = {100, 100};

            ps_path_move_to(path, &start);
            ps_path_quad_to(path, &cp, &end);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathQuadOperations, result);
    ps_path_unref(path);
}

// Test 7: Arc operations
PERF_TEST_RUN(Path, PathArcOperations)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathArcOperations, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_point start = {0, 0};
            ps_point end = {100, 100};

            ps_path_move_to(path, &start);
            ps_path_arc_to(path, 50.0f, 50.0f, 0.0f, False, True, &end);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathArcOperations, result);
    ps_path_unref(path);
}

// Test 8: Tangent arc operations
PERF_TEST_RUN(Path, PathTangentArcOperations)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathTangentArcOperations, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_point start = {0, 0};
            ps_point tp = {50, 0};
            ps_point ep = {100, 100};

            ps_path_move_to(path, &start);
            ps_path_tangent_arc_to(path, 25.0f, &tp, &ep);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathTangentArcOperations, result);
    ps_path_unref(path);
}

// Test 9: Path add operations - line
PERF_TEST_RUN(Path, PathAddLine)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathAddLine, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_point p1 = {i * 10.0f, i * 10.0f};
            ps_point p2 = {(i + 1) * 10.0f, (i + 1) * 10.0f};

            ps_path_add_line(path, &p1, &p2);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathAddLine, result);
    ps_path_unref(path);
}

// Test 10: Path add operations - rectangle
PERF_TEST_RUN(Path, PathAddRect)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathAddRect, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_rect rect = {i * 10.0f, i * 10.0f, 50, 50};
            ps_path_add_rect(path, &rect);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathAddRect, result);
    ps_path_unref(path);
}

// Test 11: Path add operations - ellipse
PERF_TEST_RUN(Path, PathAddEllipse)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathAddEllipse, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_rect rect = {i * 10.0f, i * 10.0f, 50, 30};
            ps_path_add_ellipse(path, &rect);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathAddEllipse, result);
    ps_path_unref(path);
}

// Test 12: Path add operations - rounded rectangle
PERF_TEST_RUN(Path, PathAddRoundedRect)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathAddRoundedRect, [&]() {
        for (int i = 0; i < 3000; i++) {
            ps_rect rect = {i * 10.0f, i * 10.0f, 50, 50};
            ps_path_add_rounded_rect(path, &rect, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathAddRoundedRect, result);
    ps_path_unref(path);
}

// Test 13: Path add operations - arc
PERF_TEST_RUN(Path, PathAddArc)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathAddArc, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_point cp = {50, 50};
            ps_path_add_arc(path, &cp, 25.0f, 0.0f, 3.14159f, True);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathAddArc, result);
    ps_path_unref(path);
}

// Test 14: Path add sub path
PERF_TEST_RUN(Path, PathAddSubPath)
{
    ps_path* path1 = ps_path_create();
    ps_path* path2 = ps_path_create();

    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_path_move_to(path2, &start);
    ps_path_line_to(path2, &end);

    auto result = RunBenchmark(Path_PathAddSubPath, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_path_add_sub_path(path1, path2);
            ps_path_clear(path1);
        }
    });

    CompareToBenchmark(Path_PathAddSubPath, result);
    ps_path_unref(path1);
    ps_path_unref(path2);
}

// Test 15: Path query operations - length and empty check
PERF_TEST_RUN(Path, PathQueryOperations)
{
    ps_path* path = ps_path_create();

    // Build a complex path
    for (int i = 0; i < 100; i++) {
        ps_point p = {i * 10.0f, sinf(i * 0.1) * 50};
        if (i == 0) {
            ps_path_move_to(path, &p);
        } else {
            ps_path_line_to(path, &p);
        }
    }

    auto result = RunBenchmark(Path_PathQueryOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            float length = ps_path_get_length(path);
            ps_bool empty = ps_path_is_empty(path);
            uint32_t count = ps_path_get_vertex_count(path);
            UNUSED(length);
            UNUSED(count);
            UNUSED(empty);
        }
    });

    CompareToBenchmark(Path_PathQueryOperations, result);
    ps_path_unref(path);
}

// Test 16: Path vertex operations
PERF_TEST_RUN(Path, PathVertexOperations)
{
    ps_path* path = ps_path_create();

    // Build a path with multiple vertices
    for (int i = 0; i < 50; i++) {
        ps_point p = {i * 10.0f, i * 5.0f};
        if (i == 0) {
            ps_path_move_to(path, &p);
        } else {
            ps_path_line_to(path, &p);
        }
    }

    auto result = RunBenchmark(Path_PathVertexOperations, [&]() {
        for (int i = 0; i < 5000; i++) {
            for (uint32_t j = 0; j < 50; j++) {
                ps_point point;
                ps_path_cmd cmd = ps_path_get_vertex(path, j, &point);
                UNUSED(cmd);
            }
        }
    });

    CompareToBenchmark(Path_PathVertexOperations, result);
    ps_path_unref(path);
}

// Test 17: Path bounding rectangle
PERF_TEST_RUN(Path, PathBoundingRect)
{
    ps_path* path = ps_path_create();

    // Create a complex path
    for (int i = 0; i < 100; i++) {
        ps_point p = {i * 10.0f, sinf(i * 0.1) * 100 + 200};
        if (i == 0) {
            ps_path_move_to(path, &p);
        } else {
            ps_path_line_to(path, &p);
        }
    }

    auto result = RunBenchmark(Path_PathBoundingRect, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_rect rect;
            ps_path_bounding_rect(path, &rect);
        }
    });

    CompareToBenchmark(Path_PathBoundingRect, result);
    ps_path_unref(path);
}

// Test 18: Path contains test
PERF_TEST_RUN(Path, PathContainsTest)
{
    ps_path* path = ps_path_create();

    // Create a rectangle path
    ps_rect rect = {0, 0, 100, 100};
    ps_path_add_rect(path, &rect);

    ps_point test_point = {50, 50};

    auto result = RunBenchmark(Path_PathContainsTest, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_path_contains(path, &test_point, FILL_RULE_WINDING);
            ps_path_contains(path, &test_point, FILL_RULE_EVEN_ODD);
        }
    });

    CompareToBenchmark(Path_PathContainsTest, result);
    ps_path_unref(path);
}

// Test 19: Path stroke contains test
PERF_TEST_RUN(Path, PathStrokeContainsTest)
{
    ps_path* path = ps_path_create();

    // Create a line path
    ps_point start = {0, 0};
    ps_point end = {100, 100};
    ps_path_add_line(path, &start, &end);

    ps_point test_point = {50, 50};

    auto result = RunBenchmark(Path_PathStrokeContainsTest, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_path_stroke_contains(path, &test_point, 5.0f);
            ps_path_stroke_contains(path, &test_point, 10.0f);
        }
    });

    CompareToBenchmark(Path_PathStrokeContainsTest, result);
    ps_path_unref(path);
}

// Test 20: Path clipping operations
PERF_TEST_RUN(Path, PathClippingOperations)
{
    ps_path* path1 = ps_path_create();
    ps_path* path2 = ps_path_create();
    ps_path* result = ps_path_create();

    // Create two overlapping rectangles
    ps_rect rect1 = {0, 0, 100, 100};
    ps_rect rect2 = {50, 50, 100, 100};
    ps_path_add_rect(path1, &rect1);
    ps_path_add_rect(path2, &rect2);

    auto result_bench = RunBenchmark(Path_PathClippingOperations, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_path_clipping(result, PATH_OP_UNION, path1, path2);
            ps_path_clear(result);
            ps_path_clipping(result, PATH_OP_INTERSECT, path1, path2);
            ps_path_clear(result);
            ps_path_clipping(result, PATH_OP_XOR, path1, path2);
            ps_path_clear(result);
            ps_path_clipping(result, PATH_OP_DIFF, path1, path2);
            ps_path_clear(result);
        }
    });

    CompareToBenchmark(Path_PathClippingOperations, result_bench);
    ps_path_unref(path1);
    ps_path_unref(path2);
    ps_path_unref(result);
}

// Test 21: Path clear operations
PERF_TEST_RUN(Path, PathClearOperations)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathClearOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            // Build a complex path
            for (int j = 0; j < 50; j++) {
                ps_point p = {j * 10.0f, j * 5.0f};
                if (j == 0) {
                    ps_path_move_to(path, &p);
                } else {
                    ps_path_line_to(path, &p);
                }
            }
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathClearOperations, result);
    ps_path_unref(path);
}

// Test 22: Path sub close operations
PERF_TEST_RUN(Path, PathSubCloseOperations)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathSubCloseOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_point start = {0, 0};
            ps_point p1 = {100, 0};
            ps_point p2 = {100, 100};
            ps_point p3 = {0, 100};

            ps_path_move_to(path, &start);
            ps_path_line_to(path, &p1);
            ps_path_line_to(path, &p2);
            ps_path_line_to(path, &p3);
            ps_path_sub_close(path);
            ps_path_clear(path);
        }
    });

    CompareToBenchmark(Path_PathSubCloseOperations, result);
    ps_path_unref(path);
}

// Test 23: Complex path building stress test
PERF_TEST_RUN(Path, PathComplexBuildingStress)
{
    ps_path* path = ps_path_create();

    auto result = RunBenchmark(Path_PathComplexBuildingStress, [&]() {
        for (int i = 0; i < 1000; i++) {
            // Build complex path with multiple shape types
            ps_path_clear(path);

            // Add rectangle
            ps_rect rect = {0, 0, 100, 100};
            ps_path_add_rect(path, &rect);

            // Add ellipse
            ps_rect ellipse = {50, 50, 80, 60};
            ps_path_add_ellipse(path, &ellipse);

            // Add bezier curves
            for (int j = 0; j < 10; j++) {
                ps_point start = {j * 20.0f, 200};
                ps_point cp1 = {j * 20.0f + 10, 180};
                ps_point cp2 = {j * 20.0f + 10, 220};
                ps_point end = {(j + 1) * 20.0f, 200};
                ps_path_move_to(path, &start);
                ps_path_bezier_to(path, &cp1, &cp2, &end);
            }

            // Add arc
            ps_point cp = {300, 100};
            ps_path_add_arc(path, &cp, 50.0f, 0.0f, 3.14159f, False);
        }
    });

    CompareToBenchmark(Path_PathComplexBuildingStress, result);
    ps_path_unref(path);
}

// Test 24: Path transformation stress test
PERF_TEST_RUN(Path, PathTransformStress)
{
    ps_path* path = ps_path_create();
    ps_matrix* matrix = ps_matrix_create();

    // Build a complex path
    for (int i = 0; i < 100; i++) {
        ps_point p = {i * 5.0f, sinf(i * 0.1) * 50};
        if (i == 0) {
            ps_path_move_to(path, &p);
        } else {
            ps_path_line_to(path, &p);
        }
    }

    auto result = RunBenchmark(Path_PathTransformStress, [&]() {
        for (int i = 0; i < 10000; i++) {
            // Apply various transformations
            ps_matrix_identity(matrix);
            ps_matrix_translate(matrix, i * 0.1f, i * 0.1f);
            ps_matrix_rotate(matrix, i * 0.01f);
            ps_matrix_scale(matrix, 1.0f + i * 0.001f, 1.0f + i * 0.001f);
            ps_matrix_transform_path(matrix, path);
        }
    });

    CompareToBenchmark(Path_PathTransformStress, result);
    ps_path_unref(path);
    ps_matrix_unref(matrix);
}

// Test 25: Path memory allocation stress
PERF_TEST_RUN(Path, PathMemoryStress)
{
    const int num_paths = 500;
    ps_path* paths[num_paths];

    auto result = RunBenchmark(Path_PathMemoryStress, [&]() {
        // Create many paths
        for (int i = 0; i < num_paths; i++) {
            paths[i] = ps_path_create();

            // Build each path with different complexity
            for (int j = 0; j < (i % 50 + 10); j++) {
                ps_point p = {j * 10.0f, j * 5.0f};
                if (j == 0) {
                    ps_path_move_to(paths[i], &p);
                } else {
                    ps_path_line_to(paths[i], &p);
                }
            }
        }

        // Reference operations
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < num_paths; j++) {
                ps_path_ref(paths[j]);
            }
            for (int j = 0; j < num_paths; j++) {
                ps_path_unref(paths[j]);
            }
        }

        // Clean up
        for (int i = 0; i < num_paths; i++) {
            ps_path_unref(paths[i]);
        }
    });

    CompareToBenchmark(Path_PathMemoryStress, result);
}

// Test 26: Path clipping performance with complex shapes
PERF_TEST_RUN(Path, PathComplexClipping)
{
    ps_path* path1 = ps_path_create();
    ps_path* path2 = ps_path_create();
    ps_path* result = ps_path_create();

    // Create complex shapes
    // Star shape for path1
    for (int i = 0; i < 10; i++) {
        float angle = i * 3.14159f * 2.0f / 10.0f;
        float x = cosf(angle) * 100 + 200;
        float y = sinf(angle) * 100 + 200;
        ps_point p = {x, y};
        if (i == 0) {
            ps_path_move_to(path1, &p);
        } else {
            ps_path_line_to(path1, &p);
        }

        angle = (i + 0.5f) * 3.14159f * 2.0f / 10.0f;
        x = cosf(angle) * 50 + 200;
        y = sinf(angle) * 50 + 200;
        ps_point inner = {x, y};
        ps_path_line_to(path1, &inner);
    }
    ps_path_sub_close(path1);

    // Circle for path2
    ps_point center = {200, 200};
    ps_path_add_arc(path2, &center, 80.0f, 0.0f, 3.14159f * 2.0f, False);

    auto result_bench = RunBenchmark(Path_PathComplexClipping, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_path_clipping(result, PATH_OP_UNION, path1, path2);
            ps_path_clear(result);
            ps_path_clipping(result, PATH_OP_INTERSECT, path1, path2);
            ps_path_clear(result);
            ps_path_clipping(result, PATH_OP_XOR, path1, path2);
            ps_path_clear(result);
            ps_path_clipping(result, PATH_OP_DIFF, path1, path2);
            ps_path_clear(result);
        }
    });

    CompareToBenchmark(Path_PathComplexClipping, result_bench);
    ps_path_unref(path1);
    ps_path_unref(path2);
    ps_path_unref(result);
}

// Test 27: Path vertex enumeration performance
PERF_TEST_RUN(Path, PathVertexEnumeration)
{
    ps_path* path = ps_path_create();

    // Build path with many vertices
    for (int i = 0; i < 1000; i++) {
        ps_point p = {i * 2.0f, sinf(i * 0.1) * 100};
        if (i == 0) {
            ps_path_move_to(path, &p);
        } else {
            ps_path_line_to(path, &p);
        }
    }

    auto result = RunBenchmark(Path_PathVertexEnumeration, [&]() {
        for (int i = 0; i < 1000; i++) {
            uint32_t count = ps_path_get_vertex_count(path);
            for (uint32_t j = 0; j < count; j++) {
                ps_point point;
                ps_path_cmd cmd = ps_path_get_vertex(path, j, &point);
                UNUSED(cmd);
            }
        }
    });

    CompareToBenchmark(Path_PathVertexEnumeration, result);
    ps_path_unref(path);
}

// Test 28: Path contains point performance with many test points
PERF_TEST_RUN(Path, PathContainsManyPoints)
{
    ps_path* path = ps_path_create();

    // Create a complex shape
    ps_rect rect = {0, 0, 200, 200};
    ps_path_add_rect(path, &rect);
    ps_rect inner = {50, 50, 100, 100};
    ps_path_add_rect(path, &inner);

    // Generate test points
    ps_point test_points[10];
    for (int i = 0; i < 10; i++) {
        test_points[i].x = i * 2;
        test_points[i].y = i * 2;
    }

    auto result = RunBenchmark(Path_PathContainsManyPoints, [&]() {
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 10; j++) {
                ps_path_contains(path, &test_points[j], FILL_RULE_WINDING);
                ps_path_stroke_contains(path, &test_points[j], 5.0f);
            }
        }
    });

    CompareToBenchmark(Path_PathContainsManyPoints, result);
    ps_path_unref(path);
}

// Test 29: Error handling performance
PERF_TEST_RUN(Path, PathErrorHandling)
{
    auto result = RunBenchmark(Path_PathErrorHandling, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_path_create_copy(NULL);
            ps_path_ref(NULL);
            ps_path_unref(NULL);
            ps_path_move_to(NULL, NULL);
            ps_path_line_to(NULL, NULL);
            ps_path_bezier_to(NULL, NULL, NULL, NULL);
            ps_path_quad_to(NULL, NULL, NULL);
            ps_path_arc_to(NULL, 0, 0, 0, False, False, NULL);
            ps_path_tangent_arc_to(NULL, 0, NULL, NULL);
            ps_path_sub_close(NULL);
            ps_path_get_length(NULL);
            ps_path_clear(NULL);
            ps_path_is_empty(NULL);
            ps_path_get_vertex_count(NULL);
            ps_path_get_vertex(NULL, 0, NULL);
            ps_path_bounding_rect(NULL, NULL);
            ps_path_contains(NULL, NULL, FILL_RULE_WINDING);
            ps_path_stroke_contains(NULL, NULL, 0);
            ps_path_add_line(NULL, NULL, NULL);
            ps_path_add_arc(NULL, NULL, 0, 0, 0, False);
            ps_path_add_rect(NULL, NULL);
            ps_path_add_ellipse(NULL, NULL);
            ps_path_add_rounded_rect(NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0);
            ps_path_add_sub_path(NULL, NULL);
            ps_path_clipping(NULL, PATH_OP_UNION, NULL, NULL);
        }
    });

    CompareToBenchmark(Path_PathErrorHandling, result);
}

// Test 30: Path length calculation performance
PERF_TEST_RUN(Path, PathLengthCalculation)
{
    ps_path* paths[10];

    // Create paths with different complexities
    for (int i = 0; i < 10; i++) {
        paths[i] = ps_path_create();
        for (int j = 0; j < (i + 1) * 100; j++) {
            ps_point p = {j * 5.0f, sinf(j * 0.1) * 50};
            if (j == 0) {
                ps_path_move_to(paths[i], &p);
            } else {
                ps_path_line_to(paths[i], &p);
            }
        }
    }

    auto result = RunBenchmark(Path_PathLengthCalculation, [&]() {
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 10; j++) {
                float length = ps_path_get_length(paths[j]);
                UNUSED(length);
            }
        }
    });

    CompareToBenchmark(Path_PathLengthCalculation, result);

    for (int i = 0; i < 10; i++) {
        ps_path_unref(paths[i]);
    }
}
