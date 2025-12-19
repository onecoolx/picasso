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

#define SNAPSHOT_PATH "draw"

#include "test.h"

#include "picasso_objects.h"

class ImageTest : public ::testing::Test
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
        clear_test_canvas();
    }

    void TearDown() override
    {
    }

    void create_test_data(uint8_t** data, int32_t width, int32_t height, int32_t pitch)
    {
        *data = (uint8_t*)malloc(height * pitch);
        for (int32_t y = 0; y < height; y++) {
            for (int32_t x = 0; x < width; x++) {
                int32_t idx = y * pitch + x * 4;
                (*data)[idx] = (x * 255) / width; // R
                (*data)[idx + 1] = (y * 255) / height; // G
                (*data)[idx + 2] = 128; // B
                (*data)[idx + 3] = 255; // A
            }
        }
    }
};

TEST_F(ImageTest, CreateValidImage)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_NE(nullptr, img);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_size size;
    ASSERT_TRUE(ps_image_get_size(img, &size));
    EXPECT_FLOAT_EQ(100.0f, size.w);
    EXPECT_FLOAT_EQ(100.0f, size.h);
    EXPECT_EQ(COLOR_FORMAT_RGBA, ps_image_get_format(img));

    ps_image_unref(img);
}

TEST_F(ImageTest, CreateImageInvalidParams)
{
    // Invalid dimensions
    ps_image* img1 = ps_image_create(COLOR_FORMAT_RGBA, 0, 100);
    EXPECT_EQ(nullptr, img1);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_image* img2 = ps_image_create(COLOR_FORMAT_RGBA, 100, -1);
    EXPECT_EQ(nullptr, img2);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_image* img3 = ps_image_create(COLOR_FORMAT_RGBA, -50, -50);
    EXPECT_EQ(nullptr, img3);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ImageTest, CreateFromDataValid)
{
    uint8_t* data;
    create_test_data(&data, 50, 50, 200);

    ps_image* img = ps_image_create_from_data(data, COLOR_FORMAT_RGBA, 50, 50, 200);
    ASSERT_NE(nullptr, img);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_size size;
    EXPECT_TRUE(ps_image_get_size(img, &size));
    EXPECT_FLOAT_EQ(50.0f, size.w);
    EXPECT_FLOAT_EQ(50.0f, size.h);

    ps_image_unref(img);
    free(data);
}

TEST_F(ImageTest, CreateFromDataInvalidParams)
{
    uint8_t data[100] = {0};

    // Null data
    ps_image* img1 = ps_image_create_from_data(nullptr, COLOR_FORMAT_RGBA, 10, 10, 40);
    EXPECT_EQ(nullptr, img1);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    // Invalid dimensions
    ps_image* img2 = ps_image_create_from_data(data, COLOR_FORMAT_RGBA, 0, 10, 40);
    EXPECT_EQ(nullptr, img2);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_image* img3 = ps_image_create_from_data(data, COLOR_FORMAT_RGBA, 10, -1, 40);
    EXPECT_EQ(nullptr, img3);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    // Invalid pitch
    ps_image* img4 = ps_image_create_from_data(data, COLOR_FORMAT_RGBA, 10, 10, 0);
    EXPECT_EQ(nullptr, img4);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ImageTest, CreateCompatible)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGB565, 200, 150);
    ASSERT_NE(nullptr, canvas);

    ps_image* img = ps_image_create_compatible(canvas, 100, 80);
    ASSERT_NE(nullptr, img);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    EXPECT_EQ(COLOR_FORMAT_RGB565, ps_image_get_format(img));

    ps_size size;
    EXPECT_TRUE(ps_image_get_size(img, &size));
    EXPECT_FLOAT_EQ(100.0f, size.w);
    EXPECT_FLOAT_EQ(80.0f, size.h);

    ps_image_unref(img);
    ps_canvas_unref(canvas);
}

TEST_F(ImageTest, CreateCompatibleInvalidCanvas)
{
    ps_image* img = ps_image_create_compatible(nullptr, 100, 100);
    EXPECT_EQ(nullptr, img);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ImageTest, CreateFromImage)
{
    ps_image* src = ps_image_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_NE(nullptr, src);

    // Test sub-image creation
    ps_rect rect = {10.0f, 20.0f, 50.0f, 60.0f};
    ps_image* sub = ps_image_create_from_image(src, &rect);
    ASSERT_NE(nullptr, sub);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_size size;
    EXPECT_TRUE(ps_image_get_size(sub, &size));
    EXPECT_FLOAT_EQ(50.0f, size.w);
    EXPECT_FLOAT_EQ(60.0f, size.h);

    // Test null rect (should reference)
    ps_image* ref = ps_image_create_from_image(src, nullptr);
    ASSERT_NE(nullptr, ref);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());
    EXPECT_EQ(src, ref); // Should be same object

    ps_image_unref(sub);
    ps_image_unref(ref);
    ps_image_unref(src);
}

