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

PERF_TEST_DEFINE(Context);

PERF_TEST_RUN(Context, ContextMultipleCreate)
{
    ps_canvas* canvas = get_test_canvas();

    auto result = RunBenchmark(Context_ContextMultipleCreate, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_context* ctx = ps_context_create(canvas, NULL);
            ps_context_unref(ctx);
        }
    });

    CompareToBenchmark(Context_ContextMultipleCreate, result);
}

PERF_TEST_RUN(Context, ContextStateOperations)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};

    auto result = RunBenchmark(Context_ContextStateOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_set_source_color(ctx, &color);
            ps_set_line_width(ctx, 2.0f);
            ps_set_line_cap(ctx, LINE_CAP_BUTT);
            ps_set_line_join(ctx, LINE_JOIN_MITER);
        }
    });

    CompareToBenchmark(Context_ContextStateOperations, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Context, ContextCreateWithShared)
{
    ps_context* shared_ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Context_ContextCreateWithShared, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_context* ctx = ps_context_create(get_test_canvas(), shared_ctx);
            ps_context_unref(ctx);
        }
    });

    CompareToBenchmark(Context_ContextCreateWithShared, result);
    ps_context_unref(shared_ctx);
}

PERF_TEST_RUN(Context, ContextReferenceOperations)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Context_ContextReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_context_ref(ctx);
            ps_context_unref(ctx);
        }
    });

    CompareToBenchmark(Context_ContextReferenceOperations, result);
    ps_context_unref(ctx);
}

PERF_TEST_RUN(Context, ContextMultipleReferences)
{
    auto result = RunBenchmark(Context_ContextMultipleReferences, [&]() {
        for (int i = 0; i < 1000; i++) {
            ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
            ps_context_ref(ctx);
            ps_context_ref(ctx);
            ps_context_unref(ctx);
            ps_context_unref(ctx);
            ps_context_unref(ctx);
        }
    });

    CompareToBenchmark(Context_ContextMultipleReferences, result);
}

PERF_TEST_RUN(Context, ContextCanvasOperations)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_canvas* canvas2 = ps_canvas_create(COLOR_FORMAT_RGBA, 800, 600);
    ps_canvas* canvas3 = ps_canvas_create(COLOR_FORMAT_ARGB, 1024, 768);

    auto result = RunBenchmark(Context_ContextCanvasOperations, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_canvas* old = ps_context_set_canvas(ctx, canvas2);
            ps_context_get_canvas(ctx);
            ps_context_set_canvas(ctx, canvas3);
            ps_context_get_canvas(ctx);
            ps_context_set_canvas(ctx, old);
        }
    });

    CompareToBenchmark(Context_ContextCanvasOperations, result);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas2);
    ps_canvas_unref(canvas3);
}

PERF_TEST_RUN(Context, ContextCanvasFormatOperations)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_canvas* canvases[8];

    // Create canvases with different formats
    canvases[0] = ps_canvas_create(COLOR_FORMAT_RGBA, 640, 480);
    canvases[1] = ps_canvas_create(COLOR_FORMAT_ARGB, 640, 480);
    canvases[2] = ps_canvas_create(COLOR_FORMAT_ABGR, 640, 480);
    canvases[3] = ps_canvas_create(COLOR_FORMAT_BGRA, 640, 480);
    canvases[4] = ps_canvas_create(COLOR_FORMAT_RGB, 640, 480);
    canvases[5] = ps_canvas_create(COLOR_FORMAT_BGR, 640, 480);
    canvases[6] = ps_canvas_create(COLOR_FORMAT_RGB565, 640, 480);
    canvases[7] = ps_canvas_create(COLOR_FORMAT_A8, 640, 480);

    auto result = RunBenchmark(Context_ContextCanvasFormatOperations, [&]() {
        for (int i = 0; i < 10000; i++) {
            for (int j = 0; j < 8; j++) {
                ps_context_set_canvas(ctx, canvases[j]);
                ps_context_get_canvas(ctx);
            }
        }
    });

    CompareToBenchmark(Context_ContextCanvasFormatOperations, result);

    ps_context_unref(ctx);
    for (int i = 0; i < 8; i++) {
        ps_canvas_unref(canvases[i]);
    }
}

PERF_TEST_RUN(Context, ContextStressTest)
{
    const int num_contexts = 1000;
    ps_context* contexts[num_contexts];

    auto result = RunBenchmark(Context_ContextStressTest, [&]() {
        // Create many contexts
        for (int i = 0; i < num_contexts; i++) {
            contexts[i] = ps_context_create(get_test_canvas(), NULL);
        }

        // Reference and unreferfence operations
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < num_contexts; j++) {
                ps_context_ref(contexts[j]);
            }
            for (int j = 0; j < num_contexts; j++) {
                ps_context_unref(contexts[j]);
            }
        }

        // Clean up
        for (int i = 0; i < num_contexts; i++) {
            ps_context_unref(contexts[i]);
        }
    });

    CompareToBenchmark(Context_ContextStressTest, result);
}

PERF_TEST_RUN(Context, ContextErrorHandling)
{
    auto result = RunBenchmark(Context_ContextErrorHandling, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_context_create(NULL, NULL);
            ps_context_ref(NULL);
            ps_context_unref(NULL);
            ps_context_set_canvas(NULL, NULL);
            ps_context_get_canvas(NULL);
        }
    });

    CompareToBenchmark(Context_ContextErrorHandling, result);
}

