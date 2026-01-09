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

PERF_TEST_DEFINE(Matrix);

PERF_TEST(Matrix, MatrixCreateDestroy)
{
    for (int i = 0; i < 10000; i++) {
        ps_matrix* m = ps_matrix_create();
        ps_matrix_unref(m);
    }
}

PERF_TEST(Matrix, MatrixCreateInit)
{
    for (int i = 0; i < 10000; i++) {
        ps_matrix* matrix = ps_matrix_create_init(1.0f, 0.0f, 0.0f, 1.0f, 10.0f, 20.0f);
        ps_matrix_unref(matrix);
    }
}

PERF_TEST_RUN(Matrix, MatrixCreateCopy)
{
    ps_matrix* src = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixCreateCopy, [&]() {
        for (int i = 0; i < 10000; i++) {
            ps_matrix* copy = ps_matrix_create_copy(src);
            ps_matrix_unref(copy);
        }
    });

    CompareToBenchmark(Matrix_MatrixCreateCopy, result);
    ps_matrix_unref(src);
}

PERF_TEST_RUN(Matrix, MatrixTranslate)
{
    ps_matrix* m = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixTranslate, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_translate(m, 10.0f, 20.0f);
            ps_matrix_translate(m, -20.0f, -10.0f);
        }
    });

    CompareToBenchmark(Matrix_MatrixTranslate, result);
    ps_matrix_unref(m);
}

PERF_TEST_RUN(Matrix, MatrixScale)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixScale, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_scale(matrix, 2.0f, 1.5f);
            ps_matrix_scale(matrix, 0.5f, 0.6667f);
        }
    });

    CompareToBenchmark(Matrix_MatrixScale, result);
    ps_matrix_unref(matrix);
}

PERF_TEST_RUN(Matrix, MatrixShear)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixShear, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_shear(matrix, 0.1f, 0.2f);
            ps_matrix_shear(matrix, -0.1f, -0.2f);
        }
    });

    CompareToBenchmark(Matrix_MatrixShear, result);
    ps_matrix_unref(matrix);
}

PERF_TEST_RUN(Matrix, MatrixRotate)
{
    ps_matrix* matrix = ps_matrix_create();
    const float angle = 3.14159f / 180.0f; // 1 degree in radians

    auto result = RunBenchmark(Matrix_MatrixRotate, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_rotate(matrix, angle);
            ps_matrix_rotate(matrix, -angle);
        }
    });

    CompareToBenchmark(Matrix_MatrixRotate, result);
    ps_matrix_unref(matrix);
}

PERF_TEST_RUN(Matrix, MatrixInvert)
{
    ps_matrix* matrix = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixInvert, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_invert(matrix);
            ps_matrix_invert(matrix);
        }
    });

    CompareToBenchmark(Matrix_MatrixInvert, result);
    ps_matrix_unref(matrix);
}

PERF_TEST_RUN(Matrix, MatrixFlip)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixFlip, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_flip_x(matrix);
            ps_matrix_flip_y(matrix);
            ps_matrix_flip_x(matrix);
            ps_matrix_flip_y(matrix);
        }
    });

    CompareToBenchmark(Matrix_MatrixFlip, result);
    ps_matrix_unref(matrix);
}
