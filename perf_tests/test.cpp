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
#if ENABLE_EXTENSIONS
    #include "images/psx_image_plugin.h"
#endif

volatile int tmp;
static int dummy[4096000];
void clear_dcache(void)
{
    int sum = 0;
    for (int i = 0; i < 4096000; i++) {
        dummy[i] = 2;
    }
    for (int i = 0; i < 4096000; i++) {
        sum += dummy[i];
    }

    tmp = sum;
}

const uint8_t tolerance = 5;
static uint8_t* test_buffer = NULL;
static ps_canvas* test_canvas = NULL;

void PS_Init()
{
    printf("picasso initialize\n");
    ASSERT_NE(False, ps_initialize());
#if ENABLE_EXTENSIONS
    ASSERT_NE(0, psx_image_init());
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
