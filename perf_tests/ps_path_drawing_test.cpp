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

PERF_TEST_DEFINE(PathDrawing);

// Test 1: Basic path construction - new_path and move_to
PERF_TEST_RUN(PathDrawing, NewPathMoveTo)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_NewPathMoveTo, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_point pt = {i * 0.1f, i * 0.1f};
            ps_move_to(ctx, &pt);
        }
    });

    CompareToBenchmark(PathDrawing_NewPathMoveTo, result);
    ps_context_unref(ctx);
}

// Test 2: Line drawing - line_to
PERF_TEST_RUN(PathDrawing, LineTo)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_LineTo, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_new_path(ctx);
            ps_point start = {0, 0};
            ps_move_to(ctx, &start);
            for (int j = 0; j < 10; j++) {
                ps_point pt = {j * 10.0f, j * 10.0f};
                ps_line_to(ctx, &pt);
            }
        }
    });

    CompareToBenchmark(PathDrawing_LineTo, result);
    ps_context_unref(ctx);
}

// Test 3: Path closing - close_path
PERF_TEST_RUN(PathDrawing, ClosePath)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_ClosePath, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_point p1 = {0, 0};
            ps_point p2 = {100, 0};
            ps_point p3 = {100, 100};
            ps_point p4 = {0, 100};
            ps_move_to(ctx, &p1);
            ps_line_to(ctx, &p2);
            ps_line_to(ctx, &p3);
            ps_line_to(ctx, &p4);
            ps_close_path(ctx);
        }
    });

    CompareToBenchmark(PathDrawing_ClosePath, result);
    ps_context_unref(ctx);
}

// Test 4: Sub-path creation - new_sub_path
PERF_TEST_RUN(PathDrawing, NewSubPath)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_NewSubPath, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_new_path(ctx);

            // First sub-path
            ps_point p1 = {0, 0};
            ps_point p2 = {50, 0};
            ps_point p3 = {50, 50};
            ps_move_to(ctx, &p1);
            ps_line_to(ctx, &p2);
            ps_line_to(ctx, &p3);
            ps_close_path(ctx);

            // Second sub-path
            ps_new_sub_path(ctx);
            ps_point p4 = {100, 100};
            ps_point p5 = {150, 100};
            ps_point p6 = {150, 150};
            ps_move_to(ctx, &p4);
            ps_line_to(ctx, &p5);
            ps_line_to(ctx, &p6);
            ps_close_path(ctx);
        }
    });

    CompareToBenchmark(PathDrawing_NewSubPath, result);
    ps_context_unref(ctx);
}

// Test 5: Arc drawing
PERF_TEST_RUN(PathDrawing, Arc)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_Arc, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_point center = {100, 100};
            ps_arc(ctx, &center, 50, 0, 3.14159f, False);
        }
    });

    CompareToBenchmark(PathDrawing_Arc, result);
    ps_context_unref(ctx);
}

// Test 6: Full circle arc
PERF_TEST_RUN(PathDrawing, ArcFullCircle)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_ArcFullCircle, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_point center = {100, 100};
            ps_arc(ctx, &center, 50, 0, 6.28318f, False);
        }
    });

    CompareToBenchmark(PathDrawing_ArcFullCircle, result);
    ps_context_unref(ctx);
}

// Test 7: Tangent arc drawing
PERF_TEST_RUN(PathDrawing, TangentArc)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_TangentArc, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_point start = {0, 0};
            ps_move_to(ctx, &start);
            ps_rect rect = {50, 0, 100, 100};
            ps_tangent_arc(ctx, &rect, 0, 1.57f);
        }
    });

    CompareToBenchmark(PathDrawing_TangentArc, result);
    ps_context_unref(ctx);
}

// Test 8: Ellipse drawing
PERF_TEST_RUN(PathDrawing, Ellipse)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_Ellipse, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_rect rect = {100, 100, 80, 50};
            ps_ellipse(ctx, &rect);
        }
    });

    CompareToBenchmark(PathDrawing_Ellipse, result);
    ps_context_unref(ctx);
}

// Test 9: Rectangle drawing
PERF_TEST_RUN(PathDrawing, Rectangle)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_Rectangle, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_rect rect = {i * 0.1f, i * 0.1f, 100, 80};
            ps_rectangle(ctx, &rect);
        }
    });

    CompareToBenchmark(PathDrawing_Rectangle, result);
    ps_context_unref(ctx);
}

// Test 10: Rounded rectangle drawing
PERF_TEST_RUN(PathDrawing, RoundedRect)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_RoundedRect, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_rect rect = {10, 10, 100, 80};
            ps_rounded_rect(ctx, &rect, 10, 10, 10, 10, 10, 10, 10, 10);
        }
    });

    CompareToBenchmark(PathDrawing_RoundedRect, result);
    ps_context_unref(ctx);
}

// Test 11: Bezier curves
PERF_TEST_RUN(PathDrawing, BezierCurveTo)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_BezierCurveTo, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_point start = {0, 0};
            ps_move_to(ctx, &start);
            ps_point cp1 = {25, 0};
            ps_point cp2 = {75, 100};
            ps_point end = {100, 100};
            ps_bezier_to(ctx, &cp1, &cp2, &end);
        }
    });

    CompareToBenchmark(PathDrawing_BezierCurveTo, result);
    ps_context_unref(ctx);
}

// Test 12: Quadratic curves
PERF_TEST_RUN(PathDrawing, QuadCurveTo)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_QuadCurveTo, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_point start = {0, 0};
            ps_move_to(ctx, &start);
            ps_point cp = {50, 0};
            ps_point end = {100, 100};
            ps_quad_to(ctx, &cp, &end);
        }
    });

    CompareToBenchmark(PathDrawing_QuadCurveTo, result);
    ps_context_unref(ctx);
}

