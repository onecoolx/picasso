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

#ifdef WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <sched.h>
    #include <sys/resource.h>
    #include <unistd.h>
#endif

static uint8_t* test_buffer = NULL;
static ps_canvas* test_canvas = NULL;

static void set_process_priority(void);
static void set_cpu_affinity(void);
static size_t get_cache_size(void);

// Improved cache clearing mechanism
static size_t cache_flush_size = 0;
static volatile char* cache_flush_buffer = NULL;

// Get L3 cache size (or total cache size)
static size_t get_cache_size(void)
{
#ifdef WIN32
    // Windows: use GetLogicalProcessorInformation
    DWORD buffer_size = 0;
    GetLogicalProcessorInformation(NULL, &buffer_size);

    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer =
        (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(buffer_size);

    if (buffer && GetLogicalProcessorInformation(buffer, &buffer_size)) {
        DWORD count = buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        size_t cache_size = 0;

        for (DWORD i = 0; i < count; i++) {
            if (buffer[i].Relationship == RelationCache) {
                CACHE_DESCRIPTOR* cache = &buffer[i].Cache;
                // Get L3 cache (or largest cache)
                if (cache->Level == 3 || cache->Level == 2) {
                    if (cache->Size > cache_size) {
                        cache_size = cache->Size;
                    }
                }
            }
        }
        free(buffer);

        if (cache_size > 0) {
            return cache_size;
        }
    }

    // Fallback: assume 8MB cache
    return 8 * 1024 * 1024;
#else
    // Linux: try to read from sysfs
    FILE* fp = fopen("/sys/devices/system/cpu/cpu0/cache/index3/size", "r");
    if (fp) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), fp)) {
            size_t size = 0;
            char unit = 'K';
            if (sscanf(buffer, "%zuK", &size) == 1 ||
                sscanf(buffer, "%zu%c", &size, &unit) == 2) {
                fclose(fp);
                if (unit == 'M' || unit == 'm') {
                    return size * 1024 * 1024;
                } else if (unit == 'K' || unit == 'k') {
                    return size * 1024;
                }
                return size * 1024; // Default to KB
            }
        }
        fclose(fp);
    }

    // Try L2 cache if L3 not available
    fp = fopen("/sys/devices/system/cpu/cpu0/cache/index2/size", "r");
    if (fp) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), fp)) {
            size_t size = 0;
            char unit = 'K';
            if (sscanf(buffer, "%zuK", &size) == 1 ||
                sscanf(buffer, "%zu%c", &size, &unit) == 2) {
                fclose(fp);
                if (unit == 'M' || unit == 'm') {
                    return size * 1024 * 1024;
                } else if (unit == 'K' || unit == 'k') {
                    return size * 1024;
                }
                return size * 1024;
            }
        }
        fclose(fp);
    }

    // Fallback: assume 8MB cache
    return 8 * 1024 * 1024;
#endif
}

void clear_dcache(void)
{
    // Initialize cache flush buffer on first call
    if (cache_flush_buffer == NULL) {
        size_t detected_cache_size = get_cache_size();
        // Use 3x cache size to ensure complete flush
        cache_flush_size = detected_cache_size * 3;

        std::cout << "[Cache Info] Detected cache size: "
                  << (detected_cache_size / 1024) << " KB, "
                  << "using flush buffer: " << (cache_flush_size / 1024 / 1024) << " MB"
                  << std::endl;

        cache_flush_buffer = (volatile char*)malloc(cache_flush_size);
        if (!cache_flush_buffer) {
            std::cerr << "[Cache Warning] Failed to allocate cache flush buffer, "
                      << "using smaller size" << std::endl;
            cache_flush_size = 16 * 1024 * 1024; // Fallback to 16MB
            cache_flush_buffer = (volatile char*)malloc(cache_flush_size);
        }
    }

    if (cache_flush_buffer) {
        // Write to buffer to flush cache
        for (size_t i = 0; i < cache_flush_size; i += 64) {
            cache_flush_buffer[i] = (char)(i & 0xFF);
        }

        // Read from buffer to ensure writes complete
        volatile char sum = 0;
        for (size_t i = 0; i < cache_flush_size; i += 64) {
            sum += cache_flush_buffer[i];
        }

        // Memory barrier to prevent compiler optimization
#if defined(__GNUC__) || defined(__clang__)
        __asm__ __volatile__("" ::: "memory");
#elif defined(_MSC_VER)
        _ReadWriteBarrier();
#endif

        // Prevent optimization of sum
        if (sum == 0) {
            // This branch will never be taken, but prevents optimization
            cache_flush_buffer[0] = 1;
        }
    }
}

static void set_process_priority(void)
{
#ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#else
    setpriority(PRIO_PROCESS, 0, -20);
#endif
}

static void set_cpu_affinity(void)
{
#ifdef WIN32
    DWORD_PTR mask = 1;
    SetProcessAffinityMask(GetCurrentProcess(), mask);
    SetThreadAffinityMask(GetCurrentThread(), mask);
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
#endif
}

ps_canvas* get_test_canvas(void)
{
    return test_canvas;
}

