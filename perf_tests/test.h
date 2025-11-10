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

#ifndef _PICASSO_PERF_TEST_H_
#define _PICASSO_PERF_TEST_H_

#include "picasso.h"
#include "images/psx_image.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <map>
#include <numeric>

#include "timeuse.h"

#define TEST_WIDTH 640
#define TEST_HEIGHT 480

void PS_Init();
void PS_Shutdown();
void clear_dcache(void);
ps_canvas* get_test_canvas(void);
void clear_test_canvas(void);

#if defined(WIN32)
    #define SYSTEM "win32"
#elif defined(__APPLE__)
    #define SYSTEM "apple"
#else
    #define SYSTEM "linux"
#endif

#if defined(__i386__) \
    || defined(i386)     \
    || defined(_M_IX86)  \
    || defined(_X86_)    \
    || defined(__THW_INTEL) \
    || defined(__x86_64__) \
    || defined(_M_X64)
    #define ARCH "x86"
#elif defined(arm) \
    || defined(__arm__) \
    || defined(ARM) \
    || defined(_ARM_)
    #define ARCH "arm"
#else
    #define ARCH "unknown"
#endif

class PerformanceTest : public ::testing::Test
{
protected:
    static constexpr int TEST_ITERATIONS = 100;
    static constexpr double TOLERANCE_PERCENT = 5.0; // 10% tolerance

    struct BenchmarkResult {
        double avg_ms;
        double min_ms;
        double max_ms;
    };

    std::map<std::string, BenchmarkResult> baseline_data;
    std::string baseline_file;

    static void SetUpTestSuite()
    {
        PS_Init();
    }

    static void TearDownTestSuite()
    {
        PS_Shutdown();
    }

    void Init(const std::string& test_name)
    {
        baseline_file = "./benchmark/" + test_name + "_" + SYSTEM + "_" + ARCH + ".json";
    }

    void SetUp() override
    {
        LoadBaseline();
    }

    void TearDown() override
    {
        SaveBaseline();
    }

    void LoadBaseline();
    void SaveBaseline();

    template<typename Func>
    BenchmarkResult RunBenchmark(const std::string& test_name, Func&& func)
    {
        std::vector<double> times;
        times.reserve(TEST_ITERATIONS);

        // clean cache
        clear_dcache();
        for (int i = 0; i < TEST_ITERATIONS; ++i) {
            clocktime_t start = get_clock();
            func();
            clocktime_t end = get_clock();
            times.push_back(get_clock_used_ms(start, end));
        }

        BenchmarkResult result;
        result.avg_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        result.min_ms = *std::min_element(times.begin(), times.end());
        result.max_ms = *std::max_element(times.begin(), times.end());

        return result;
    }

    void CompareToBenchmark(const std::string& test_name, const BenchmarkResult& result)
    {
        std::string key = test_name;

        if (baseline_data.find(key) == baseline_data.end()) {
            baseline_data[key] = result;
            std::cout << "[New Baseline] " << test_name << ": "
                      << std::setprecision(8)
                      << result.avg_ms << " ms (min: " << result.min_ms
                      << ", max: " << result.max_ms << ")" << std::endl;
        } else {
            const auto& baseline = baseline_data[key];
            double diff_percent = ((result.avg_ms - baseline.avg_ms) / baseline.avg_ms) * 100.0;
            if (result.avg_ms <= baseline.avg_ms) {
                if (std::abs(diff_percent) > TOLERANCE_PERCENT) {
                    std::cout << "[Performance Improve " << std::abs(diff_percent) << "%] " << test_name << ": "
                              << std::setprecision(8) << result.avg_ms << " ms (min: " << result.min_ms
                              << ", max: " << result.max_ms << ")" << std::endl;
                    baseline_data[key] = result;
                }
            } else {
                EXPECT_LE(std::abs(diff_percent), TOLERANCE_PERCENT)
                        << "Performance regression detected for " << test_name
                        << std::setprecision(8) << std::endl
                        << "Baseline: " << baseline.avg_ms << " ms" << std::endl
                        << "Current: " << result.avg_ms << " ms" << std::endl
                        << "Difference: " << diff_percent << "%" << std::endl;
            }
        }
    }
};

#define PERF_TEST_DEFINE(name) \
    class name##PerformanceTest : public PerformanceTest { \
    public: \
        name##PerformanceTest() { \
            Init(#name); \
        } \
    };

#define PERF_TEST(test_name, case_name) \
    static inline void case_name##_func(); \
    TEST_F(test_name##PerformanceTest, case_name) \
    { \
        auto result = RunBenchmark(#case_name, case_name##_func); \
        CompareToBenchmark(#case_name, result); \
    } \
    static inline void case_name##_func()

#endif
