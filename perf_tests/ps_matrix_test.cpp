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

// Test 10: Matrix initialization
PERF_TEST_RUN(Matrix, MatrixInit)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixInit, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_init(matrix, 2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);
            ps_matrix_init(matrix, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        }
    });

    CompareToBenchmark(Matrix_MatrixInit, result);
    ps_matrix_unref(matrix);
}

// Test 11: Matrix multiplication
PERF_TEST_RUN(Matrix, MatrixMultiply)
{
    ps_matrix* m1 = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);
    ps_matrix* m2 = ps_matrix_create_init(1.5f, 0.2f, 0.4f, 2.0f, 50.0f, 100.0f);
    ps_matrix* result = ps_matrix_create();

    auto bench_result = RunBenchmark(Matrix_MatrixMultiply, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_multiply(result, m1, m2);
            ps_matrix_multiply(result, m2, m1);
        }
    });

    CompareToBenchmark(Matrix_MatrixMultiply, bench_result);
    ps_matrix_unref(result);
    ps_matrix_unref(m1);
    ps_matrix_unref(m2);
}

// Test 12: Matrix identity check
PERF_TEST_RUN(Matrix, MatrixIsIdentity)
{
    ps_matrix* identity = ps_matrix_create();
    ps_matrix* non_identity = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixIsIdentity, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_is_identity(identity);
            ps_matrix_is_identity(non_identity);
        }
    });

    CompareToBenchmark(Matrix_MatrixIsIdentity, result);
    ps_matrix_unref(identity);
    ps_matrix_unref(non_identity);
}

// Test 13: Matrix equality check
PERF_TEST_RUN(Matrix, MatrixIsEqual)
{
    ps_matrix* m1 = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);
    ps_matrix* m2 = ps_matrix_create_copy(m1);
    ps_matrix* m3 = ps_matrix_create_init(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);

    auto result = RunBenchmark(Matrix_MatrixIsEqual, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_is_equal(m1, m2);
            ps_matrix_is_equal(m1, m3);
        }
    });

    CompareToBenchmark(Matrix_MatrixIsEqual, result);
    ps_matrix_unref(m1);
    ps_matrix_unref(m2);
    ps_matrix_unref(m3);
}

// Test 14: Get determinant
PERF_TEST_RUN(Matrix, MatrixGetDeterminant)
{
    ps_matrix* matrix = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixGetDeterminant, [&]() {
        for (int i = 0; i < 100000; i++) {
            float det = ps_matrix_get_determinant(matrix);
            UNUSED(det);
        }
    });

    CompareToBenchmark(Matrix_MatrixGetDeterminant, result);
    ps_matrix_unref(matrix);
}

// Test 15: Get scale factor
PERF_TEST_RUN(Matrix, MatrixGetScaleFactor)
{
    ps_matrix* matrix = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixGetScaleFactor, [&]() {
        for (int i = 0; i < 100000; i++) {
            float sx, sy;
            ps_matrix_get_scale_factor(matrix, &sx, &sy);
        }
    });

    CompareToBenchmark(Matrix_MatrixGetScaleFactor, result);
    ps_matrix_unref(matrix);
}

// Test 16: Set scale factor
PERF_TEST_RUN(Matrix, MatrixSetScaleFactor)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixSetScaleFactor, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_set_scale_factor(matrix, 2.0f, 1.5f);
            ps_matrix_set_scale_factor(matrix, 1.0f, 1.0f);
        }
    });

    CompareToBenchmark(Matrix_MatrixSetScaleFactor, result);
    ps_matrix_unref(matrix);
}

// Test 17: Get shear factor
PERF_TEST_RUN(Matrix, MatrixGetShearFactor)
{
    ps_matrix* matrix = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixGetShearFactor, [&]() {
        for (int i = 0; i < 100000; i++) {
            float shx, shy;
            ps_matrix_get_shear_factor(matrix, &shx, &shy);
        }
    });

    CompareToBenchmark(Matrix_MatrixGetShearFactor, result);
    ps_matrix_unref(matrix);
}

