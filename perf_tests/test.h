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
#include "picasso_backport.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <map>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <vector>

#include "timeuse.h"

#define TEST_WIDTH 640
#define TEST_HEIGHT 480

#define UNUSED(x) ((void)(x))

// Performance test configuration
// Minimum performance difference threshold (in percentage)
// Differences smaller than this will not be considered as regression
// even if they are statistically significant
// Recommended: 5.0% - 10.0% for stable testing
#define PERF_REGRESSION_THRESHOLD 10.0  // 10% threshold

// Statistical significance level (p-value threshold)
#define PERF_SIGNIFICANCE_LEVEL 0.05   // 95% confidence

void PS_Init();
void PS_Shutdown();
ps_canvas* get_test_canvas(void);
void clear_dcache(void);

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
#elif defined(__aarch64__)
    #define ARCH "arm64"
#else
    #define ARCH "unknown"
#endif

class PerformanceTest : public ::testing::Test
{
protected:
    // Adaptive iteration configuration
    static constexpr int DEFAULT_WARMUP_ITERATIONS = 10;
    static constexpr int DEFAULT_TEST_ITERATIONS = 200;
    static constexpr double TOLERANCE_PERCENT = 10.0; // 10% tolerance

    // Target total test duration in milliseconds
    static constexpr double TARGET_TEST_DURATION_MS = 1000.0; // 1 second per test
    static constexpr int MIN_ITERATIONS = 20; // Minimum for statistical validity
    static constexpr int MAX_ITERATIONS = 500; // Maximum to prevent excessive runtime

    struct BenchmarkResult {
        double avg_ms;
        double mid_ms;
        double min_ms;
        double max_ms;
        int iterations; // Actual number of iterations used
        double total_time_ms; // Total test time including cache clearing
        double std_dev; // Standard deviation
        double cv; // Coefficient of variation (std_dev / avg)
    };

