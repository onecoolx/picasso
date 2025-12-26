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
#include "lodepng.h"
#if ENABLE_EXTENSIONS
    #include "images/psx_image_plugin.h"
#endif

const uint8_t tolerance = 5; // pixels compare tolerance
static uint8_t* test_buffer = NULL;
static ps_canvas* test_canvas = NULL;

#if ENABLE_EXTENSIONS
    static void init_png_decoder(void);
    static void deinit_png_decoder(void);
#endif

void PS_Init()
{
    printf("picasso initialize\n");
    ASSERT_NE(False, ps_initialize());
#if ENABLE_EXTENSIONS
    ASSERT_NE(0, psx_image_init());
    init_png_decoder();
#endif
    int v = ps_version();
    fprintf(stderr, "picasso version %d \n", v);
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());

    if (!test_buffer) {
        test_buffer = (uint8_t*)calloc(TEST_WIDTH * 4, TEST_HEIGHT);
        test_canvas = ps_canvas_create_with_data(test_buffer, COLOR_FORMAT_RGBA, TEST_WIDTH, TEST_HEIGHT, TEST_WIDTH * 4);
    }
}

void PS_Shutdown()
{
    if (test_buffer) {
        ps_canvas_unref(test_canvas);
        test_canvas = NULL;
        free(test_buffer);
        test_buffer = NULL;
    }

    printf("picasso shutdown\n");
#if ENABLE_EXTENSIONS
    deinit_png_decoder();
    psx_image_shutdown();
#endif
    ps_shutdown();
    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}

ps_canvas* get_test_canvas(void)
{
    return test_canvas;
}

void clear_test_canvas(void)
{
    memset(test_buffer, 0xFF, TEST_WIDTH * TEST_HEIGHT * 4);
}

static bool _file_exists(const char* path)
{
    struct stat fileInfo;
    return !stat(path, &fileInfo);
}

static unsigned int _load_image(const char* actual_file, std::vector<uint8_t>& image, unsigned int& width, unsigned int& height)
{
    return lodepng::decode(image, width, height, actual_file);
}

// image compare
static ::testing::AssertionResult _compare_images(
    const uint8_t* expected_image, unsigned int expected_width, unsigned int expected_height,
    const uint8_t* actual_image, unsigned int actual_width, unsigned int actual_height)
{
    if (expected_width != actual_width || expected_height != actual_height) {
        return ::testing::AssertionFailure()
               << "Image size mismatch!\n"
               << "Expected: " << expected_width << "x" << expected_height << "\n"
               << "Actual:   " << actual_width << "x" << actual_height;
    }

    const size_t pixel_count = expected_width * expected_height;
    for (size_t i = 0; i < pixel_count; ++i) {
        const size_t idx = i * 4;

        if (std::abs(expected_image[idx] - actual_image[idx]) > tolerance || // R
            std::abs(expected_image[idx + 1] - actual_image[idx + 1]) > tolerance || // G
            std::abs(expected_image[idx + 2] - actual_image[idx + 2]) > tolerance || // B
            std::abs(expected_image[idx + 3] - actual_image[idx + 3]) > tolerance) { // A

            const size_t x = i % expected_width;
            const size_t y = i / expected_width;

            return ::testing::AssertionFailure()
                   << "Pixel mismatch at (" << x << ", " << y << ")\n"
                   << "Expected: RGBA("
                   << static_cast<int>(expected_image[idx]) << ", "
                   << static_cast<int>(expected_image[idx + 1]) << ", "
                   << static_cast<int>(expected_image[idx + 2]) << ", "
                   << static_cast<int>(expected_image[idx + 3]) << ")\n"
                   << "Actual:   RGBA("
                   << static_cast<int>(actual_image[idx]) << ", "
                   << static_cast<int>(actual_image[idx + 1]) << ", "
                   << static_cast<int>(actual_image[idx + 2]) << ", "
                   << static_cast<int>(actual_image[idx + 3]) << ")";
        }
    }

    return ::testing::AssertionSuccess();
}

static ::testing::AssertionResult _save_image(
    const uint8_t* image_buf, unsigned int width, unsigned int height,
    const char* save_file)
{
    unsigned error = lodepng_encode32_file(save_file, image_buf, width, height);
    if (error) {
        return ::testing::AssertionFailure()
               << "Failed to save actual image: " << lodepng_error_text(error);
    }
    return ::testing::AssertionSuccess();
}

