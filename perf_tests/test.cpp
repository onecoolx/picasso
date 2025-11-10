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

#include <iostream>
#include <string>
#include <map>
#include <fstream>

#include "cJSON.h"

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

void PerformanceTest::LoadBaseline()
{
    baseline_data.clear();

    std::ifstream file(baseline_file);
    if (!file.is_open()) {
        return;
    }

    std::string json_str((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    if (json_str.empty()) {
        std::cerr << "Error: JSON file is empty" << std::endl;
        return;
    }

    cJSON* root = cJSON_Parse(json_str.c_str());
    if (!root) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            std::cerr << "Error: JSON parse error before: " << error_ptr << std::endl;
        }
        std::cerr << "Error: Failed to parse JSON from file " << baseline_file << std::endl;
        return;
    }

    if (cJSON_IsObject(root) == 0) {
        std::cerr << "Error: Root JSON element is not an object" << std::endl;
        cJSON_Delete(root);
        return;
    }

    cJSON* current = NULL;
    cJSON_ArrayForEach(current, root) {
        if (current->string == NULL) {
            continue;
        }

        std::string key = current->string;

        if (cJSON_IsObject(current) == 0) {
            std::cerr << "Warning: Value for key '" << key << "' is not an object, skipping" << std::endl;
            continue;
        }

        BenchmarkResult result;
        bool valid = true;

        cJSON* avg_obj = cJSON_GetObjectItemCaseSensitive(current, "avg_ms");
        if (cJSON_IsNumber(avg_obj)) {
            result.avg_ms = avg_obj->valuedouble;
        } else {
            std::cerr << "Warning: Missing or invalid 'avg_ms' for key '" << key << "', skipping" << std::endl;
            valid = false;
        }

        cJSON* min_obj = cJSON_GetObjectItemCaseSensitive(current, "min_ms");
        if (valid && cJSON_IsNumber(min_obj)) {
            result.min_ms = min_obj->valuedouble;
        } else if (valid) {
            std::cerr << "Warning: Missing or invalid 'min_ms' for key '" << key << "', skipping" << std::endl;
            valid = false;
        }

        cJSON* max_obj = cJSON_GetObjectItemCaseSensitive(current, "max_ms");
        if (valid && cJSON_IsNumber(max_obj)) {
            result.max_ms = max_obj->valuedouble;
        } else if (valid) {
            std::cerr << "Warning: Missing or invalid 'max_ms' for key '" << key << "', skipping" << std::endl;
            valid = false;
        }

        if (valid) {
            baseline_data[key] = result;
        }
    }

    cJSON_Delete(root);
    std::cout << "Successfully read benchmark data from " << baseline_file << std::endl;
}

void PerformanceTest::SaveBaseline()
{
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        std::cerr << "Error: Failed to create root JSON object" << std::endl;
        return;
    }

    for (const auto& pair : baseline_data) {
        const std::string& key = pair.first;
        const BenchmarkResult& result = pair.second;

        cJSON* result_obj = cJSON_CreateObject();
        if (!result_obj) {
            std::cerr << "Error: Failed to create result object for " << key << std::endl;
            cJSON_Delete(root);
            return;
        }

        cJSON_AddNumberToObject(result_obj, "avg_ms", result.avg_ms);
        cJSON_AddNumberToObject(result_obj, "min_ms", result.min_ms);
        cJSON_AddNumberToObject(result_obj, "max_ms", result.max_ms);

        cJSON_AddItemToObject(root, key.c_str(), result_obj);
    }

    char* json_str = cJSON_Print(root);
    if (!json_str) {
        std::cerr << "Error: Failed to serialize JSON" << std::endl;
        cJSON_Delete(root);
        return;
    }

    std::ofstream file(baseline_file);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << baseline_file << " for writing" << std::endl;
        free(json_str);
        cJSON_Delete(root);
        return;
    }

    file << json_str;
    file.close();

    free(json_str);
    cJSON_Delete(root);

    std::cout << "Successfully wrote benchmark data to " << baseline_file << std::endl;
}
