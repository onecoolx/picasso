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

class CanvasTest : public ::testing::Test
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
    }

    void TearDown() override
    {
    }

};

TEST_F(CanvasTest, CanvasWithDataAndCompatible)
{
    uint8_t buf[200] = {0};
    ps_canvas* newCanvas = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGB565, 10, 10, 20);
    ps_canvas* newCanvas2 = ps_canvas_create_compatible(newCanvas, 20, 20);

    ASSERT_EQ(ps_canvas_get_format(newCanvas), ps_canvas_get_format(newCanvas2));

    ps_size s;
    ps_bool r = ps_canvas_get_size(newCanvas2, &s);
    ASSERT_FLOAT_EQ(s.w, 20.0f);
    ASSERT_FLOAT_EQ(s.h, 20.0f);
    ASSERT_EQ(r, True);

    ps_canvas_unref(newCanvas);
    ps_canvas_unref(newCanvas2);

    ASSERT_EQ(STATUS_SUCCEED, ps_last_status());
}