static void _generate_diff_image(std::vector<uint8_t>& diffImage, const uint8_t* image1, unsigned int width1, unsigned int height1,
                                 const uint8_t* image2, unsigned int width2, unsigned int height2)
{
    unsigned int width = width1;
    unsigned int height = height1;
    const size_t pixelCount = width * height;

    diffImage.resize(pixelCount * 4);

    for (size_t i = 0; i < pixelCount; ++i) {
        const size_t idx = i * 4;
        uint8_t r1 = image1[idx];
        uint8_t g1 = image1[idx + 1];
        uint8_t b1 = image1[idx + 2];
        uint8_t a1 = image1[idx + 3];

        uint8_t r2 = image2[idx];
        uint8_t g2 = image2[idx + 1];
        uint8_t b2 = image2[idx + 2];
        uint8_t a2 = image2[idx + 3];

        bool diff = (std::abs(r1 - r2) > tolerance) ||
                    (std::abs(g1 - g2) > tolerance) ||
                    (std::abs(b1 - b2) > tolerance) ||
                    (std::abs(a1 - a2) > tolerance);

        if (diff) {
            diffImage[idx] = 255; // R
            diffImage[idx + 1] = 0; // G
            diffImage[idx + 2] = 0; // B
            diffImage[idx + 3] = 255; // A
        } else {
            diffImage[idx] = r1; // R
            diffImage[idx + 1] = g1; // G
            diffImage[idx + 2] = b1; // B
            diffImage[idx + 3] = (uint8_t)(a1 * 0.5f); // A
        }
    }
}

::testing::AssertionResult CompareToImage(const char* actual_file)
{
    if (!_file_exists(actual_file)) {
        return _save_image(test_buffer, TEST_WIDTH, TEST_HEIGHT, actual_file);
    }

    std::vector<uint8_t> actual_image;
    unsigned int actual_width, actual_height;
    unsigned int error = _load_image(actual_file, actual_image, actual_width, actual_height);
    if (error) {
        return ::testing::AssertionFailure()
               << "Failed to load actual image: "
               << lodepng_error_text(error);
    }

    ::testing::AssertionResult result = _compare_images(test_buffer, TEST_WIDTH, TEST_HEIGHT,
                                                        actual_image.data(), actual_width, actual_height);
    if (!result) {
        std::string file_name = actual_file;
        std::string pure_name = file_name.substr(0, file_name.rfind("."));

        std::string err_file = pure_name + "_failed.png";
        _save_image(test_buffer, TEST_WIDTH, TEST_HEIGHT, err_file.c_str());

        std::vector<uint8_t> diff_image;
        std::string diff_file = pure_name + "_diff.png";
        _generate_diff_image(diff_image, test_buffer, TEST_WIDTH, TEST_HEIGHT,
                             actual_image.data(), actual_width, actual_height);
        _save_image(diff_image.data(), TEST_WIDTH, TEST_HEIGHT, diff_file.c_str());
    }
    return result;
}

#if ENABLE_EXTENSIONS
struct lode_png_image_ctx {
    std::vector<uint8_t> image;
    unsigned int width;
    unsigned int height;
};

static int lode_read_png_info(const ps_byte* data, size_t len, psx_image_header* header)
{
    struct lode_png_image_ctx* ctx = (struct lode_png_image_ctx*)calloc(1, sizeof(struct lode_png_image_ctx));
    if (!ctx) {
        return -1; // out of memory.
    }

    new ((void*) & (ctx->image)) std::vector<uint8_t>();
    lodepng::decode(ctx->image, ctx->width, ctx->height, data, len);

    header->priv = ctx;
    header->width = ctx->width;
    header->height = ctx->height;
    header->pitch = ctx->width * 4;
    header->depth = 32;
    header->bpp = 4;
    header->format = 0;
    header->alpha = 1;
    header->frames = 1;
    return 0;
}

static int lode_release_read_png_info(psx_image_header* header)
{
    struct lode_png_image_ctx* ctx = (struct lode_png_image_ctx*)header->priv;

    (&ctx->image)->vector<uint8_t>::~vector<uint8_t>();
    free(ctx);
    return 0;
}

static int lode_decode_png_data(psx_image_header* header, const psx_image* image, psx_image_frame* frame, int idx, ps_byte* buffer, size_t buffer_len)
{
    struct lode_png_image_ctx* ctx = (struct lode_png_image_ctx*)header->priv;
    memcpy(buffer, ctx->image.data(), ctx->width * ctx->height * 4);
    return 0;
}

static psx_image_operator png_coder;

static void init_png_decoder(void)
{
    png_coder.read_header_info = lode_read_png_info;
    png_coder.decode_image_data = lode_decode_png_data;
    png_coder.release_read_header_info = lode_release_read_png_info;

    psx_image_register_operator(
        "png", (ps_byte*)"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 0, 8, PRIORITY_DEFAULT, &png_coder);
}

static void deinit_png_decoder(void)
{
    psx_image_unregister_operator(&png_coder);
}
#endif