void PS_Init()
{
    printf("picasso initialize\n");
    ASSERT_NE(False, ps_initialize());

    set_process_priority();
    set_cpu_affinity();

    if (!test_buffer) {
        test_buffer = (uint8_t*)calloc(TEST_WIDTH * 4, TEST_HEIGHT);
        test_canvas = ps_canvas_create_with_data(test_buffer, COLOR_FORMAT_RGBA, 100, 100, 100 * 4);
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

    // Free cache flush buffer
    if (cache_flush_buffer) {
        free((void*)cache_flush_buffer);
        cache_flush_buffer = NULL;
        cache_flush_size = 0;
    }

    printf("picasso shutdown\n");
    ps_shutdown();
}

bool PerformanceTest::LoadBaseline(const std::string& file_name, BenchmarkData& data)
{
    data.clear();

    std::ifstream file(file_name);
    if (!file.is_open()) {
        return false;
    }

    std::string json_str((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    if (json_str.empty()) {
        std::cerr << "Error: JSON file is empty" << std::endl;
        return false;
    }

    cJSON* root = cJSON_Parse(json_str.c_str());
    if (!root) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            std::cerr << "Error: JSON parse error before: " << error_ptr << std::endl;
        }
        std::cerr << "Error: Failed to parse JSON from file " << baseline_file << std::endl;
        return false;
    }

    if (cJSON_IsObject(root) == 0) {
        std::cerr << "Error: Root JSON element is not an object" << std::endl;
        cJSON_Delete(root);
        return false;
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

        cJSON* mid_obj = cJSON_GetObjectItemCaseSensitive(current, "mid_ms");
        if (cJSON_IsNumber(mid_obj)) {
            result.mid_ms = mid_obj->valuedouble;
        } else {
            std::cerr << "Warning: Missing or invalid 'mid_ms' for key '" << key << "', skipping" << std::endl;
            valid = false;
        }

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

        // Optional fields (for backward compatibility)
        cJSON* iterations_obj = cJSON_GetObjectItemCaseSensitive(current, "iterations");
        if (cJSON_IsNumber(iterations_obj)) {
            result.iterations = iterations_obj->valueint;
        } else {
            result.iterations = 200; // Default value for old baseline data
        }

        cJSON* total_time_obj = cJSON_GetObjectItemCaseSensitive(current, "total_time_ms");
        if (cJSON_IsNumber(total_time_obj)) {
            result.total_time_ms = total_time_obj->valuedouble;
        } else {
            result.total_time_ms = 0.0; // Unknown for old data
        }

        cJSON* std_dev_obj = cJSON_GetObjectItemCaseSensitive(current, "std_dev");
        if (cJSON_IsNumber(std_dev_obj)) {
            result.std_dev = std_dev_obj->valuedouble;
        } else {
            // Estimate std_dev from range (assuming normal distribution)
            result.std_dev = (result.max_ms - result.min_ms) / 6.0;
        }

        cJSON* cv_obj = cJSON_GetObjectItemCaseSensitive(current, "cv");
        if (cJSON_IsNumber(cv_obj)) {
            result.cv = cv_obj->valuedouble;
        } else {
            result.cv = result.std_dev / result.avg_ms;
        }

        if (valid) {
            data[key] = result;
        }
    }

    cJSON_Delete(root);
    return true;
}

double truncat8(double value)
{
    double multiplier = 100000000.0; // 10^8
    return trunc(value * multiplier) / multiplier;
}

void PerformanceTest::SaveBaseline(const std::string& file_name)
{
    cJSON* root = cJSON_CreateObject();
    if (!root) {
        std::cerr << "Error: Failed to create root JSON object" << std::endl;
        return;
    }

    for (const auto& pair : newbase_data) {
        const std::string& key = pair.first;
        const BenchmarkResult& result = pair.second;

        cJSON* result_obj = cJSON_CreateObject();
        if (!result_obj) {
            std::cerr << "Error: Failed to create result object for " << key << std::endl;
            cJSON_Delete(root);
            return;
        }

        cJSON_AddNumberToObject(result_obj, "mid_ms", truncat8(result.mid_ms));
        cJSON_AddNumberToObject(result_obj, "avg_ms", truncat8(result.avg_ms));
        cJSON_AddNumberToObject(result_obj, "min_ms", truncat8(result.min_ms));
        cJSON_AddNumberToObject(result_obj, "max_ms", truncat8(result.max_ms));
        cJSON_AddNumberToObject(result_obj, "std_dev", truncat8(result.std_dev));
        cJSON_AddNumberToObject(result_obj, "cv", truncat8(result.cv));
        cJSON_AddNumberToObject(result_obj, "iterations", result.iterations);
        cJSON_AddNumberToObject(result_obj, "total_time_ms", truncat8(result.total_time_ms));

        cJSON_AddItemToObject(root, key.c_str(), result_obj);
    }

    char* json_str = cJSON_Print(root);
    if (!json_str) {
        std::cerr << "Error: Failed to serialize JSON" << std::endl;
        cJSON_Delete(root);
        return;
    }

    std::ofstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file " << file_name << " for writing" << std::endl;
        free(json_str);
        cJSON_Delete(root);
        return;
    }

    file << json_str;
    file.close();

    free(json_str);
    cJSON_Delete(root);
}
