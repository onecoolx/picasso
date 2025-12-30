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

#include "images/psx_image.h"
#include "images/psx_image_plugin.h"

class PsxImagTest : public ::testing::Test
{
protected:
    static psx_image_operator test_operator;

    static void SetUpTestSuite()
    {
        PS_Init();
        psx_result ret = psx_image_init();
        ASSERT_EQ(S_OK, ret);
    }

    static void TearDownTestSuite()
    {
        PS_Shutdown();
        psx_image_shutdown();
    }

    static int32_t test_read_header(const ps_byte* data, size_t len, psx_image_header* header)
    {
        header->width = 100;
        header->height = 100;
        header->pitch = 400;
        header->bpp = 4;
        header->format = 0;
        header->alpha = 1;
        header->frames = 1;
        return 0;
    }

    static int32_t test_decode_data(psx_image_header* header, const psx_image* image,
                                    psx_image_frame* frame, int32_t idx, ps_byte* buffer, size_t buffer_len)
    {
        return 0;
    }

    static int32_t test_release_header(psx_image_header* header)
    {
        return 0;
    }
};

psx_image_operator PsxImagTest::test_operator = {
    test_read_header,
    test_decode_data,
    test_release_header,
    nullptr, nullptr, nullptr
};

TEST_F(PsxImagTest, LoadFromMemoryNoData)
{
    const ps_byte png_data[] = "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A";
    psx_image* img = psx_image_load_from_memory(png_data, sizeof(png_data), NULL);
    EXPECT_EQ(nullptr, img);
}

TEST_F(PsxImagTest, LoadFromMemoryInvalid)
{
    psx_image* img = psx_image_load_from_memory(NULL, 0, NULL);
    EXPECT_EQ(nullptr, img);

    ps_byte invalid_data[] = {0x00, 0x01, 0x02};
    img = psx_image_load_from_memory(invalid_data, sizeof(invalid_data), NULL);
    EXPECT_EQ(nullptr, img);
}

TEST_F(PsxImagTest, LoadFromFile)
{
    psx_image* img = psx_image_load("test.png", NULL);
    EXPECT_NE(nullptr, img);
    EXPECT_GT(img->width, 0);
    EXPECT_GT(img->height, 0);
    psx_image_destroy(img);
}

TEST_F(PsxImagTest, SaveToFile)
{
    psx_image* img = psx_image_load("test.png", NULL);
    ASSERT_NE(nullptr, img);

    psx_result result = psx_image_save_to_file(img, "output.png", "png", 90);
    EXPECT_EQ(S_NOT_SUPPORT, result);

    psx_image_destroy(img);
}

TEST_F(PsxImagTest, ImageFrameAccess)
{
    psx_image* img = psx_image_load("test.png", NULL);
    ASSERT_NE(nullptr, img);

    ps_image* first_img = IMG_OBJ(img);
    EXPECT_NE(nullptr, first_img);

    ps_byte* first_data = IMG_DATA(img);
    EXPECT_NE(nullptr, first_data);

    psx_image_destroy(img);
}

// Module Management Tests
TEST_F(PsxImagTest, RegisterUnregisterOperator)
{
    psx_result result = psx_image_register_operator(
                            "test", (ps_byte*)"TEST", 0, 4, PRIORITY_DEFAULT, &test_operator);
    EXPECT_EQ(0, result);

    result = psx_image_unregister_operator(&test_operator);
    EXPECT_EQ(0, result);
}

TEST_F(PsxImagTest, RegisterInvalidParams)
{
    psx_result result = psx_image_register_operator(
                            "test", (ps_byte*)"TEST", 0, 4, PRIORITY_DEFAULT, nullptr);
    EXPECT_NE(0, result);

    result = psx_image_register_operator(
                 nullptr, (ps_byte*)"TEST", 0, 4, PRIORITY_DEFAULT, &test_operator);
    EXPECT_NE(0, result);
}

TEST_F(PsxImagTest, PriorityLevels)
{
    psx_image_operator op1 = test_operator;
    psx_image_operator op2 = test_operator;
    psx_image_operator op3 = test_operator;

    psx_image_register_operator("low", (ps_byte*)"LOW", 0, 3, PRIORITY_EXTENTED, &op1);
    psx_image_register_operator("default", (ps_byte*)"DEF", 0, 3, PRIORITY_DEFAULT, &op2);
    psx_image_register_operator("master", (ps_byte*)"MAS", 0, 3, PRIORITY_MASTER, &op3);

    psx_image_unregister_operator(&op1);
    psx_image_unregister_operator(&op2);
    psx_image_unregister_operator(&op3);
}
