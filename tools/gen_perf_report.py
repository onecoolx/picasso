#!/usr/bin/env python

import os
import json
import glob
from datetime import datetime
from collections import defaultdict

# HTML template with improved bilingual support and collapsible charts
HTML_TEMPLATE = """<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Performance Benchmark Comparison Report</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/luxon@3.3.0/build/global/luxon.min.js"></script>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <style>
        /* CSS remains the same as before */
        * { margin: 0; padding: 0; box-sizing: border-box; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }
        body { background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); color: #e0e0e0; min-height: 100vh; padding: 20px; }
        .container { max-width: 1400px; margin: 0 auto; }
        .header { text-align: center; margin-bottom: 40px; padding: 30px; background: rgba(255, 255, 255, 0.05); border-radius: 20px; backdrop-filter: blur(10px); border: 1px solid rgba(255, 255, 255, 0.1); box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3); }
        .header h1 { font-size: 2.8rem; background: linear-gradient(45deg, #4cc9f0, #4361ee); -webkit-background-clip: text; background-clip: text; color: transparent; margin-bottom: 10px; }
        .header .subtitle { font-size: 1.2rem; opacity: 0.8; margin-bottom: 20px; }
        .timestamp { font-size: 0.9rem; color: #8a8a8a; }
        .summary-cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; margin-bottom: 40px; }
        .card { background: rgba(255, 255, 255, 0.07); border-radius: 15px; padding: 25px; border: 1px solid rgba(255, 255, 255, 0.1); transition: transform 0.3s, box-shadow 0.3s; }
        .card:hover { transform: translateY(-5px); box-shadow: 0 15px 35px rgba(0, 0, 0, 0.4); }
        .card h3 { display: flex; align-items: center; gap: 10px; margin-bottom: 15px; color: #4cc9f0; }
        .card-content { font-size: 1.1rem; }
        .improvement { color: #4ade80; }
        .regression { color: #f87171; }
        .neutral { color: #94a3b8; }
        .controls { display: flex; justify-content: space-between; align-items: center; margin-bottom: 30px; flex-wrap: wrap; gap: 15px; padding: 20px; background: rgba(255, 255, 255, 0.05); border-radius: 15px; }
        .search-box { flex: 1; min-width: 300px; position: relative; }
        .search-box input { width: 100%; padding: 12px 20px 12px 45px; border-radius: 50px; border: 1px solid rgba(255, 255, 255, 0.2); background: rgba(0, 0, 0, 0.3); color: white; font-size: 1rem; }
        .search-box i { position: absolute; left: 15px; top: 50%; transform: translateY(-50%); color: #8a8a8a; }
        .filter-buttons { display: flex; gap: 10px; flex-wrap: wrap; }
        .filter-btn { padding: 10px 20px; border-radius: 50px; border: 1px solid rgba(255, 255, 255, 0.2); background: rgba(0, 0, 0, 0.3); color: white; cursor: pointer; transition: all 0.3s; }
        .filter-btn.active { background: #4361ee; border-color: #4361ee; }
        .filter-btn:hover { background: rgba(67, 97, 238, 0.3); }
        .chart-container { display: grid; grid-template-columns: 1fr 1fr; gap: 30px; margin-bottom: 40px; }
        @media (max-width: 1100px) { .chart-container { grid-template-columns: 1fr; } }
        .chart-box { background: rgba(255, 255, 255, 0.07); border-radius: 15px; padding: 25px; border: 1px solid rgba(255, 255, 255, 0.1); }
        .chart-box h3 { margin-bottom: 20px; color: #4cc9f0; display: flex; align-items: center; gap: 10px; cursor: pointer; }
        .chart-box h3 .collapse-icon { margin-left: auto; transition: transform 0.3s; }
        .chart-box h3 .collapse-icon.collapsed { transform: rotate(-90deg); }
        .chart-wrapper { position: relative; height: 300px; }
        .chart-wrapper.collapsed { display: none; }
        .comparison-table { background: rgba(255, 255, 255, 0.07); border-radius: 15px; padding: 25px; border: 1px solid rgba(255, 255, 255, 0.1); margin-bottom: 40px; }
        .comparison-table h3 { margin-bottom: 20px; color: #4cc9f0; display: flex; align-items: center; gap: 10px; }

        /* Improved table scrolling - only content scrolls */
        .table-container-wrapper { overflow-x: auto; }
        .table-container { max-height: 500px; overflow-y: auto; position: relative; }
        .table-container table { width: 100%; border-collapse: collapse; min-width: 1000px; }
        .table-container thead { position: sticky; top: 0; z-index: 10; background: #3a4f8c; }
        .table-container th { padding: 15px; text-align: left; border-bottom: 2px solid rgba(255, 255, 255, 0.1); }
        .table-container td { padding: 15px; border-bottom: 1px solid rgba(255, 255, 255, 0.05); }
        .table-container tr:hover { background: rgba(255, 255, 255, 0.05); }

        .test-name { font-weight: bold; color: #4cc9f0; }
        .metric-value { font-family: 'Courier New', monospace; font-size: 0.95rem; }
        .percentage-change { font-weight: bold; padding: 5px 10px; border-radius: 5px; display: inline-block; }
        .positive { background: rgba(72, 187, 120, 0.2); color: #4ade80; }
        .negative { background: rgba(239, 68, 68, 0.2); color: #f87171; }
        .zero { background: rgba(148, 163, 184, 0.2); color: #94a3b8; }
        .legend { display: flex; gap: 20px; margin-top: 20px; flex-wrap: wrap; }
        .legend-item { display: flex; align-items: center; gap: 8px; font-size: 0.9rem; }
        .legend-color { width: 15px; height: 15px; border-radius: 3px; }
        .footer { text-align: center; padding: 20px; margin-top: 40px; color: #8a8a8a; font-size: 0.9rem; border-top: 1px solid rgba(255, 255, 255, 0.1); }
        .no-data { text-align: center; padding: 50px; color: #8a8a8a; font-size: 1.2rem; }
        .stat-badge { display: inline-block; padding: 3px 8px; border-radius: 10px; font-size: 0.8rem; margin-left: 10px; }
        .performance-summary { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin-top: 15px; }
        .performance-item { text-align: center; padding: 15px; background: rgba(255, 255, 255, 0.05); border-radius: 10px; }
        .performance-value { font-size: 1.5rem; font-weight: bold; margin: 5px 0; }
        .performance-label { font-size: 0.9rem; opacity: 0.8; }
        .language-switcher { position: absolute; top: 20px; right: 20px; display: flex; gap: 10px; }
        .lang-btn { padding: 8px 15px; border-radius: 5px; border: 1px solid rgba(255, 255, 255, 0.2); background: rgba(0, 0, 0, 0.3); color: white; cursor: pointer; }
        .lang-btn.active { background: #4361ee; border-color: #4361ee; }
        .benchmark-set { color: #fbbf24; font-weight: bold; }

        /* Pagination styles */
        .pagination-container { display: flex; justify-content: space-between; align-items: center; margin-top: 20px; padding: 15px; background: rgba(255, 255, 255, 0.05); border-radius: 10px; }
        .pagination-info { font-size: 0.9rem; color: #94a3b8; }
        .pagination-controls { display: flex; align-items: center; gap: 10px; }
        .page-btn { padding: 8px 15px; border-radius: 5px; border: 1px solid rgba(255, 255, 255, 0.2); background: rgba(0, 0, 0, 0.3); color: white; cursor: pointer; transition: all 0.3s; }
        .page-btn:hover:not(:disabled) { background: rgba(67, 97, 238, 0.5); }
        .page-btn:disabled { opacity: 0.5; cursor: not-allowed; }
        .page-numbers { display: flex; gap: 5px; }
        .page-number { padding: 8px 12px; border-radius: 5px; border: 1px solid rgba(255, 255, 255, 0.2); background: rgba(0, 0, 0, 0.3); color: white; cursor: pointer; transition: all 0.3s; }
        .page-number.active { background: #4361ee; border-color: #4361ee; }
        .page-number:hover:not(.active) { background: rgba(255, 255, 255, 0.1); }
        .page-size-selector { display: flex; align-items: center; gap: 10px; }
        .page-size-selector select { padding: 8px; border-radius: 5px; border: 1px solid rgba(255, 255, 255, 0.2); background: rgba(0, 0, 0, 0.3); color: white; }

        /* Collapsible sections */
        .section-toggle { display: flex; align-items: center; justify-content: space-between; margin-bottom: 20px; padding: 15px; background: rgba(255, 255, 255, 0.05); border-radius: 10px; cursor: pointer; }
        .section-toggle h3 { margin: 0; color: #4cc9f0; }
        .toggle-icon { transition: transform 0.3s; }
        .toggle-icon.collapsed { transform: rotate(-90deg); }
        .charts-section { margin-bottom: 30px; }
        .charts-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }
        @media (max-width: 1100px) { .charts-grid { grid-template-columns: 1fr; } }

        /* Language-specific styles for dynamic content */
        [data-translate] { display: inline; }
    </style>
</head>
<body>
    <div class="container">
        <div class="language-switcher">
            <button class="lang-btn active" data-lang="en">English</button>
            <button class="lang-btn" data-lang="zh">中文</button>
        </div>

        <div class="header">
            <h1 data-translate="report-title">Performance Benchmark Comparison Report</h1>
            <div class="subtitle" data-translate="report-subtitle">Baseline vs New Test Data Analysis</div>
            <div class="timestamp">Report Generated: {{timestamp}}</div>
        </div>

        {{summary_section}}

        <div class="controls">
            <div class="search-box">
                <i class="fas fa-search"></i>
                <input type="text" id="searchInput" placeholder="Search test items..." data-translate-placeholder="search-placeholder">
            </div>
            <div class="filter-buttons">
                <button class="filter-btn active" data-filter="all" data-translate="filter-all">All</button>
                <button class="filter-btn" data-filter="improved" data-translate="filter-improved">Improved</button>
                <button class="filter-btn" data-filter="regressed" data-translate="filter-regressed">Regressed</button>
                <button class="filter-btn" data-filter="unchanged" data-translate="filter-unchanged">Unchanged</button>
            </div>
        </div>

        <!-- Collapsible charts section -->
        <div class="charts-section">
            <div class="section-toggle" id="charts-toggle">
                <h3><i class="fas fa-chart-bar"></i> <span data-translate="charts-section-title">Performance Charts</span></h3>
                <i class="fas fa-chevron-down toggle-icon" id="charts-toggle-icon"></i>
            </div>
            <div class="charts-grid" id="charts-grid">
                <div class="chart-box">
                    <h3 class="chart-toggle">
                        <i class="fas fa-chart-pie"></i> <span data-translate="distribution-chart-title">Performance Change Distribution</span>
                        <i class="fas fa-chevron-down collapse-icon"></i>
                    </h3>
                    <div class="chart-wrapper">
                        <canvas id="distributionChart"></canvas>
                    </div>
                    <div class="legend">
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(72, 187, 120, 0.8);"></div>
                            <span data-translate="legend-improved">Improved</span>
                        </div>
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(239, 68, 68, 0.8);"></div>
                            <span data-translate="legend-regressed">Regressed</span>
                        </div>
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(148, 163, 184, 0.8);"></div>
                            <span data-translate="legend-unchanged">Unchanged</span>
                        </div>
                    </div>
                </div>

                <div class="chart-box">
                    <h3 class="chart-toggle">
                        <i class="fas fa-chart-bar"></i> <span data-translate="magnitude-chart-title">Performance Change Magnitude</span>
                        <i class="fas fa-chevron-down collapse-icon"></i>
                    </h3>
                    <div class="chart-wrapper">
                        <canvas id="magnitudeChart"></canvas>
                    </div>
                    <div class="legend">
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(72, 187, 120, 0.8);"></div>
                            <span data-translate="legend-improved-faster">Improved (Faster)</span>
                        </div>
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(239, 68, 68, 0.8);"></div>
                            <span data-translate="legend-regressed-slower">Regressed (Slower)</span>
                        </div>
                    </div>
                </div>

                <div class="chart-box">
                    <h3 class="chart-toggle">
                        <i class="fas fa-chart-line"></i> <span data-translate="comparison-chart-title">Median Time Comparison</span>
                        <i class="fas fa-chevron-down collapse-icon"></i>
                    </h3>
                    <div class="chart-wrapper">
                        <canvas id="comparisonChart"></canvas>
                    </div>
                    <div class="legend">
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(76, 201, 240, 0.8);"></div>
                            <span data-translate="legend-baseline-data">Baseline Data (mid_ms)</span>
                        </div>
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(245, 158, 11, 0.8);"></div>
                            <span data-translate="legend-new-data">New Test Data (mid_ms)</span>
                        </div>
                    </div>
                </div>

                <div class="chart-box">
                    <h3 class="chart-toggle">
                        <i class="fas fa-bullseye"></i> <span data-translate="radar-chart-title">Average Time Radar Comparison</span>
                        <i class="fas fa-chevron-down collapse-icon"></i>
                    </h3>
                    <div class="chart-wrapper">
                        <canvas id="radarChart"></canvas>
                    </div>
                    <div class="legend">
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(76, 201, 240, 0.8);"></div>
                            <span data-translate="legend-baseline-avg">Baseline Data (avg_ms)</span>
                        </div>
                        <div class="legend-item">
                            <div class="legend-color" style="background-color: rgba(245, 158, 11, 0.8);"></div>
                            <span data-translate="legend-new-avg">New Test Data (avg_ms)</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <div class="comparison-table">
            <h3><i class="fas fa-table"></i> <span data-translate="table-title">Detailed Performance Data Comparison</span></h3>
            {{comparison_table}}
        </div>

        <div class="footer">
            <p><span data-translate="footer-line1">Performance Benchmark Comparison Report | Auto-generated | Data Source: benchmark/ directory</span></p>
            <p><span data-translate="footer-line2">Note: All time units are milliseconds (ms), percentage change calculated based on median (mid_ms)</span></p>
        </div>
    </div>

    <script>
        // Performance data
        const performanceData = {{performance_data}};

        // Pagination variables
        let currentPage = 1;
        let rowsPerPage = 10;
        let filteredData = [];

        // Language support
        let currentLang = 'en';

        // Bilingual text dictionary
        const translations = {
            en: {
                // Header
                'report-title': 'Performance Benchmark Comparison Report',
                'report-subtitle': 'Baseline vs New Test Data Analysis',

                // Search placeholder
                'search-placeholder': 'Search test items...',

                // Filter buttons
                'filter-all': 'All',
                'filter-improved': 'Improved',
                'filter-regressed': 'Regressed',
                'filter-unchanged': 'Unchanged',

                // Charts section
                'charts-section-title': 'Performance Charts',

                // Table title
                'table-title': 'Detailed Performance Data Comparison',

                // Footer
                'footer-line1': 'Performance Benchmark Comparison Report | Auto-generated | Data Source: benchmark/ directory',
                'footer-line2': 'Note: All time units are milliseconds (ms), percentage change calculated based on median (mid_ms)',

                // Summary cards
                'test-overview': 'Test Overview',
                'total-benchmark-sets': 'Total Benchmark Sets:',
                'total-test-items': 'Total Test Items:',
                'improved': 'Improved',
                'regressed': 'Regressed',
                'unchanged': 'Unchanged',

                'performance-improvements': 'Performance Improvements',
                'average-improvement': 'Average Improvement:',
                'best-improvement': 'Best Improvement:',
                'improved-items': 'Improved Items:',

                'performance-regressions': 'Performance Regressions',
                'average-regression': 'Average Regression:',
                'worst-regression': 'Worst Regression:',
                'regressed-items': 'Regressed Items:',

                'data-statistics': 'Data Statistics',
                'baseline-files': 'Baseline Files:',
                'new-test-files': 'New Test Files:',
                'valid-comparisons': 'Valid Comparisons:',
                'benchmark-sets': 'Benchmark Sets:',

                // Chart titles
                'distribution-chart-title': 'Performance Change Distribution',
                'magnitude-chart-title': 'Performance Change Magnitude',
                'comparison-chart-title': 'Median Time Comparison',
                'radar-chart-title': 'Average Time Radar Comparison',

                // Chart legends
                'legend-improved': 'Improved',
                'legend-regressed': 'Regressed',
                'legend-unchanged': 'Unchanged',
                'legend-improved-faster': 'Improved (Faster)',
                'legend-regressed-slower': 'Regressed (Slower)',
                'legend-baseline-data': 'Baseline Data (mid_ms)',
                'legend-new-data': 'New Test Data (mid_ms)',
                'legend-baseline-avg': 'Baseline Data (avg_ms)',
                'legend-new-avg': 'New Test Data (avg_ms)',

                // Table headers
                'benchmark-set': 'Benchmark Set',
                'test-item': 'Test Item',
                'baseline-median': 'Baseline Median (ms)',
                'new-median': 'New Median (ms)',
                'change': 'Change %',
                'baseline-avg': 'Baseline Avg (ms)',
                'new-avg': 'New Avg (ms)',
                'baseline-min': 'Baseline Min (ms)',
                'new-min': 'New Min (ms)',

                // Pagination
                'page-info': 'Showing {start} to {end} of {total} entries',
                'previous': 'Previous',
                'next': 'Next',
                'rows-per-page': 'Rows per page:',

                // No data message
                'no-data': 'No data available for comparison'
            },
            zh: {
                // Header
                'report-title': '性能基准对比报告',
                'report-subtitle': '基准数据 vs 新测试数据分析',

                // Search placeholder
                'search-placeholder': '搜索测试项...',

                // Filter buttons
                'filter-all': '全部',
                'filter-improved': '性能提升',
                'filter-regressed': '性能下降',
                'filter-unchanged': '无变化',

                // Charts section
                'charts-section-title': '性能图表',

                // Table title
                'table-title': '详细性能数据对比',

                // Footer
                'footer-line1': '性能基准对比报告 | 自动生成 | 数据来源: benchmark/ 目录',
                'footer-line2': '注: 所有时间单位均为毫秒(ms)，百分比变化基于中位数(mid_ms)计算',

                // Summary cards
                'test-overview': '测试概况',
                'total-benchmark-sets': '总测试集:',
                'total-test-items': '总测试项:',
                'improved': '性能提升',
                'regressed': '性能下降',
                'unchanged': '无变化',

                'performance-improvements': '性能提升',
                'average-improvement': '平均提升幅度:',
                'best-improvement': '最佳提升项:',
                'improved-items': '提升项数量:',

                'performance-regressions': '性能下降',
                'average-regression': '平均下降幅度:',
                'worst-regression': '最差下降项:',
                'regressed-items': '下降项数量:',

                'data-statistics': '数据统计',
                'baseline-files': '基准文件数:',
                'new-test-files': '新测试文件:',
                'valid-comparisons': '有效对比项:',
                'benchmark-sets': '测试集数量:',

                // Chart titles
                'distribution-chart-title': '性能变化分布',
                'magnitude-chart-title': '性能变化幅度',
                'comparison-chart-title': '中位数时间对比',
                'radar-chart-title': '平均时间雷达对比',

                // Chart legends
                'legend-improved': '性能提升',
                'legend-regressed': '性能下降',
                'legend-unchanged': '无变化',
                'legend-improved-faster': '性能提升 (更快)',
                'legend-regressed-slower': '性能下降 (更慢)',
                'legend-baseline-data': '基准数据 (mid_ms)',
                'legend-new-data': '新测试数据 (mid_ms)',
                'legend-baseline-avg': '基准数据 (avg_ms)',
                'legend-new-avg': '新测试数据 (avg_ms)',

                // Table headers
                'benchmark-set': '测试集',
                'test-item': '测试项',
                'baseline-median': '基准中位数 (ms)',
                'new-median': '新中位数 (ms)',
                'change': '变化 %',
                'baseline-avg': '基准平均值 (ms)',
                'new-avg': '新平均值 (ms)',
                'baseline-min': '基准最小值 (ms)',
                'new-min': '新最小值 (ms)',

                // Pagination
                'page-info': '显示第 {start} 到 {end} 条，共 {total} 条',
                'previous': '上一页',
                'next': '下一页',
                'rows-per-page': '每页显示:',

                // No data message
                'no-data': '没有可比较的数据'
            }
        };

        // Enhanced language switching function
        function switchLanguage(lang) {
            currentLang = lang;

            // Update language buttons
            document.querySelectorAll('.lang-btn').forEach(btn => {
                btn.classList.remove('active');
                if (btn.dataset.lang === lang) {
                    btn.classList.add('active');
                }
            });

            // Update all elements with data-translate attribute
            document.querySelectorAll('[data-translate]').forEach(el => {
                const key = el.getAttribute('data-translate');
                if (translations[lang] && translations[lang][key]) {
                    el.textContent = translations[lang][key];
                }
            });

            // Update input placeholders
            document.querySelectorAll('[data-translate-placeholder]').forEach(el => {
                const key = el.getAttribute('data-translate-placeholder');
                if (translations[lang] && translations[lang][key]) {
                    el.placeholder = translations[lang][key];
                }
            });

            // Update filter buttons
            document.querySelectorAll('.filter-btn').forEach(btn => {
                const filter = btn.dataset.filter;
                if (filter && translations[lang][`filter-${filter}`]) {
                    btn.textContent = translations[lang][`filter-${filter}`];
                }
            });

            // Update table headers
            updateTableHeaders(lang);

            // Update pagination info and controls
            updatePaginationInfo();
            updatePaginationControlsText(lang);

            // Update chart labels and tooltips
            updateChartsLanguage(lang);
        }

        function updatePaginationControlsText(lang) {
            // Update pagination control texts if they exist
            const rowsPerPageText = document.getElementById('rows-per-page-text');
            if (rowsPerPageText && translations[lang]['rows-per-page']) {
                rowsPerPageText.textContent = translations[lang]['rows-per-page'];
            }

            const prevPageBtn = document.getElementById('prev-page');
            if (prevPageBtn && translations[lang]['previous']) {
                prevPageBtn.textContent = translations[lang]['previous'];
            }

            const nextPageBtn = document.getElementById('next-page');
            if (nextPageBtn && translations[lang]['next']) {
                nextPageBtn.textContent = translations[lang]['next'];
            }

            // Update select options if needed
            const rowsPerPageSelect = document.getElementById('rows-per-page-select');
            if (rowsPerPageSelect) {
                // If we need to translate options, we would do it here
            }
        }

        function updateTableHeaders(lang) {
            const thElements = document.querySelectorAll('#comparisonTable th');
            const headerKeys = [
                'benchmark-set',
                'test-item',
                'baseline-median',
                'new-median',
                'change',
                'baseline-avg',
                'new-avg',
                'baseline-min',
                'new-min'
            ];

            thElements.forEach((th, index) => {
                if (index < headerKeys.length && translations[lang][headerKeys[index]]) {
                    th.textContent = translations[lang][headerKeys[index]];
                }
            });
        }

        function updateChartsLanguage(lang) {
            // Update distribution chart labels
            const distributionChart = Chart.getChart('distributionChart');
            if (distributionChart) {
                distributionChart.data.labels = [
                    translations[lang]['legend-improved'],
                    translations[lang]['legend-regressed'],
                    translations[lang]['legend-unchanged']
                ];

                // Update tooltip callback
                distributionChart.options.plugins.tooltip.callbacks.label = function(context) {
                    const label = context.label || '';
                    const value = context.raw || 0;
                    const total = context.dataset.data.reduce((a, b) => a + b, 0);
                    const percentage = Math.round((value / total) * 100);
                    const itemText = lang === 'zh' ? '项' : 'items';
                    return `${label}: ${value} ${itemText} (${percentage}%)`;
                };

                distributionChart.update();
            }

            // Update magnitude chart label
            const magnitudeChart = Chart.getChart('magnitudeChart');
            if (magnitudeChart) {
                magnitudeChart.data.datasets[0].label = translations[lang]['magnitude-chart-title'] + ' (%)';

                // Update tooltip callback
                magnitudeChart.options.plugins.tooltip.callbacks.label = function(context) {
                    const changeText = lang === 'zh' ? '变化' : 'Change';
                    return `${changeText}: ${context.raw.toFixed(2)}%`;
                };

                magnitudeChart.update();
            }
        }

        // Collapsible charts functionality
        document.querySelectorAll('.chart-toggle').forEach(toggle => {
            toggle.addEventListener('click', function() {
                const chartWrapper = this.nextElementSibling;
                const collapseIcon = this.querySelector('.collapse-icon');

                chartWrapper.classList.toggle('collapsed');
                collapseIcon.classList.toggle('collapsed');

                // If we have a chart in this wrapper, resize it
                const canvas = chartWrapper.querySelector('canvas');
                if (canvas) {
                    const chart = Chart.getChart(canvas);
                    if (chart) {
                        setTimeout(() => {
                            chart.resize();
                        }, 300);
                    }
                }
            });
        });

        // Collapsible charts section
        document.getElementById('charts-toggle').addEventListener('click', function() {
            const chartsGrid = document.getElementById('charts-grid');
            const toggleIcon = document.getElementById('charts-toggle-icon');

            chartsGrid.classList.toggle('collapsed');
            toggleIcon.classList.toggle('collapsed');

            // Resize all charts when section is expanded
            if (!chartsGrid.classList.contains('collapsed')) {
                setTimeout(() => {
                    const charts = ['distributionChart', 'magnitudeChart', 'comparisonChart', 'radarChart'];
                    charts.forEach(chartId => {
                        const chart = Chart.getChart(chartId);
                        if (chart) {
                            chart.resize();
                        }
                    });
                }, 300);
            }
        });

        // Pagination functions
        function setupPagination() {
            const tableBody = document.querySelector('#comparisonTable tbody');
            if (!tableBody) return;

            // Get all rows
            const allRows = Array.from(tableBody.querySelectorAll('tr'));

            // Initialize filteredData with all rows
            filteredData = allRows;

            // Show initial page
            displayPage(1);

            // Setup pagination controls
            setupPaginationControls();
        }

        function displayPage(page) {
            currentPage = page;
            const tableBody = document.querySelector('#comparisonTable tbody');
            if (!tableBody) return;

            // Get all rows
            const allRows = Array.from(tableBody.querySelectorAll('tr'));

            // Hide all rows
            allRows.forEach(row => {
                row.style.display = 'none';
            });

            // Calculate start and end indices
            const startIndex = (page - 1) * rowsPerPage;
            const endIndex = startIndex + rowsPerPage;

            // Show rows for current page
            const pageRows = filteredData.slice(startIndex, endIndex);
            pageRows.forEach(row => {
                row.style.display = '';
            });

            // Update pagination info
            updatePaginationInfo();

            // Update pagination buttons
            updatePaginationButtons();
        }

        function updatePaginationInfo() {
            const paginationInfo = document.querySelector('.pagination-info');
            if (!paginationInfo) return;

            const totalRows = filteredData.length;
            const startRow = totalRows > 0 ? (currentPage - 1) * rowsPerPage + 1 : 0;
            const endRow = Math.min(currentPage * rowsPerPage, totalRows);

            let infoText = translations[currentLang]['page-info']
                .replace('{start}', startRow)
                .replace('{end}', endRow)
                .replace('{total}', totalRows);

            paginationInfo.textContent = infoText;
        }

        function updatePaginationButtons() {
            const totalRows = filteredData.length;
            const totalPages = Math.ceil(totalRows / rowsPerPage);

            // Update previous/next buttons
            const prevBtn = document.getElementById('prev-page');
            const nextBtn = document.getElementById('next-page');

            if (prevBtn) {
                prevBtn.disabled = currentPage === 1;
            }

            if (nextBtn) {
                nextBtn.disabled = currentPage === totalPages || totalPages === 0;
            }

            // Update page numbers
            updatePageNumbers(totalPages);
        }

        function updatePageNumbers(totalPages) {
            const pageNumbersContainer = document.querySelector('.page-numbers');
            if (!pageNumbersContainer) return;

            // Clear existing page numbers
            pageNumbersContainer.innerHTML = '';

            // Show up to 5 page numbers
            let startPage = Math.max(1, currentPage - 2);
            let endPage = Math.min(totalPages, startPage + 4);

            // Adjust start page if we're near the end
            if (endPage - startPage < 4 && startPage > 1) {
                startPage = Math.max(1, endPage - 4);
            }

            // Add page number buttons
            for (let i = startPage; i <= endPage; i++) {
                const pageBtn = document.createElement('button');
                pageBtn.className = `page-number ${i === currentPage ? 'active' : ''}`;
                pageBtn.textContent = i;
                pageBtn.addEventListener('click', () => {
                    displayPage(i);
                });
                pageNumbersContainer.appendChild(pageBtn);
            }
        }

        function setupPaginationControls() {
            // Create pagination container if it doesn't exist
            if (!document.querySelector('.pagination-container')) {
                const tableContainer = document.querySelector('.comparison-table');
                const tableWrapper = document.querySelector('.table-container-wrapper');

                if (!tableWrapper) return;

                // Create pagination container
                const paginationContainer = document.createElement('div');
                paginationContainer.className = 'pagination-container';
                paginationContainer.innerHTML = `
                    <div class="pagination-info"></div>
                    <div class="pagination-controls">
                        <div class="page-size-selector">
                            <span id="rows-per-page-text">${translations[currentLang]['rows-per-page']}</span>
                            <select id="rows-per-page-select">
                                <option value="5">5</option>
                                <option value="10" selected>10</option>
                                <option value="20">20</option>
                                <option value="50">50</option>
                                <option value="100">100</option>
                            </select>
                        </div>
                        <button class="page-btn" id="prev-page">${translations[currentLang]['previous']}</button>
                        <div class="page-numbers"></div>
                        <button class="page-btn" id="next-page">${translations[currentLang]['next']}</button>
                    </div>
                `;

                // Insert pagination container after the table wrapper
                tableWrapper.parentNode.insertBefore(paginationContainer, tableWrapper.nextSibling);

                // Add event listeners
                document.getElementById('prev-page').addEventListener('click', () => {
                    if (currentPage > 1) {
                        displayPage(currentPage - 1);
                    }
                });

                document.getElementById('next-page').addEventListener('click', () => {
                    const totalRows = filteredData.length;
                    const totalPages = Math.ceil(totalRows / rowsPerPage);
                    if (currentPage < totalPages) {
                        displayPage(currentPage + 1);
                    }
                });

                document.getElementById('rows-per-page-select').addEventListener('change', function() {
                    rowsPerPage = parseInt(this.value);
                    displayPage(1);
                });
            }
        }

        // Filter functionality
        document.querySelectorAll('.filter-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                // Update active state
                document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
                this.classList.add('active');

                const filter = this.dataset.filter;
                const tableBody = document.querySelector('#comparisonTable tbody');
                if (!tableBody) return;

                // Get all rows
                const allRows = Array.from(tableBody.querySelectorAll('tr'));

                // Filter rows based on selection
                if (filter === 'all') {
                    filteredData = allRows;
                } else {
                    filteredData = allRows.filter(row => {
                        const changeClass = row.querySelector('.percentage-change').className;
                        if (filter === 'improved' && changeClass.includes('positive')) {
                            return true;
                        } else if (filter === 'regressed' && changeClass.includes('negative')) {
                            return true;
                        } else if (filter === 'unchanged' && changeClass.includes('zero')) {
                            return true;
                        }
                        return false;
                    });
                }

                // Reset to first page after filtering
                displayPage(1);
            });
        });

        // Search functionality
        document.getElementById('searchInput').addEventListener('input', function(e) {
            const searchTerm = e.target.value.toLowerCase();
            const tableBody = document.querySelector('#comparisonTable tbody');
            if (!tableBody) return;

            // Get all rows
            const allRows = Array.from(tableBody.querySelectorAll('tr'));

            // Filter rows based on search term
            filteredData = allRows.filter(row => {
                const testName = row.querySelector('.test-name').textContent.toLowerCase();
                const benchmarkSet = row.querySelector('.benchmark-set').textContent.toLowerCase();
                return testName.includes(searchTerm) || benchmarkSet.includes(searchTerm);
            });

            // Reset to first page after searching
            displayPage(1);
        });

        // Initialize charts
        function initCharts() {
            // Performance distribution pie chart
            const distributionCtx = document.getElementById('distributionChart').getContext('2d');
            const distributionData = {
                labels: [
                    translations[currentLang]['legend-improved'],
                    translations[currentLang]['legend-regressed'],
                    translations[currentLang]['legend-unchanged']
                ],
                datasets: [{
                    data: performanceData.summary.distribution,
                    backgroundColor: [
                        'rgba(72, 187, 120, 0.8)',
                        'rgba(239, 68, 68, 0.8)',
                        'rgba(148, 163, 184, 0.8)'
                    ],
                    borderColor: [
                        'rgba(72, 187, 120, 1)',
                        'rgba(239, 68, 68, 1)',
                        'rgba(148, 163, 184, 1)'
                    ],
                    borderWidth: 1
                }]
            };

            new Chart(distributionCtx, {
                type: 'doughnut',
                data: distributionData,
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: {
                            position: 'bottom',
                            labels: {
                                color: '#e0e0e0',
                                padding: 20
                            }
                        },
                        tooltip: {
                            callbacks: {
                                label: function(context) {
                                    const label = context.label || '';
                                    const value = context.raw || 0;
                                    const total = context.dataset.data.reduce((a, b) => a + b, 0);
                                    const percentage = Math.round((value / total) * 100);
                                    const itemText = currentLang === 'zh' ? '项' : 'items';
                                    return `${label}: ${value} ${itemText} (${percentage}%)`;
                                }
                            }
                        }
                    }
                }
            });

            // Performance change magnitude bar chart
            const magnitudeCtx = document.getElementById('magnitudeChart').getContext('2d');

            // Prepare data
            const testNames = performanceData.chartData.testNames;
            const improvements = performanceData.chartData.improvements;
            const colors = performanceData.chartData.colors;

            new Chart(magnitudeCtx, {
                type: 'bar',
                data: {
                    labels: testNames,
                    datasets: [{
                        label: translations[currentLang]['magnitude-chart-title'] + ' (%)',
                        data: improvements,
                        backgroundColor: colors,
                        borderColor: colors.map(c => c.replace('0.8', '1')),
                        borderWidth: 1
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    indexAxis: 'y',
                    scales: {
                        x: {
                            beginAtZero: true,
                            grid: {
                                color: 'rgba(255, 255, 255, 0.1)'
                            },
                            ticks: {
                                color: '#e0e0e0',
                                callback: function(value) {
                                    return value + '%';
                                }
                            }
                        },
                        y: {
                            grid: {
                                color: 'rgba(255, 255, 255, 0.1)'
                            },
                            ticks: {
                                color: '#e0e0e0'
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            display: false
                        },
                        tooltip: {
                            callbacks: {
                                label: function(context) {
                                    const changeText = currentLang === 'zh' ? '变化' : 'Change';
                                    return `${changeText}: ${context.raw.toFixed(2)}%`;
                                }
                            }
                        }
                    }
                }
            });

            // Median comparison line chart
            const comparisonCtx = document.getElementById('comparisonChart').getContext('2d');

            new Chart(comparisonCtx, {
                type: 'line',
                data: performanceData.chartData.comparisonChartData,
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        x: {
                            grid: {
                                color: 'rgba(255, 255, 255, 0.1)'
                            },
                            ticks: {
                                color: '#e0e0e0'
                            }
                        },
                        y: {
                            beginAtZero: false,
                            grid: {
                                color: 'rgba(255, 255, 255, 0.1)'
                            },
                            ticks: {
                                color: '#e0e0e0',
                                callback: function(value) {
                                    return value + ' ms';
                                }
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            labels: {
                                color: '#e0e0e0'
                            }
                        },
                        tooltip: {
                            callbacks: {
                                label: function(context) {
                                    return `${context.dataset.label}: ${context.parsed.y.toFixed(6)} ms`;
                                }
                            }
                        }
                    }
                }
            });

            // Average time comparison radar chart
            const radarCtx = document.getElementById('radarChart').getContext('2d');

            new Chart(radarCtx, {
                type: 'radar',
                data: performanceData.chartData.radarChartData,
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        r: {
                            angleLines: {
                                color: 'rgba(255, 255, 255, 0.1)'
                            },
                            grid: {
                                color: 'rgba(255, 255, 255, 0.1)'
                            },
                            pointLabels: {
                                color: '#e0e0e0'
                            },
                            ticks: {
                                color: '#e0e0e0',
                                backdropColor: 'transparent',
                                callback: function(value) {
                                    return value + ' ms';
                                }
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            labels: {
                                color: '#e0e0e0'
                            }
                        }
                    }
                }
            });
        }

        // Initialize on page load
        document.addEventListener('DOMContentLoaded', function() {
            initCharts();
            setupPagination();

            // Initialize language switcher
            document.querySelectorAll('.lang-btn').forEach(btn => {
                btn.addEventListener('click', function() {
                    switchLanguage(this.dataset.lang);
                });
            });
        });
    </script>
</body>
</html>
"""

