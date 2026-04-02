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

TEST_F(MatrixTest, CreateDestroy)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_TRUE(ps_matrix_is_identity(m));
    ps_matrix_unref(m);
}

TEST_F(MatrixTest, CreateInit)
{
    ps_matrix* m = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 10.0f, 20.0f);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_FALSE(ps_matrix_is_identity(m));
    ps_matrix_unref(m);

    m = ps_matrix_create_init(1, 0, 0, 1, 0, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_TRUE(ps_matrix_is_identity(m));
    ps_matrix_unref(m);
}

TEST_F(MatrixTest, CreateCopy)
{
    ps_matrix* original = ps_matrix_create_init(1.5f, 0.5f, 0.3f, 2.0f, 5.0f, 10.0f);
    ASSERT_NE(original, nullptr);

    ps_matrix* copy = ps_matrix_create_copy(original);
    ASSERT_NE(copy, nullptr);
    EXPECT_TRUE(ps_matrix_is_equal(original, copy));

    ps_matrix_unref(original);
    ps_matrix_unref(copy);
}

TEST_F(MatrixTest, ReferenceCounting)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    // Test ref
    ps_matrix* ref = ps_matrix_ref(m);
    EXPECT_EQ(ref, m);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test unref (should not destroy yet)
    ps_matrix_unref(m);

    // Second unref should destroy
    ps_matrix_unref(ref);
}

TEST_F(MatrixTest, Init)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_init(m, 3.0f, 0.0f, 0.0f, 3.0f, 15.0f, 25.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_FALSE(ps_matrix_is_identity(m));

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, Translate)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_translate(m, 5.0f, 10.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    float tx, ty;
    EXPECT_TRUE(ps_matrix_get_translate_factor(m, &tx, &ty));
    EXPECT_FLOAT_EQ(tx, 5.0f);
    EXPECT_FLOAT_EQ(ty, 10.0f);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, Scale)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_scale(m, 2.0f, 3.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    float sx, sy;
    EXPECT_TRUE(ps_matrix_get_scale_factor(m, &sx, &sy));
    EXPECT_FLOAT_EQ(sx, 2.0f);
    EXPECT_FLOAT_EQ(sy, 3.0f);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, Rotate)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_rotate(m, 90.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_FALSE(ps_matrix_is_identity(m));

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, Shear)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_shear(m, 0.5f, 0.3f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    float shx, shy;
    EXPECT_TRUE(ps_matrix_get_shear_factor(m, &shx, &shy));
    EXPECT_FLOAT_EQ(shx, 0.5f);
    EXPECT_FLOAT_EQ(shy, 0.3f);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, Invert)
{
    ps_matrix* m = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 10.0f, 20.0f);
    ASSERT_NE(m, nullptr);

    ps_matrix_invert(m);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test that invert worked by multiplying with original
    ps_matrix* original = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 10.0f, 20.0f);
    ps_matrix* result = ps_matrix_create();
    ps_matrix_multiply(result, m, original);
    EXPECT_TRUE(ps_matrix_is_identity(result));

    ps_matrix_unref(m);
    ps_matrix_unref(original);
    ps_matrix_unref(result);
}

TEST_F(MatrixTest, Identity)
{
    ps_matrix* m = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 10.0f, 20.0f);
    ASSERT_NE(m, nullptr);

    ps_matrix_identity(m);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_TRUE(ps_matrix_is_identity(m));

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, FlipX)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_flip_x(m);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_FALSE(ps_matrix_is_identity(m));

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, FlipY)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_flip_y(m);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_FALSE(ps_matrix_is_identity(m));

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, Multiply)
{
    ps_matrix* a = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 5.0f, 10.0f);
    ps_matrix* b = ps_matrix_create_init(3.0f, 0.0f, 0.0f, 3.0f, 15.0f, 20.0f);
    ps_matrix* result = ps_matrix_create();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(result, nullptr);

    ps_matrix_multiply(result, a, b);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_FALSE(ps_matrix_is_identity(result));

    ps_matrix_unref(a);
    ps_matrix_unref(b);
    ps_matrix_unref(result);
}

TEST_F(MatrixTest, IsIdentity)
{
    ps_matrix* identity = ps_matrix_create();
    ps_matrix* transformed = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 5.0f, 10.0f);
    ASSERT_NE(identity, nullptr);
    ASSERT_NE(transformed, nullptr);

    EXPECT_TRUE(ps_matrix_is_identity(identity));
    EXPECT_FALSE(ps_matrix_is_identity(transformed));

    ps_matrix_unref(identity);
    ps_matrix_unref(transformed);
}