TEST_F(ImageTest, CreateFromImageInvalid)
{
    ps_image* img = ps_image_create_from_image(nullptr, nullptr);
    EXPECT_EQ(nullptr, img);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ImageTest, CreateFromCanvas)
{
    ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, 200, 150);
    ASSERT_NE(nullptr, canvas);

    // Fill canvas with test pattern
    ps_context* ctx = ps_context_create(canvas, nullptr);
    ps_color color = {1.0f, 0.0f, 0.0f, 1.0f};
    ps_set_source_color(ctx, &color);
    ps_paint(ctx);
    ps_context_unref(ctx);

    // Create image from canvas
    ps_rect rect = {50.0f, 30.0f, 100.0f, 90.0f};
    ps_image* img = ps_image_create_from_canvas(canvas, &rect);
    ASSERT_NE(nullptr, img);
    EXPECT_EQ(STATUS_SUCCEED, ps_last_status());

    ps_size size;
    EXPECT_TRUE(ps_image_get_size(img, &size));
    EXPECT_FLOAT_EQ(100.0f, size.w);
    EXPECT_FLOAT_EQ(90.0f, size.h);

    ps_image_unref(img);
    ps_canvas_unref(canvas);
}

TEST_F(ImageTest, CreateFromCanvasInvalid)
{
    ps_image* img = ps_image_create_from_canvas(nullptr, nullptr);
    EXPECT_EQ(nullptr, img);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ImageTest, ReferenceCounting)
{
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 50, 50);
    ASSERT_NE(nullptr, img);

    // Initial refcount should be 1
    EXPECT_EQ(1, img->refcount);

    // Ref should increment
    ps_image* ref = ps_image_ref(img);
    EXPECT_EQ(img, ref);
    EXPECT_EQ(2, img->refcount);

    // Unref should decrement
    ps_image_unref(img);
    EXPECT_EQ(1, img->refcount);

    // Final unref should cleanup
    ps_image_unref(img);
}

TEST_F(ImageTest, ReferenceCountingInvalid)
{
    ps_image* ref = ps_image_ref(nullptr);
    EXPECT_EQ(nullptr, ref);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_image_unref(nullptr); // Should not crash
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ImageTest, GetSizeInvalid)
{
    ps_size size;
    ps_bool result = ps_image_get_size(nullptr, &size);
    EXPECT_FALSE(result);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 10, 10);
    result = ps_image_get_size(img, nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());

    ps_image_unref(img);
}

TEST_F(ImageTest, GetFormatInvalid)
{
    ps_color_format fmt = ps_image_get_format(nullptr);
    EXPECT_EQ(COLOR_FORMAT_UNKNOWN, fmt);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
}

TEST_F(ImageTest, CustomDrawTest)
{
    // Create test image with gradient
    ps_image* img = ps_image_create(COLOR_FORMAT_RGBA, 100, 100);
    ASSERT_NE(nullptr, img);

    // Create context to draw on image
    ps_canvas* canvas = ps_canvas_create_from_image(img, nullptr);
    ps_context* ctx = ps_context_create(canvas, nullptr);

    // Draw gradient
    for (int i = 0; i < 10; i++) {
        ps_color color = {i / 10.0f, 0.5f, 1.0f - i / 10.0f, 1.0f};
        ps_set_source_color(ctx, &color);
        ps_rect rc = {i * 10.0f, 0.0f, 10.0f, 100.0f};
        ps_rectangle(ctx, &rc);
        ps_fill(ctx);
    }

    // Draw image to test canvas
    ps_context* test_ctx = ps_context_create(get_test_canvas(), nullptr);
    ps_set_source_image(test_ctx, img);
    ps_rect rc = {50.0f, 50.0f, 100.0f, 100.0f};
    ps_rectangle(test_ctx, &rc);
    ps_fill(test_ctx);

    EXPECT_SNAPSHOT_EQ(image_custom_draw);

    ps_context_unref(test_ctx);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    ps_image_unref(img);
}

TEST_F(ImageTest, BadCaseRenderingTest)
{
    // Test with invalid image in rendering
    ps_context* ctx = ps_context_create(get_test_canvas(), nullptr);

    // Drawing null image should not crash
    ps_set_source_image(ctx, nullptr);
    EXPECT_EQ(STATUS_INVALID_ARGUMENT, ps_last_status());
    ps_fill(ctx);

    // Test with zero-sized image
    ps_image* zero_img = ps_image_create(COLOR_FORMAT_RGBA, 0, 0);
    EXPECT_EQ(nullptr, zero_img);

    ps_context_unref(ctx);
}

TEST_F(ImageTest, MultipleFormatsTest)
{
    ps_color_format formats[] = {
        COLOR_FORMAT_RGBA,
        COLOR_FORMAT_ARGB,
        COLOR_FORMAT_ABGR,
        COLOR_FORMAT_BGRA,
        COLOR_FORMAT_RGB,
        COLOR_FORMAT_BGR,
        COLOR_FORMAT_RGB565,
        COLOR_FORMAT_RGB555
    };

    for (size_t i = 0; i < sizeof(formats) / sizeof(ps_color_format); i++) {
        ps_image* img = ps_image_create(formats[i], 50, 50);
        ASSERT_NE(nullptr, img) << "Failed to create image with format " << i;
        EXPECT_EQ(formats[i], ps_image_get_format(img));

        ps_size size;
        EXPECT_TRUE(ps_image_get_size(img, &size));
        EXPECT_FLOAT_EQ(50.0f, size.w);
        EXPECT_FLOAT_EQ(50.0f, size.h);

        ps_image_unref(img);
    }
}