// Test 18: Set shear factor
PERF_TEST_RUN(Matrix, MatrixSetShearFactor)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixSetShearFactor, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_set_shear_factor(matrix, 0.2f, 0.3f);
            ps_matrix_set_shear_factor(matrix, 0.0f, 0.0f);
        }
    });

    CompareToBenchmark(Matrix_MatrixSetShearFactor, result);
    ps_matrix_unref(matrix);
}

// Test 19: Get translate factor
PERF_TEST_RUN(Matrix, MatrixGetTranslateFactor)
{
    ps_matrix* matrix = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixGetTranslateFactor, [&]() {
        for (int i = 0; i < 100000; i++) {
            float tx, ty;
            ps_matrix_get_translate_factor(matrix, &tx, &ty);
        }
    });

    CompareToBenchmark(Matrix_MatrixGetTranslateFactor, result);
    ps_matrix_unref(matrix);
}

// Test 20: Set translate factor
PERF_TEST_RUN(Matrix, MatrixSetTranslateFactor)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixSetTranslateFactor, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_set_translate_factor(matrix, 100.0f, 200.0f);
            ps_matrix_set_translate_factor(matrix, 0.0f, 0.0f);
        }
    });

    CompareToBenchmark(Matrix_MatrixSetTranslateFactor, result);
    ps_matrix_unref(matrix);
}

// Test 21: Transform point
PERF_TEST_RUN(Matrix, MatrixTransformPoint)
{
    ps_matrix* matrix = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixTransformPoint, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_point pt = {i * 0.1f, i * 0.1f};
            ps_matrix_transform_point(matrix, &pt);
        }
    });

    CompareToBenchmark(Matrix_MatrixTransformPoint, result);
    ps_matrix_unref(matrix);
}

// Test 22: Transform rectangle
PERF_TEST_RUN(Matrix, MatrixTransformRect)
{
    ps_matrix* matrix = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixTransformRect, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_rect rect = {i * 0.1f, i * 0.1f, 100, 80};
            ps_matrix_transform_rect(matrix, &rect);
        }
    });

    CompareToBenchmark(Matrix_MatrixTransformRect, result);
    ps_matrix_unref(matrix);
}

// Test 23: Matrix reference counting
PERF_TEST_RUN(Matrix, MatrixReferenceOperations)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixReferenceOperations, [&]() {
        for (int i = 0; i < 100000; i++) {
            ps_matrix_ref(matrix);
            ps_matrix_unref(matrix);
        }
    });

    CompareToBenchmark(Matrix_MatrixReferenceOperations, result);
    ps_matrix_unref(matrix);
}

// Test 24: Matrix decomposition (get all factors)
PERF_TEST_RUN(Matrix, MatrixDecomposition)
{
    ps_matrix* matrix = ps_matrix_create_init(2.0f, 0.5f, 0.3f, 1.5f, 100.0f, 200.0f);

    auto result = RunBenchmark(Matrix_MatrixDecomposition, [&]() {
        for (int i = 0; i < 50000; i++) {
            float sx, sy, shx, shy, tx, ty;
            ps_matrix_get_scale_factor(matrix, &sx, &sy);
            ps_matrix_get_shear_factor(matrix, &shx, &shy);
            ps_matrix_get_translate_factor(matrix, &tx, &ty);
        }
    });

    CompareToBenchmark(Matrix_MatrixDecomposition, result);
    ps_matrix_unref(matrix);
}

// Test 25: Matrix composition (set all factors)
PERF_TEST_RUN(Matrix, MatrixComposition)
{
    ps_matrix* matrix = ps_matrix_create();

    auto result = RunBenchmark(Matrix_MatrixComposition, [&]() {
        for (int i = 0; i < 50000; i++) {
            ps_matrix_set_scale_factor(matrix, 2.0f, 1.5f);
            ps_matrix_set_shear_factor(matrix, 0.2f, 0.3f);
            ps_matrix_set_translate_factor(matrix, 100.0f, 200.0f);
        }
    });

    CompareToBenchmark(Matrix_MatrixComposition, result);
    ps_matrix_unref(matrix);
}