class PerformanceAnalyzer:
    def __init__(self, benchmark_dir="benchmark"):
        self.benchmark_dir = benchmark_dir
        self.baseline_files = []
        self.new_files = []
        self.benchmark_sets = {}  # {set_name: {baseline_data: {}, new_data: {}}}
        self.comparison_data = []  # List of comparison data
        self.summary_stats = {
            'total_tests': 0,
            'total_sets': 0,
            'improved': 0,
            'regressed': 0,
            'unchanged': 0,
            'avg_improvement': 0,
            'avg_regression': 0
        }

    def load_json_files(self):
        """Load all JSON files from benchmark directory"""
        # Check if benchmark directory exists
        if not os.path.exists(self.benchmark_dir):
            print(f"Error: {self.benchmark_dir} directory does not exist")
            return False

        # Find all JSON files
        json_pattern = os.path.join(self.benchmark_dir, "*.json")
        all_json_files = glob.glob(json_pattern)

        if not all_json_files:
            print(f"Warning: No JSON files found in {self.benchmark_dir}")
            return False

        # Organize files by benchmark set
        benchmark_sets = {}

        for file_path in all_json_files:
            file_name = os.path.basename(file_path)
            file_name_no_ext = os.path.splitext(file_name)[0]

            # Determine if this is a baseline or new file
            is_new = file_name.endswith("_new.json")

            # Extract benchmark set name
            if is_new:
                # Remove '_new' suffix
                set_name = file_name_no_ext.replace('_new', '')
            else:
                set_name = file_name_no_ext

            # Initialize set if not exists
            if set_name not in benchmark_sets:
                benchmark_sets[set_name] = {
                    'baseline_file': None,
                    'new_file': None,
                    'baseline_data': {},
                    'new_data': {}
                }

            # Load JSON data
            try:
                with open(file_path, 'r') as f:
                    data = json.load(f)

                if is_new:
                    benchmark_sets[set_name]['new_file'] = file_path
                    benchmark_sets[set_name]['new_data'] = data
                    self.new_files.append(file_path)
                else:
                    benchmark_sets[set_name]['baseline_file'] = file_path
                    benchmark_sets[set_name]['baseline_data'] = data
                    self.baseline_files.append(file_path)

            except Exception as e:
                print(f"Warning: Failed to read file {file_path}: {e}")

        # Store only sets that have baseline data
        for set_name, set_data in benchmark_sets.items():
            if set_data['baseline_data']:  # Only include sets with baseline data
                self.benchmark_sets[set_name] = set_data
                self.summary_stats['total_sets'] += 1

        print(f"Found {len(self.baseline_files)} baseline files")
        print(f"Found {len(self.new_files)} new test files")
        print(f"Organized into {len(self.benchmark_sets)} benchmark sets")

        return True

    def compare_performance(self):
        """Compare baseline and new data for each benchmark set"""
        for set_name, set_data in self.benchmark_sets.items():
            baseline_data = set_data['baseline_data']
            new_data = set_data['new_data']

            # Get all test items from both baseline and new data
            all_test_items = set(list(baseline_data.keys()) + list(new_data.keys()))

            for test_item in all_test_items:
                baseline_metrics = baseline_data.get(test_item, {})
                new_metrics = new_data.get(test_item, {})

                # Only compare if both have 'mid_ms' metric
                if 'mid_ms' in baseline_metrics and 'mid_ms' in new_metrics:
                    baseline_mid = baseline_metrics['mid_ms']
                    new_mid = new_metrics['mid_ms']

                    # Calculate percentage change
                    if baseline_mid > 0:
                        change_percent = ((new_mid - baseline_mid) / baseline_mid) * 100
                    else:
                        change_percent = 0

                    # Determine change type
                    if abs(change_percent) < 0.1:  # Less than 0.1% change considered unchanged
                        change_type = 'unchanged'
                        self.summary_stats['unchanged'] += 1
                    elif change_percent < 0:
                        change_type = 'improved'
                        self.summary_stats['improved'] += 1
                        self.summary_stats['avg_improvement'] += abs(change_percent)
                    else:
                        change_type = 'regressed'
                        self.summary_stats['regressed'] += 1
                        self.summary_stats['avg_regression'] += change_percent

                    self.summary_stats['total_tests'] += 1

                    # Collect comparison data
                    comparison = {
                        'benchmark_set': set_name,
                        'test_item': test_item,
                        'baseline_mid_ms': baseline_mid,
                        'new_mid_ms': new_mid,
                        'change_percent': change_percent,
                        'change_type': change_type,
                        'baseline_avg_ms': baseline_metrics.get('avg_ms', 0),
                        'new_avg_ms': new_metrics.get('avg_ms', 0),
                        'baseline_min_ms': baseline_metrics.get('min_ms', 0),
                        'new_min_ms': new_metrics.get('min_ms', 0),
                        'baseline_max_ms': baseline_metrics.get('max_ms', 0),
                        'new_max_ms': new_metrics.get('max_ms', 0)
                    }

                    self.comparison_data.append(comparison)

        # Calculate average changes
        if self.summary_stats['improved'] > 0:
            self.summary_stats['avg_improvement'] /= self.summary_stats['improved']
        if self.summary_stats['regressed'] > 0:
            self.summary_stats['avg_regression'] /= self.summary_stats['regressed']

    def generate_summary_section(self):
        """Generate summary section HTML with proper translation attributes"""
        improvement_icon = "fas fa-arrow-up" if self.summary_stats['improved'] > 0 else "fas fa-minus"
        regression_icon = "fas fa-arrow-down" if self.summary_stats['regressed'] > 0 else "fas fa-minus"

        return f"""
        <div class="summary-cards">
            <div class="card">
                <h3><i class="fas fa-clipboard-list"></i> <span data-translate="test-overview">Test Overview</span></h3>
                <div class="card-content">
                    <p><span data-translate="total-benchmark-sets">Total Benchmark Sets:</span> <strong>{self.summary_stats['total_sets']}</strong></p>
                    <p><span data-translate="total-test-items">Total Test Items:</span> <strong>{self.summary_stats['total_tests']}</strong></p>
                    <div class="performance-summary">
                        <div class="performance-item">
                            <div class="performance-value improvement">{self.summary_stats['improved']}</div>
                            <div class="performance-label" data-translate="improved">Improved</div>
                        </div>
                        <div class="performance-item">
                            <div class="performance-value regression">{self.summary_stats['regressed']}</div>
                            <div class="performance-label" data-translate="regressed">Regressed</div>
                        </div>
                        <div class="performance-item">
                            <div class="performance-value neutral">{self.summary_stats['unchanged']}</div>
                            <div class="performance-label" data-translate="unchanged">Unchanged</div>
                        </div>
                    </div>
                </div>
            </div>

            <div class="card">
                <h3><i class="{improvement_icon}"></i> <span data-translate="performance-improvements">Performance Improvements</span></h3>
                <div class="card-content">
                    <p><span data-translate="average-improvement">Average Improvement:</span> <strong class="improvement">{abs(self.summary_stats['avg_improvement']):.2f}%</strong></p>
                    <p><span data-translate="best-improvement">Best Improvement:</span> <strong class="improvement">{self.get_best_improvement():.2f}%</strong></p>
                    <p><span data-translate="improved-items">Improved Items:</span> <strong>{self.summary_stats['improved']}</strong></p>
                </div>
            </div>

            <div class="card">
                <h3><i class="{regression_icon}"></i> <span data-translate="performance-regressions">Performance Regressions</span></h3>
                <div class="card-content">
                    <p><span data-translate="average-regression">Average Regression:</span> <strong class="regression">{self.summary_stats['avg_regression']:.2f}%</strong></p>
                    <p><span data-translate="worst-regression">Worst Regression:</span> <strong class="regression">{self.get_worst_regression():.2f}%</strong></p>
                    <p><span data-translate="regressed-items">Regressed Items:</span> <strong>{self.summary_stats['regressed']}</strong></p>
                </div>
            </div>

            <div class="card">
                <h3><i class="fas fa-chart-line"></i> <span data-translate="data-statistics">Data Statistics</span></h3>
                <div class="card-content">
                    <p><span data-translate="baseline-files">Baseline Files:</span> <strong>{len(self.baseline_files)}</strong></p>
                    <p><span data-translate="new-test-files">New Test Files:</span> <strong>{len(self.new_files)}</strong></p>
                    <p><span data-translate="valid-comparisons">Valid Comparisons:</span> <strong>{self.summary_stats['total_tests']}</strong></p>
                    <p><span data-translate="benchmark-sets">Benchmark Sets:</span> <strong>{self.summary_stats['total_sets']}</strong></p>
                </div>
            </div>
        </div>
        """

    def get_best_improvement(self):
        """Get best performance improvement percentage"""
        if not self.comparison_data:
            return 0

        improvements = [abs(d['change_percent']) for d in self.comparison_data if d['change_percent'] < 0]
        return max(improvements) if improvements else 0

    def get_worst_regression(self):
        """Get worst performance regression percentage"""
        if not self.comparison_data:
            return 0

        regressions = [d['change_percent'] for d in self.comparison_data if d['change_percent'] > 0]
        return max(regressions) if regressions else 0

    def generate_comparison_table(self):
        """Generate comparison table HTML"""
        if not self.comparison_data:
            return '<div class="no-data" data-translate="no-data">No data available for comparison</div>'

        # Sort by performance change magnitude
        #sorted_data = sorted(self.comparison_data, key=lambda x: abs(x['change_percent']), reverse=True)
        sorted_data = self.comparison_data # not sort

        rows = []
        for i, data in enumerate(sorted_data):
            change_class = "positive" if data['change_percent'] < -0.1 else ("negative" if data['change_percent'] > 0.1 else "zero")
            change_icon = "fa-arrow-up" if data['change_percent'] < -0.1 else ("fa-arrow-down" if data['change_percent'] > 0.1 else "")
            change_sign = "+" if data['change_percent'] < 0 else ""

            row = f"""
            <tr>
                <td class="benchmark-set">{data['benchmark_set']}</td>
                <td class="test-name">{data['test_item']}</td>
                <td class="metric-value">{data['baseline_mid_ms']:.6f}</td>
                <td class="metric-value">{data['new_mid_ms']:.6f}</td>
                <td>
                    <span class="percentage-change {change_class}">
                        <i class="fas {change_icon}"></i> {change_sign}{abs(data['change_percent']):.2f}%
                    </span>
                </td>
                <td class="metric-value">{data['baseline_avg_ms']:.6f}</td>
                <td class="metric-value">{data['new_avg_ms']:.6f}</td>
                <td class="metric-value">{data['new_max_ms']:.6f}</td>
                <td class="metric-value">{data['new_min_ms']:.6f}</td>
            </tr>
            """
            rows.append(row)

        table_html = f"""
        <div class="table-container-wrapper">
            <div class="table-container">
                <table id="comparisonTable">
                    <thead>
                        <tr>
                            <th data-translate-key="benchmark-set">Benchmark Set</th>
                            <th data-translate-key="test-item">Test Item</th>
                            <th data-translate-key="baseline-median">Baseline Median (ms)</th>
                            <th data-translate-key="new-median">New Median (ms)</th>
                            <th data-translate-key="change">Change %</th>
                            <th data-translate-key="baseline-avg">Baseline Avg (ms)</th>
                            <th data-translate-key="new-avg">New Avg (ms)</th>
                            <th data-translate-key="new-max">New Max (ms)</th>
                            <th data-translate-key="new-min">New Min (ms)</th>
                        </tr>
                    </thead>
                    <tbody>
                        {''.join(rows)}
                    </tbody>
                </table>
            </div>
        </div>
        """

        return table_html

    def prepare_chart_data(self):
        """Prepare chart data for JavaScript"""
        if not self.comparison_data:
            return {
                'summary': {
                    'distribution': [0, 0, 0]
                },
                'chartData': {
                    'testNames': [],
                    'improvements': [],
                    'colors': [],
                    'comparisonChartData': {'labels': [], 'datasets': []},
                    'radarChartData': {'labels': [], 'datasets': []}
                }
            }

        # Sort by change percentage magnitude
        sorted_data = sorted(self.comparison_data, key=lambda x: abs(x['change_percent']), reverse=True)

        # Get top 15 test items for bar chart
        top_tests = sorted_data[:15]

        # Create labels with benchmark set name
        test_names = [f"{d['benchmark_set']}: {d['test_item']}" for d in top_tests]
        improvements = [d['change_percent'] for d in top_tests]
        colors = []

        for improvement in improvements:
            if improvement < -5:
                colors.append('rgba(72, 187, 120, 0.8)')  # Dark green - big improvement
            elif improvement < -0.1:
                colors.append('rgba(134, 239, 172, 0.8)')  # Light green - small improvement
            elif improvement > 5:
                colors.append('rgba(239, 68, 68, 0.8)')    # Dark red - big regression
            elif improvement > 0.1:
                colors.append('rgba(252, 165, 165, 0.8)')  # Light red - small regression
            else:
                colors.append('rgba(148, 163, 184, 0.8)')  # Gray - unchanged

        # Median comparison line chart data
        comparison_labels = [f"{d['benchmark_set']}: {d['test_item']}" for d in sorted_data[:10]]
        baseline_mid_values = [d['baseline_mid_ms'] for d in sorted_data[:10]]
        new_mid_values = [d['new_mid_ms'] for d in sorted_data[:10]]

        comparison_chart_data = {
            'labels': comparison_labels,
            'datasets': [
                {
                    'label': 'Baseline Data (mid_ms)',
                    'data': baseline_mid_values,
                    'borderColor': 'rgba(76, 201, 240, 1)',
                    'backgroundColor': 'rgba(76, 201, 240, 0.2)',
                    'tension': 0.3
                },
                {
                    'label': 'New Test Data (mid_ms)',
                    'data': new_mid_values,
                    'borderColor': 'rgba(245, 158, 11, 1)',
                    'backgroundColor': 'rgba(245, 158, 11, 0.2)',
                    'tension': 0.3
                }
            ]
        }

        # Radar chart data (average time comparison)
        radar_labels = [f"{d['benchmark_set']}: {d['test_item']}" for d in sorted_data[:8]]
        baseline_avg_values = [d['baseline_avg_ms'] for d in sorted_data[:8]]
        new_avg_values = [d['new_avg_ms'] for d in sorted_data[:8]]

        radar_chart_data = {
            'labels': radar_labels,
            'datasets': [
                {
                    'label': 'Baseline Data (avg_ms)',
                    'data': baseline_avg_values,
                    'borderColor': 'rgba(76, 201, 240, 1)',
                    'backgroundColor': 'rgba(76, 201, 240, 0.2)',
                    'pointBackgroundColor': 'rgba(76, 201, 240, 1)'
                },
                {
                    'label': 'New Test Data (avg_ms)',
                    'data': new_avg_values,
                    'borderColor': 'rgba(245, 158, 11, 1)',
                    'backgroundColor': 'rgba(245, 158, 11, 0.2)',
                    'pointBackgroundColor': 'rgba(245, 158, 11, 1)'
                }
            ]
        }

        return {
            'summary': {
                'distribution': [
                    self.summary_stats['improved'],
                    self.summary_stats['regressed'],
                    self.summary_stats['unchanged']
                ]
            },
            'chartData': {
                'testNames': test_names,
                'improvements': improvements,
                'colors': colors,
                'comparisonChartData': comparison_chart_data,
                'radarChartData': radar_chart_data
            }
        }

    def generate_html_report(self, output_file="performance_report.html"):
        """Generate HTML report"""
        # Load and compare data
        if not self.load_json_files():
            print("No benchmark data found. Please check if benchmark directory exists and contains JSON files.")
            return None

        self.compare_performance()

        # Prepare chart data
        chart_data = self.prepare_chart_data()

        # Generate HTML
        html_content = HTML_TEMPLATE
        html_content = html_content.replace("{{timestamp}}", datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        html_content = html_content.replace("{{summary_section}}", self.generate_summary_section())
        html_content = html_content.replace("{{comparison_table}}", self.generate_comparison_table())
        html_content = html_content.replace("{{performance_data}}", json.dumps(chart_data))

        # Write to file
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(html_content)

        print(f"Performance report generated: {output_file}")
        print(f"Total benchmark sets: {self.summary_stats['total_sets']}")
        print(f"Total test items: {self.summary_stats['total_tests']}")
        print(f"Improved: {self.summary_stats['improved']}")
        print(f"Regressed: {self.summary_stats['regressed']}")
        print(f"Unchanged: {self.summary_stats['unchanged']}")

        return output_file


# Main program
if __name__ == "__main__":
    # Set benchmark directory
    benchmark_dir = "benchmark"

    # Check if benchmark directory exists
    if not os.path.exists(benchmark_dir):
        print(f"Error: {benchmark_dir} directory does not exist.")
        print(f"Please create a '{benchmark_dir}' directory in the current working directory and add JSON files.")
        print("Expected file structure:")
        print(f"  {benchmark_dir}/")
        print(f"    ├── benchmark_set_1.json        (baseline data)")
        print(f"    ├── benchmark_set_1_new.json    (new test data, optional)")
        print(f"    ├── benchmark_set_2.json        (baseline data)")
        print(f"    └── benchmark_set_2_new.json    (new test data, optional)")
        exit(1)

    # Generate performance report
    analyzer = PerformanceAnalyzer(benchmark_dir)
    report_file = analyzer.generate_html_report()

    if report_file:
        # Instructions to open report
        print("\nUse the following commands to open the report:")
        print(f"  - Windows: start {report_file}")
        print(f"  - macOS: open {report_file}")
        print(f"  - Linux: xdg-open {report_file}")