TEST_F(MatrixTest, IsEqual)
{
    ps_matrix* a = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 5.0f, 10.0f);
    ps_matrix* b = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 5.0f, 10.0f);
    ps_matrix* c = ps_matrix_create_init(3.0f, 0.0f, 0.0f, 3.0f, 15.0f, 20.0f);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);

    EXPECT_TRUE(ps_matrix_is_equal(a, b));
    EXPECT_FALSE(ps_matrix_is_equal(a, c));
    EXPECT_TRUE(ps_matrix_is_equal(a, a)); // Same object

    ps_matrix_unref(a);
    ps_matrix_unref(b);
    ps_matrix_unref(c);
}

TEST_F(MatrixTest, GetDeterminant)
{
    ps_matrix* identity = ps_matrix_create();
    ps_matrix* scaled = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f);
    ASSERT_NE(identity, nullptr);
    ASSERT_NE(scaled, nullptr);

    EXPECT_FLOAT_EQ(ps_matrix_get_determinant(identity), 1.0f);
    EXPECT_FLOAT_EQ(ps_matrix_get_determinant(scaled), 6.0f); // 2 * 3

    ps_matrix_unref(identity);
    ps_matrix_unref(scaled);
}

TEST_F(MatrixTest, SetGetTranslateFactor)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_set_translate_factor(m, 15.0f, 25.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    float tx, ty;
    EXPECT_TRUE(ps_matrix_get_translate_factor(m, &tx, &ty));
    EXPECT_FLOAT_EQ(tx, 15.0f);
    EXPECT_FLOAT_EQ(ty, 25.0f);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, SetGetScaleFactor)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_set_scale_factor(m, 4.0f, 5.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    float sx, sy;
    EXPECT_TRUE(ps_matrix_get_scale_factor(m, &sx, &sy));
    EXPECT_FLOAT_EQ(sx, 4.0f);
    EXPECT_FLOAT_EQ(sy, 5.0f);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, SetGetShearFactor)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    ps_matrix_set_shear_factor(m, 0.7f, 0.9f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    float shx, shy;
    EXPECT_TRUE(ps_matrix_get_shear_factor(m, &shx, &shy));
    EXPECT_FLOAT_EQ(shx, 0.7f);
    EXPECT_FLOAT_EQ(shy, 0.9f);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, TransformPoint)
{
    ps_matrix* m = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 3.0f, 10.0f, 20.0f);
    ASSERT_NE(m, nullptr);

    ps_point point = {5.0f, 7.0f};
    ps_matrix_transform_point(m, &point);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    EXPECT_FLOAT_EQ(point.x, 20.0f); // 5*2 + 10
    EXPECT_FLOAT_EQ(point.y, 41.0f); // 7*3 + 20

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, TransformRect)
{
    ps_matrix* m = ps_matrix_create_init(2.0f, 0.0f, 0.0f, 2.0f, 5.0f, 10.0f);
    ASSERT_NE(m, nullptr);

    ps_rect rect = {10.0f, 20.0f, 30.0f, 40.0f};
    ps_matrix_transform_rect(m, &rect);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    EXPECT_FLOAT_EQ(rect.x, 25.0f); // 10*2 + 5
    EXPECT_FLOAT_EQ(rect.y, 50.0f); // 20*2 + 10
    EXPECT_FLOAT_EQ(rect.w, 60.0f); // 30*2
    EXPECT_FLOAT_EQ(rect.h, 80.0f); // 40*2

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, ErrorHandling)
{
    // Test null pointer handling
    EXPECT_EQ(ps_matrix_create_copy(nullptr), nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    EXPECT_EQ(ps_matrix_ref(nullptr), nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_init(nullptr, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_translate(nullptr, 1.0f, 1.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    EXPECT_FALSE(ps_matrix_is_identity(nullptr));
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    EXPECT_FALSE(ps_matrix_is_equal(nullptr, nullptr));
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    EXPECT_FLOAT_EQ(ps_matrix_get_determinant(nullptr), 0.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);
}

TEST_F(MatrixTest, ComplexTransformations)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    // Apply multiple transformations
    ps_matrix_translate(m, 10.0f, 20.0f);
    ps_matrix_scale(m, 2.0f, 3.0f);
    ps_matrix_rotate(m, 45.0f);
    ps_matrix_shear(m, 0.1f, 0.1f);

    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);
    EXPECT_FALSE(ps_matrix_is_identity(m));

    // Test that we can still get factors
    float tx, ty, sx, sy, shx, shy;
    EXPECT_TRUE(ps_matrix_get_translate_factor(m, &tx, &ty));
    EXPECT_TRUE(ps_matrix_get_scale_factor(m, &sx, &sy));
    EXPECT_TRUE(ps_matrix_get_shear_factor(m, &shx, &shy));

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, NullPointerHandling)
{
    // Test all functions with null pointers
    ps_matrix_translate(nullptr, 1.0f, 1.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_scale(nullptr, 1.0f, 1.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_rotate(nullptr, 45.0f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_shear(nullptr, 0.1f, 0.1f);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_invert(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_flip_x(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_flip_y(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_identity(nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_multiply(nullptr, nullptr, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    // Test getter functions with null output parameters
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    EXPECT_FALSE(ps_matrix_get_translate_factor(m, nullptr, nullptr));
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    EXPECT_FALSE(ps_matrix_get_scale_factor(m, nullptr, nullptr));
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    EXPECT_FALSE(ps_matrix_get_shear_factor(m, nullptr, nullptr));
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_transform_point(nullptr, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_transform_rect(nullptr, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, SingularMatrixInversion)
{
    // Create a singular matrix (determinant = 0)
    ps_matrix* singular = ps_matrix_create_init(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    ASSERT_NE(singular, nullptr);

    // Attempt to invert singular matrix
    ps_matrix_invert(singular);
    // Note: The implementation doesn't check for singular matrices,
    // but this test documents the behavior
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_matrix_unref(singular);
}

TEST_F(MatrixTest, ExtremeValues)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    // Test with infinity
    ps_matrix_translate(m, INFINITY, INFINITY);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test with NaN
    ps_matrix_translate(m, NAN, NAN);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test with very large values
    ps_matrix_scale(m, 1e10f, 1e10f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test with very small values
    ps_matrix_scale(m, 1e-10f, 1e-10f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, ReferenceCountingEdgeCases)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    // Test multiple refs
    for (int i = 0; i < 100; i++) {
        ps_matrix_ref(m);
    }
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test multiple unrefs
    for (int i = 0; i < 100; i++) {
        ps_matrix_unref(m);
    }
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Final unref to clean up
    ps_matrix_unref(m);
}

TEST_F(MatrixTest, ZeroScaleMatrix)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    // Scale to zero - creates degenerate matrix
    ps_matrix_scale(m, 0.0f, 0.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test determinant of zero-scale matrix
    float det = ps_matrix_get_determinant(m);
    EXPECT_FLOAT_EQ(det, 0.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Try to invert zero-scale matrix
    ps_matrix_invert(m);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, LargeRotationAngles)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    // Test with very large rotation angle
    ps_matrix_rotate(m, 1e6f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test with negative large rotation angle
    ps_matrix_rotate(m, -1e6f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test with rotation beyond 2π
    ps_matrix_rotate(m, 720.0f); // 2 full rotations
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, MatrixMultiplyWithNulls)
{
    ps_matrix* valid = ps_matrix_create();
    ASSERT_NE(valid, nullptr);
    ps_matrix* result = ps_matrix_create();
    ASSERT_NE(result, nullptr);

    // Test various null combinations
    ps_matrix_multiply(nullptr, valid, valid);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_multiply(result, nullptr, valid);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_multiply(result, valid, nullptr);
    EXPECT_EQ(ps_last_status(), STATUS_INVALID_ARGUMENT);

    ps_matrix_unref(valid);
    ps_matrix_unref(result);
}

TEST_F(MatrixTest, TransformWithInvalidGeometry)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    // Transform point with extreme values
    ps_point extreme_point = {INFINITY, NAN};
    ps_matrix_transform_point(m, &extreme_point);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Transform rect with negative dimensions
    ps_rect negative_rect = {-10.0f, -20.0f, -5.0f, -8.0f};
    ps_matrix_transform_rect(m, &negative_rect);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Transform rect with zero dimensions
    ps_rect zero_rect = {0.0f, 0.0f, 0.0f, 0.0f};
    ps_matrix_transform_rect(m, &zero_rect);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_matrix_unref(m);
}

TEST_F(MatrixTest, MatrixEqualityEdgeCases)
{
    ps_matrix* a = ps_matrix_create();
    ps_matrix* b = ps_matrix_create();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);

    // Apply transformations that might cause floating point precision issues
    ps_matrix_rotate(a, 45.0f);
    ps_matrix_rotate(b, 45.0f);

    // Should be equal despite floating point operations
    EXPECT_TRUE(ps_matrix_is_equal(a, b));
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Test with matrices that have very small differences
    ps_matrix_translate(a, 1e-10f, 1e-10f);
    EXPECT_FALSE(ps_matrix_is_equal(a, b));
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    ps_matrix_unref(a);
    ps_matrix_unref(b);
}

TEST_F(MatrixTest, MatrixInitWithInvalidValues)
{
    ps_matrix* m = ps_matrix_create();
    ASSERT_NE(m, nullptr);

    // Initialize with extreme values
    ps_matrix_init(m, INFINITY, NAN, -INFINITY, 1e20f, -1e20f, 0.0f);
    EXPECT_EQ(ps_last_status(), STATUS_SUCCEED);

    // Verify the matrix was set (even with invalid values)
    EXPECT_FALSE(ps_matrix_is_identity(m));

    ps_matrix_unref(m);
}