PERF_TEST_RUN(Context, ContextWithDataCanvas)
{
    uint8_t* buffer = (uint8_t*)malloc(640 * 480 * 4);
    ps_canvas* data_canvas = ps_canvas_create_with_data(buffer, COLOR_FORMAT_RGBA, 640, 480, 640 * 4);

    auto result = RunBenchmark(Context_ContextWithDataCanvas, [&]() {
        for (int i = 0; i < 5000; i++) {
            ps_context* ctx = ps_context_create(data_canvas, NULL);
            ps_context_get_canvas(ctx);
            ps_context_unref(ctx);
        }
    });

    CompareToBenchmark(Context_ContextWithDataCanvas, result);

    ps_canvas_unref(data_canvas);
    free(buffer);
}

PERF_TEST_RUN(Context, ContextSharedResourceChain)
{
    ps_context* root = ps_context_create(get_test_canvas(), NULL);
    ps_context* child1 = ps_context_create(get_test_canvas(), root);
    ps_context* child2 = ps_context_create(get_test_canvas(), child1);
    ps_context* child3 = ps_context_create(get_test_canvas(), child2);

    auto result = RunBenchmark(Context_ContextSharedResourceChain, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_context_ref(root);
            ps_context_ref(child1);
            ps_context_ref(child2);
            ps_context_ref(child3);

            ps_context_get_canvas(root);
            ps_context_get_canvas(child1);
            ps_context_get_canvas(child2);
            ps_context_get_canvas(child3);

            ps_context_unref(child3);
            ps_context_unref(child2);
            ps_context_unref(child1);
            ps_context_unref(root);
        }
    });

    CompareToBenchmark(Context_ContextSharedResourceChain, result);

    ps_context_unref(child3);
    ps_context_unref(child2);
    ps_context_unref(child1);
    ps_context_unref(root);
}

// Test 13: Coordinate conversion - viewport to world
PERF_TEST_RUN(Context, ViewportToWorld)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Context_ViewportToWorld, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_point pt = {i * 0.1f, i * 0.1f};
            ps_viewport_to_world(ctx, &pt);
        }
    });

    CompareToBenchmark(Context_ViewportToWorld, result);
    ps_context_unref(ctx);
}

// Test 14: Coordinate conversion - world to viewport
PERF_TEST_RUN(Context, WorldToViewport)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Context_WorldToViewport, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_point pt = {i * 0.1f, i * 0.1f};
            ps_world_to_viewport(ctx, &pt);
        }
    });

    CompareToBenchmark(Context_WorldToViewport, result);
    ps_context_unref(ctx);
}

// Test 15: Coordinate conversion round-trip
PERF_TEST_RUN(Context, CoordinateConversionRoundTrip)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    auto result = RunBenchmark(Context_CoordinateConversionRoundTrip, [&]() {
        for (int i = 0; i < 25000; i++) {
            ps_point pt = {i * 0.1f, i * 0.1f};
            ps_world_to_viewport(ctx, &pt);
            ps_viewport_to_world(ctx, &pt);
        }
    });

    CompareToBenchmark(Context_CoordinateConversionRoundTrip, result);
    ps_context_unref(ctx);
}

// Test 16: Get matrix
PERF_TEST_RUN(Context, GetMatrix)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Context_GetMatrix, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_get_matrix(ctx, matrix);
        }
    });

    CompareToBenchmark(Context_GetMatrix, result);
    ps_context_unref(ctx);
    ps_matrix_unref(matrix);
}

// Test 17: Set matrix
PERF_TEST_RUN(Context, SetMatrix)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_matrix* matrix1 = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 100.0f, 100.0f);
    ps_matrix* matrix2 = ps_matrix_create_init(1.5f, 0.0f, 0.0f, 1.5f, 50.0f, 50.0f);

    auto result = RunBenchmark(Context_SetMatrix, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_set_matrix(ctx, matrix1);
            ps_set_matrix(ctx, matrix2);
        }
    });

    CompareToBenchmark(Context_SetMatrix, result);
    ps_context_unref(ctx);
    ps_matrix_unref(matrix1);
    ps_matrix_unref(matrix2);
}

// Test 18: Get path
PERF_TEST_RUN(Context, GetPath)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_path* path = ps_path_create();

    // Build a path in context
    ps_new_path(ctx);
    ps_rect rect = {10, 10, 100, 80};
    ps_rectangle(ctx, &rect);

    auto result = RunBenchmark(Context_GetPath, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_get_path(ctx, path);
        }
    });

    CompareToBenchmark(Context_GetPath, result);
    ps_path_unref(path);
    ps_context_unref(ctx);
}

// Test 19: Matrix get/set operations
PERF_TEST_RUN(Context, MatrixGetSetOperations)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Context_MatrixGetSetOperations, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_get_matrix(ctx, matrix);
            ps_matrix_translate(matrix, 10.0f, 10.0f);
            ps_set_matrix(ctx, matrix);
        }
    });

    CompareToBenchmark(Context_MatrixGetSetOperations, result);
    ps_context_unref(ctx);
    ps_matrix_unref(matrix);
}

// Test 20: Coordinate conversion with transformation
PERF_TEST_RUN(Context, CoordinateConversionWithTransform)
{
    ps_context* ctx = ps_context_create(get_test_canvas(), NULL);

    // Apply some transformations
    ps_translate(ctx, 100, 100);
    ps_scale(ctx, 2.0f, 2.0f);
    ps_rotate(ctx, 0.785f); // 45 degrees

    auto result = RunBenchmark(Context_CoordinateConversionWithTransform, [&]() {
        for (int i = 0; i < 25000; i++) {
            ps_point pt = {i * 0.1f, i * 0.1f};
            ps_world_to_viewport(ctx, &pt);
            ps_viewport_to_world(ctx, &pt);
        }
    });

    CompareToBenchmark(Context_CoordinateConversionWithTransform, result);
    ps_context_unref(ctx);
}
