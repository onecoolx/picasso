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


TEST_F(PsxImagTest, LoadFromFile) {
    psx_image* img = psx_image_load("test.png", NULL);
    EXPECT_NE(nullptr, img);
    EXPECT_GT(img->width, 0);
    EXPECT_GT(img->height, 0);
    psx_image_destroy(img);
}