    typedef std::map<std::string, BenchmarkResult> BenchmarkData;
    BenchmarkData baseline_data;
    BenchmarkData newbase_data;
    std::string baseline_file;
    std::string newbase_file;

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
        newbase_file = "./benchmark/" + test_name + "_" + SYSTEM + "_" + ARCH + "_new.json";
    }

    void SetUp() override
    {
        LoadBaseline(baseline_file, baseline_data);
        LoadBaseline(newbase_file, newbase_data);

        clear_dcache();
    }

    void TearDown() override
    {
        SaveBaseline(newbase_file);
    }

    bool LoadBaseline(const std::string& file_name, BenchmarkData& data);
    void SaveBaseline(const std::string& file_name);

    // Determine optimal iteration count based on operation speed
    template<typename Func>
    int DetermineIterationCount(const std::string& test_name, Func&& func)
    {
        // First, check if we have a baseline with iteration count
        std::string key = test_name;
        if (baseline_data.find(key) != baseline_data.end()) {
            int baseline_iterations = baseline_data[key].iterations;
            if (baseline_iterations > 0) {
                // Use the same iteration count as baseline for consistency
                std::cout << "[" << test_name << "] Using baseline iterations: "
                          << baseline_iterations << " (for consistency)" << std::endl;
                return baseline_iterations;
            }
        }

        // No baseline or baseline has no iteration info, auto-determine
        const int SAMPLE_COUNT = 5;
        std::vector<double> samples;
        samples.reserve(SAMPLE_COUNT);

        for (int i = 0; i < SAMPLE_COUNT; ++i) {
            clear_dcache();
            clocktime_t start = get_clock();
            func();
            clocktime_t end = get_clock();
            samples.push_back(get_clock_used_ms(start, end));
        }

        // Use median of samples to avoid outliers
        std::sort(samples.begin(), samples.end());
        double median_time = samples[SAMPLE_COUNT / 2];

        // Calculate iterations to reach target duration
        // Account for cache clearing overhead (~1-2ms per iteration)
        double estimated_time_per_iter = median_time + 1.5; // Add cache clear overhead
        int iterations = static_cast<int>(TARGET_TEST_DURATION_MS / estimated_time_per_iter);

        // Clamp to reasonable range
        iterations = std::max(MIN_ITERATIONS, std::min(MAX_ITERATIONS, iterations));

        std::cout << "[" << test_name << "] Auto-determined iterations: "
                  << iterations << " (first run, will be saved as baseline)" << std::endl;

        return iterations;
    }

    template<typename Func>
    BenchmarkResult RunBenchmark(const std::string& test_name, Func&& func, int run_count = -1)
    {
        clocktime_t total_start = get_clock();

        // Auto-determine iteration count if not specified
        if (run_count < 0) {
            run_count = DetermineIterationCount(test_name, func);
        }

        // Adaptive warmup (fewer warmups for slow operations)
        int warmup_count = run_count > 100 ? DEFAULT_WARMUP_ITERATIONS : (DEFAULT_WARMUP_ITERATIONS / 2);

        // Warmup
        for (int i = 0; i < warmup_count; ++i) {
            clear_dcache();
            func();
        }

        std::vector<double> times;
        times.reserve(run_count);

        // Run benchmark iterations
        for (int i = 0; i < run_count; ++i) {
            clear_dcache();
            clocktime_t start = get_clock();
            func();
            clocktime_t end = get_clock();
            times.push_back(get_clock_used_ms(start, end));
        }

        // Sort times
        std::sort(times.begin(), times.end());

        // Discard 10% outliers
        int discard_count = static_cast<int>(times.size() * 0.1);
        std::vector<double> filtered_times(
            times.begin() + discard_count,
            times.end() - discard_count
        );

        BenchmarkResult result;
        result.avg_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        result.min_ms = filtered_times.front();
        result.max_ms = filtered_times.back();
        result.iterations = run_count;

        size_t n = filtered_times.size();
        if (n % 2 == 0) {
            result.mid_ms = (filtered_times[n / 2 - 1] + filtered_times[n / 2]) / 2.0;
        } else {
            result.mid_ms = filtered_times[n / 2];
        }

        // Calculate standard deviation for statistical comparison
        double variance = 0.0;
        for (const auto& t : filtered_times) {
            variance += (t - result.avg_ms) * (t - result.avg_ms);
        }
        result.std_dev = sqrt(variance / filtered_times.size());
        result.cv = result.std_dev / result.avg_ms; // Coefficient of variation

        clocktime_t total_end = get_clock();
        result.total_time_ms = get_clock_used_ms(total_start, total_end);

        return result;
    }

    // Statistical comparison using t-test for different sample sizes
    bool IsStatisticallyDifferent(const BenchmarkResult& baseline,
                                  const BenchmarkResult& current,
                                  double& t_statistic,
                                  double& significance_level)
    {
        // Welch's t-test for samples with potentially different variances and sizes
        // t = (mean1 - mean2) / sqrt(s1²/n1 + s2²/n2)

        double mean_diff = current.avg_ms - baseline.avg_ms;

        // Use coefficient of variation to estimate std_dev if not available
        double baseline_std = (baseline.std_dev > 0) ? baseline.std_dev :
                              (baseline.avg_ms * 0.05); // Assume 5% CV if unknown
        double current_std = current.std_dev;

        int n1 = baseline.iterations;
        int n2 = current.iterations;

        // Standard error of the difference
        double se = sqrt((baseline_std * baseline_std) / n1 +
                         (current_std * current_std) / n2);

        if (se < 1e-10) {
            // Avoid division by zero
            t_statistic = 0.0;
            significance_level = 0.0;
            return false;
        }

        t_statistic = mean_diff / se;

        // Degrees of freedom (Welch-Satterthwaite equation)
        double s1_sq_n1 = (baseline_std * baseline_std) / n1;
        double s2_sq_n2 = (current_std * current_std) / n2;
        double df = (s1_sq_n1 + s2_sq_n2) * (s1_sq_n1 + s2_sq_n2) /
                    (s1_sq_n1 * s1_sq_n1 / (n1 - 1) + s2_sq_n2 * s2_sq_n2 / (n2 - 1));

        // Simplified significance levels (two-tailed test)
        // For large df (>30), use normal distribution approximation
        if (df > 30) {
            if (std::abs(t_statistic) > 2.576) { significance_level = 0.01; } // 99% confidence
            else if (std::abs(t_statistic) > 1.96) { significance_level = 0.05; } // 95% confidence
            else if (std::abs(t_statistic) > 1.645) { significance_level = 0.10; } // 90% confidence
            else { significance_level = 1.0; } // Not significant
        } else {
            // For smaller df, use more conservative thresholds
            if (std::abs(t_statistic) > 3.0) { significance_level = 0.01; }
            else if (std::abs(t_statistic) > 2.0) { significance_level = 0.05; }
            else if (std::abs(t_statistic) > 1.7) { significance_level = 0.10; }
            else { significance_level = 1.0; }
        }

        // Consider significant if p < 0.05 (95% confidence)
        return significance_level < 0.05;
    }

    void CompareToBenchmark(const std::string& test_name, const BenchmarkResult& result)
    {
        std::string key = test_name;

        if (baseline_data.find(key) == baseline_data.end()) {
            std::cout << "[New Baseline] " << test_name << ": "
                      << std::setprecision(6)
                      << "median: " << result.mid_ms << " ms (avg: "
                      << result.avg_ms << " ± " << result.std_dev
                      << ", cv: " << (result.cv * 100) << "%"
                      << ", iterations: " << result.iterations
                      << ", total: " << (result.total_time_ms / 1000.0) << "s)" << std::endl;
        } else {
            const auto& baseline = baseline_data[key];
            double diff_percent = ((result.mid_ms - baseline.mid_ms) / baseline.mid_ms) * 100.0;

            // Statistical comparison
            double t_stat, p_value;
            bool is_significant = IsStatisticallyDifferent(baseline, result, t_stat, p_value);

            // Check if difference exceeds threshold
            bool exceeds_threshold = std::abs(diff_percent) >= PERF_REGRESSION_THRESHOLD;

            if (!is_significant) {
                // No statistically significant difference
                std::cout << "[No Significant Change] " << test_name << ": "
                          << std::setprecision(6)
                          << "median: " << result.mid_ms << " ms "
                          << "(baseline: " << baseline.mid_ms << " ms, "
                          << "diff: " << diff_percent << "%, "
                          << "p=" << p_value << ", not significant)" << std::endl;
            } else if (!exceeds_threshold) {
                // Statistically significant but below threshold - acceptable
                std::cout << "[Acceptable Change] " << test_name << ": "
                          << std::setprecision(6)
                          << "median: " << result.mid_ms << " ms "
                          << "(baseline: " << baseline.mid_ms << " ms, "
                          << "diff: " << diff_percent << "%, "
                          << "below " << PERF_REGRESSION_THRESHOLD << "% threshold)" << std::endl;
            } else if (result.mid_ms < baseline.mid_ms) {
                // Statistically significant improvement above threshold
                std::cout << "[Performance Improve " << std::abs(diff_percent) << "%] " << test_name << ": "
                          << std::setprecision(6) << "median: " << result.mid_ms << " ms "
                          << "(baseline: " << baseline.mid_ms << " ms, "
                          << "t=" << t_stat << ", p=" << p_value << ")" << std::endl;
            } else {
                // Statistically significant regression above threshold - FAIL
                EXPECT_FALSE(is_significant && exceeds_threshold)
                        << "Performance regression detected for " << test_name
                        << std::setprecision(6) << std::endl
                        << "Baseline: " << baseline.avg_ms << " ms (±" << baseline.std_dev
                        << ", n=" << baseline.iterations << ")" << std::endl
                        << "Current:  " << result.avg_ms << " ms (±" << result.std_dev
                        << ", n=" << result.iterations << ")" << std::endl
                        << "Difference: " << diff_percent << "% (threshold: "
                        << PERF_REGRESSION_THRESHOLD << "%)" << std::endl
                        << "Statistical test: t=" << t_stat << ", p=" << p_value << std::endl
                        << "This regression exceeds the acceptable threshold" << std::endl;
            }
        }
        newbase_data[key] = result;
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

#define PERF_TEST_RUN(test_name, case_name) \
    const char * test_name##_##case_name = #case_name; \
    TEST_F(test_name##PerformanceTest, case_name)

#endif