// Test 13: Complex path with multiple shapes
PERF_TEST_RUN(PathDrawing, ComplexPath)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_ComplexPath, [&]() {
        for (int i = 0; i < 2000; i++) {
            ps_new_path(ctx);

            // Rectangle
            ps_rect rect1 = {10, 10, 100, 80};
            ps_rectangle(ctx, &rect1);

            // Ellipse
            ps_rect rect2 = {200, 50, 60, 40};
            ps_ellipse(ctx, &rect2);

            // Rounded rect
            ps_rect rect3 = {300, 10, 100, 80};
            ps_rounded_rect(ctx, &rect3, 10, 10, 10, 10, 10, 10, 10, 10);

            // Arc
            ps_point center = {500, 50};
            ps_arc(ctx, &center, 40, 0, 3.14159f, False);
        }
    });

    CompareToBenchmark(PathDrawing_ComplexPath, result);
    ps_context_unref(ctx);
}

// Test 14: Polyline (multiple line segments)
PERF_TEST_RUN(PathDrawing, Polyline)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_Polyline, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_new_path(ctx);
            ps_point start = {0, 0};
            ps_move_to(ctx, &start);

            for (int j = 1; j < 20; j++) {
                ps_point pt = {j * 10.0f, sinf(j * 0.2f) * 50 + 100};
                ps_line_to(ctx, &pt);
            }
        }
    });

    CompareToBenchmark(PathDrawing_Polyline, result);
    ps_context_unref(ctx);
}

// Test 15: Polygon (closed polyline)
PERF_TEST_RUN(PathDrawing, Polygon)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_Polygon, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);
            ps_point center = {100, 50};
            ps_move_to(ctx, &center);

            // Draw hexagon
            for (int j = 0; j < 6; j++) {
                float angle = j * 3.14159f / 3.0f;
                ps_point pt = {100 + cosf(angle) * 50, 100 + sinf(angle) * 50};
                ps_line_to(ctx, &pt);
            }
            ps_close_path(ctx);
        }
    });

    CompareToBenchmark(PathDrawing_Polygon, result);
    ps_context_unref(ctx);
}

// Test 16: Star shape
PERF_TEST_RUN(PathDrawing, StarShape)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_StarShape, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_new_path(ctx);

            // Draw 5-pointed star
            for (int j = 0; j < 10; j++) {
                float angle = j * 3.14159f / 5.0f;
                float radius = (j % 2 == 0) ? 50.0f : 25.0f;
                float x = 100 + cosf(angle) * radius;
                float y = 100 + sinf(angle) * radius;
                ps_point pt = {x, y};

                if (j == 0) {
                    ps_move_to(ctx, &pt);
                } else {
                    ps_line_to(ctx, &pt);
                }
            }
            ps_close_path(ctx);
        }
    });

    CompareToBenchmark(PathDrawing_StarShape, result);
    ps_context_unref(ctx);
}

// Test 17: Smooth curve path
PERF_TEST_RUN(PathDrawing, SmoothCurvePath)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_SmoothCurvePath, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_new_path(ctx);
            ps_point start = {0, 100};
            ps_move_to(ctx, &start);

            // Draw smooth curve using multiple bezier segments
            for (int j = 0; j < 5; j++) {
                float x1 = j * 40.0f + 10;
                float y1 = 100 + sinf(j * 0.5f) * 30;
                float x2 = j * 40.0f + 30;
                float y2 = 100 + cosf(j * 0.5f) * 30;
                float x3 = (j + 1) * 40.0f;
                float y3 = 100;

                ps_point cp1 = {x1, y1};
                ps_point cp2 = {x2, y2};
                ps_point end = {x3, y3};
                ps_bezier_to(ctx, &cp1, &cp2, &end);
            }
        }
    });

    CompareToBenchmark(PathDrawing_SmoothCurvePath, result);
    ps_context_unref(ctx);
}

// Test 18: Multiple sub-paths
PERF_TEST_RUN(PathDrawing, MultipleSubPaths)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_MultipleSubPaths, [&]() {
        for (int i = 0; i < 2000; i++) {
            ps_new_path(ctx);

            // Create 5 separate rectangles as sub-paths
            for (int j = 0; j < 5; j++) {
                ps_rect rect = {j * 50.0f, j * 50.0f, 40, 40};
                ps_rectangle(ctx, &rect);
            }
        }
    });

    CompareToBenchmark(PathDrawing_MultipleSubPaths, result);
    ps_context_unref(ctx);
}

// Test 19: Arcs with various angles
PERF_TEST_RUN(PathDrawing, ArcVariousAngles)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_ArcVariousAngles, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_new_path(ctx);

            float start_angle = i * 0.01f;
            float end_angle = start_angle + 1.57f; // 90 degrees

            ps_point center = {100, 100};
            ps_arc(ctx, &center, 50, start_angle, end_angle, False);
        }
    });

    CompareToBenchmark(PathDrawing_ArcVariousAngles, result);
    ps_context_unref(ctx);
}

// Test 20: Ellipses with various sizes
PERF_TEST_RUN(PathDrawing, EllipseVariousSizes)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(PathDrawing_EllipseVariousSizes, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_new_path(ctx);

            float rx = 20 + (i % 80);
            float ry = 15 + (i % 60);

            ps_rect rect = {100, 100, rx, ry};
            ps_ellipse(ctx, &rect);
        }
    });

    CompareToBenchmark(PathDrawing_EllipseVariousSizes, result);
    ps_context_unref(ctx);
}
