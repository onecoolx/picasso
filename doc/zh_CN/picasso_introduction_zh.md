# Picasso 矢量图形库

> 版本: 2.9.0 | 许可证: BSD 2-Clause | 语言: C++98 + C API  
> 作者: Zhang Ji Peng (onecoolx@gmail.com)  
> 仓库: https://github.com/onecoolx/picasso

---

## 目录

1. [项目概述](#1-项目概述)
2. [整体架构](#2-整体架构)
3. [目录结构](#3-目录结构)
4. [核心引擎 (src/)](#4-核心引擎)
5. [渲染管线](#5-渲染管线)
6. [扩展子系统 (ext/)](#6-扩展子系统)
7. [字体系统](#7-字体系统)
8. [构建系统](#8-构建系统)
9. [测试体系](#9-测试体系)
10. [WebAssembly 支持](#10-webassembly-支持)
11. [工程约束与编码规范](#11-工程约束与编码规范)
12. [快速上手](#12-快速上手)
13. [API 参考索引](#13-api-参考索引)

---

## 1. 项目概述

Picasso 是一个高质量、低内存占用的 2D 矢量图形渲染库。它提供了一套完整的高层 2D 图形 API，可用于 GUI 系统、PostScript 渲染、SVG 图像渲染等场景。

### 核心特性

- **路径操作**: 填充、描边、裁剪、布尔运算
- **变换**: 仿射矩阵变换（平移、旋转、缩放、倾斜）
- **填充样式**: 纯色、线性/径向/锥形渐变、图案填充
- **描边样式**: 虚线、线帽（Butt/Round/Square）、连接方式（Miter/Round/Bevel）
- **图像处理**: 多格式解码（JPEG、PNG、WebP、GIF）、图像合成
- **文本渲染**: TrueType 字体、字距调整、抗锯齿
- **效果**: 模糊、阴影、Gamma 校正、Alpha 合成
- **SVG**: SVG Tiny 1.2 解析与渲染、SVG 动画播放
- **像素格式**: RGBA、ARGB、ABGR、BGRA、RGB、BGR、RGB565、RGB555、A8

### 平台支持

| 平台 | 构建工具 | 字体后端 | 状态 |
|------|---------|---------|------|
| Linux (x86/ARM) | CMake + Make | FreeType2 + FontConfig | ✅ |
| macOS | CMake + Xcode | CoreText | ✅ |
| Windows | CMake + Visual Studio | GDI | ✅ |
| Android | CMake + NDK | FreeType2 | ✅ |
| WebAssembly | Emscripten | — | ✅ |

### 版本历史摘要

| 版本 | 日期 | 里程碑 |
|------|------|--------|
| 1.2.0 | 2011-05 | 首个 GPL-v2 版本 |
| 2.0.0 | 2013-02 | 架构重写，切换至 BSD 许可证 |
| 2.1.5 | 2015-03 | Android 移植 |
| 2.2.0 | 2016-02 | 图像编解码扩展模块 |
| 2.5.0 | 2023-02 | 完整 macOS 支持 |
| 2.8.0 | 2024-07 | 全面 CMake 构建 |
| 2.9.0 | 当前 | SVG Tiny 1.2 动画支持 |

---

## 2. 整体架构

Picasso 采用分层架构设计，从上到下分为公共 API 层、核心引擎层、图形后端层和扩展子系统。

```
┌─────────────────────────────────────────────────────────┐
│                    应用程序代码                           │
├─────────────────────────────────────────────────────────┤
│                  公共 C API (include/)                    │
│   ps_context  ps_canvas  ps_path  ps_gradient  ps_font  │
├──────────────────────┬──────────────────────────────────┤
│   核心引擎 (src/)     │      扩展子系统 (ext/)            │
│  ┌────────────────┐  │  ┌─────────────┐ ┌────────────┐ │
│  │ 图形对象管理    │  │  │ SVG 子系统   │ │ 图像编解码  │ │
│  │ 状态栈管理      │  │  │ 解析/渲染    │ │ JPEG/PNG   │ │
│  │ 路径/矩阵/裁剪  │  │  │ 动画播放器   │ │ WebP/GIF   │ │
│  └────────────────┘  │  └─────────────┘ └────────────┘ │
├──────────────────────┴──────────────────────────────────┤
│              图形后端 (src/gfx/)                          │
│  光栅化器 → 扫描线渲染 → 像素混合 → 合成 → 渲染缓冲区    │
├─────────────────────────────────────────────────────────┤
│              基础设施 (src/include/, src/core/)           │
│  内存管理  数学类型  矩阵运算  路径数据  曲线处理  裁剪器  │
└─────────────────────────────────────────────────────────┘
```

### 设计原则

1. **C++98 严格兼容**: 不使用 STL 容器、lambda、auto 等 C++11+ 特性
2. **显式内存管理**: 使用项目内部分配器 (`mem_malloc`/`mem_free`)，无隐式所有权
3. **引用计数**: 所有不透明对象通过引用计数管理生命周期
4. **模块化扩展**: SVG 和图像编解码为可选的插件式子系统
5. **性能优先**: SIMD 优化、扫描线光栅化、高效像素混合
6. **多格式支持**: 灵活的像素格式处理，每种格式有专门优化
7. **平台抽象**: 跨 Windows、macOS、Linux、Android、WebAssembly 的统一 API

---

## 3. 目录结构

```
picasso/
├── include/                  # 公共 API 头文件
│   ├── picasso.h             # 核心 API（上下文、画布、路径、渐变等）
│   ├── picasso_ext.h         # 扩展公共定义（结果码）
│   ├── picasso_backport.h    # 内存分配器自定义接口
│   ├── images/               # 图像扩展 API
│   │   ├── psx_image.h       # 图像加载/保存接口
│   │   └── psx_image_plugin.h # 图像编解码插件接口
│   └── svg/                  # SVG 扩展 API
│       ├── psx_svg.h         # SVG 文档加载与渲染
│       └── psx_svg_animation.h # SVG 动画播放器
├── src/                      # 核心引擎实现
│   ├── include/              # 内部头文件（数据结构、接口定义）
│   ├── core/                 # 基础组件（路径、矩阵、裁剪、内存）
│   ├── gfx/                  # 图形后端（光栅化、合成、像素格式）
│   ├── font/                 # 字体适配器（平台相关）
│   ├── simd/                 # SIMD 优化实现
│   └── picasso_*.cpp/h       # API 实现与内部对象
├── ext/                      # 扩展子系统
│   ├── common/               # 扩展公共工具
│   ├── image_coders/         # 图像编解码器
│   │   ├── jpeg/png/gif/webp/apple/  # 各格式实现
│   │   └── psx_image_*.c/h   # 加载器、模块管理
│   └── svg/                  # SVG 子系统
│       ├── psx_svg_parser.cpp # XML 解析器
│       ├── psx_svg_render.cpp # SVG 渲染器
│       ├── psx_svg_player.cpp # 动画播放器
│       ├── psx_svg_node.cpp/h # 节点树
│       └── psx_xml_token.cpp/h # XML 词法分析
├── third_party/              # 第三方依赖
│   ├── zlib-1.2.8/           # 压缩库
│   ├── libpng-1.6.17/        # PNG 编解码
│   ├── libjpeg-turbo-1.4.1/  # JPEG 编解码
│   ├── giflib-5.1.3/         # GIF 编解码
│   ├── libwebp-0.5.1/        # WebP 编解码
│   ├── freetype-2.3.6/       # 字体光栅化
│   └── expat-2.1.0/          # XML 解析（Android）
├── unit_tests/               # GoogleTest 单元测试
├── perf_tests/               # 性能基准测试
├── test/                     # 平台 GUI 测试应用
├── demos/                    # 示例程序
├── wasm/                     # WebAssembly 构建
├── tools/                    # 开发工具脚本
├── cfg/                      # 配置文件（字体等）
├── .design_docs/             # 设计文档
├── build/                    # CMake 构建配置
└── CMakeLists.txt            # 根构建文件
```


---

## 4. 核心引擎

核心引擎位于 `src/` 目录，负责图形对象管理、状态栈、路径运算和渲染调度。

### 4.1 公共 API 对象 (`include/picasso.h`)

所有公共对象均为不透明类型（opaque type），通过 C 函数接口操作，使用引用计数管理生命周期。

| 对象类型 | 说明 | 创建/销毁 |
|---------|------|----------|
| `ps_context` | 绘图上下文，持有状态栈和画布引用 | `ps_context_create` / `ps_context_unref` |
| `ps_canvas` | 像素缓冲区，渲染目标 | `ps_canvas_create*` / `ps_canvas_unref` |
| `ps_image` | 图像对象 | `ps_image_create*` / `ps_image_unref` |
| `ps_path` | 图形路径（贝塞尔曲线、直线、弧线） | `ps_path_create` / `ps_path_unref` |
| `ps_matrix` | 仿射变换矩阵 | `ps_matrix_create*` / `ps_matrix_unref` |
| `ps_gradient` | 渐变（线性、径向、锥形） | `ps_gradient_create_*` / `ps_gradient_unref` |
| `ps_pattern` | 图案填充 | `ps_pattern_create*` / `ps_pattern_unref` |
| `ps_mask` | Alpha 蒙版 | `ps_mask_create` / `ps_mask_unref` |
| `ps_font` | 字体对象 | `ps_font_create*` / `ps_font_unref` |

### 4.2 内部对象 (`src/picasso_objects.h`)

内部 C++ 类封装了绘图状态的各个方面：

- **`graphic_pen`**: 描边样式 — 颜色/渐变/图案/图像填充、线宽、线帽、连接方式、虚线模式
- **`graphic_brush`**: 填充样式 — 颜色/渐变/图案/图像填充、填充规则
- **`shadow_state`**: 阴影效果 — 偏移、模糊半径、颜色
- **`clip_area`**: 裁剪区域 — 路径裁剪或矩形裁剪
- **`context_state`**: 绘图状态 — 矩阵、画笔、画刷、裁剪、字体、透明度、Gamma、模糊、合成模式

`context_state` 支持保存/恢复（`ps_save`/`ps_restore`），形成状态栈。

### 4.3 基础设施 (`src/include/`, `src/core/`)

| 文件 | 职责 |
|------|------|
| `memory_manager.h/cpp` | 自定义内存分配器 (`mem_malloc`/`mem_free`) |
| `math_type.h` | 数学类型定义（`scalar` 类型、定点数支持） |
| `matrix.h` / `matrix.cpp` | 仿射变换矩阵 (`trans_affine`) |
| `graphic_path.h/cpp` | 路径数据结构与操作 |
| `curve.h/cpp` | 贝塞尔曲线处理（二次/三次曲线展平） |
| `clipper.h/cpp` | 路径布尔裁剪运算 |
| `data_vector.h` | 项目内部动态数组（替代 `std::vector`） |
| `color_type.h` | 颜色类型定义 (`rgba`) |
| `geometry.h` | 几何工具函数 |
| `interfaces.h` | 抽象接口定义（设备抽象层） |


---

## 5. 渲染管线

渲染管线位于 `src/gfx/`，是 Picasso 的性能核心。采用扫描线光栅化架构。

### 5.1 管线流程

```
用户绘图调用 (ps_fill / ps_stroke)
        │
        ▼
┌─────────────────┐
│  路径处理        │  graphic_path → 变换 → 描边/填充展开
│  (raster_adapter)│
└────────┬────────┘
         ▼
┌─────────────────┐
│  光栅化          │  路径 → 单元格 → 扫描线
│  (rasterizer)    │  gfx_rasterizer_cell → gfx_rasterizer_scanline
└────────┬────────┘
         ▼
┌─────────────────┐
│  Span 生成       │  根据填充类型生成像素 span
│  (span_generator)│  纯色 / 渐变 / 图案 / 图像
└────────┬────────┘
         ▼
┌─────────────────┐
│  合成与混合      │  Alpha 合成、混合模式
│  (composite)     │  gfx_composite_packed / gfx_blender_packed
└────────┬────────┘
         ▼
┌─────────────────┐
│  像素格式写入    │  格式特定的像素操作
│  (pixfmt)        │  gfx_pixfmt_rgba / rgb / rgb16 / gray
└────────┬────────┘
         ▼
┌─────────────────┐
│  渲染缓冲区      │  最终像素输出
│  (rendering_buf) │  ps_canvas 的底层存储
└─────────────────┘
```

### 5.2 关键组件

#### 光栅化器
- **`gfx_rasterizer_cell.h`**: 单元格级光栅化，将路径分解为覆盖单元格
- **`gfx_rasterizer_scanline.h`**: 扫描线光栅化器，将单元格转换为扫描线数据
- **`gfx_scanline.h`**: 扫描线数据结构
- **`gfx_scanline_storage.h`**: 扫描线存储（用于缓存光栅化结果）

#### 渲染器
- **`gfx_renderer.h`**: 基础渲染原语
- **`gfx_scanline_renderer.h`**: 扫描线渲染器，将扫描线数据写入像素缓冲区
- **`gfx_painter.h`**: 高层画家接口，协调整个渲染流程

#### Span 生成器
- **`gfx_span_generator.h`**: Span 生成框架
- **`gfx_span_image_filters.h`**: 图像过滤 span（缩放、插值）
- **`gfx_gradient_adapter.h/cpp`**: 渐变渲染适配器

#### 像素格式处理
- **`gfx_pixfmt_rgba.h`**: 32 位 RGBA/ARGB/ABGR/BGRA
- **`gfx_pixfmt_rgb.h`**: 24 位 RGB/BGR
- **`gfx_pixfmt_rgb16.h`**: 16 位 RGB565/RGB555
- **`gfx_pixfmt_gray.h`**: 8 位灰度 (A8)
- **`gfx_pixfmt_wrapper.h`**: 像素格式抽象包装

#### 合成与混合
- **`gfx_composite_packed.h`**: Porter-Duff 合成操作
- **`gfx_blender_packed.h`**: 像素级混合操作

#### 效果
- **`gfx_blur.h/cpp`**: 高斯模糊实现
- **`gfx_image_filters.h/cpp`**: 图像过滤器（双线性、双三次插值等）
- **`gfx_gamma_function.h`**: Gamma 校正函数
- **`gfx_mask_layer.h`**: Alpha 蒙版层

### 5.3 画家类 (`src/picasso_painter.h`)

`painter` 类是渲染管线的入口，提供以下渲染操作：

```cpp
class painter {
    void render_stroke(context_state*, raster_adapter&, const graphic_path&);
    void render_fill(context_state*, raster_adapter&, const graphic_path&);
    void render_paint(context_state*, raster_adapter&, const graphic_path&);
    void render_clear(context_state*);
    void render_blur(context_state*);
    void render_gamma(context_state*, raster_adapter&);
    void render_clip(context_state*, bool clip);
    void render_shadow(context_state*, const graphic_path&, bool fill, bool stroke);
    void render_mask(const mask_layer&, bool mask);
    void render_copy(rendering_buffer& src, const rect*, const painter* dst, int32_t, int32_t);
    void render_glyph(context_state*, raster_adapter&, const font*, int32_t type);
};
```


---

## 6. 扩展子系统

扩展子系统位于 `ext/`，包含图像编解码和 SVG 两个主要模块，通过 `OPT_EXTENSIONS` CMake 选项控制是否编译。

### 6.1 图像编解码 (`ext/image_coders/`)

#### 架构

图像系统采用插件式架构，通过签名检测自动识别图像格式：

```
psx_image_load()
      │
      ▼
┌──────────────┐
│ 签名检测      │  读取文件头字节，匹配已注册编解码器
└──────┬───────┘
       ▼
┌──────────────┐
│ 编解码器选择  │  按优先级选择匹配的 psx_image_operator
└──────┬───────┘
       ▼
┌──────────────┐
│ 解码执行      │  read_header_info → decode_image_data → release
└──────┬───────┘
       ▼
   psx_image (多帧支持)
```

#### 插件接口 (`psx_image_plugin.h`)

每个编解码器实现 `psx_image_operator` 结构体：

```c
typedef struct _psx_image_operator {
    int32_t (*read_header_info)(...);       // 读取图像头信息
    int32_t (*decode_image_data)(...);      // 解码帧数据
    int32_t (*release_read_header_info)(...); // 释放读取资源
    int32_t (*write_header_info)(...);      // 创建写入头
    int32_t (*encode_image_data)(...);      // 编码帧数据
    int32_t (*release_write_header_info)(...); // 释放写入资源
} psx_image_operator;
```

注册时指定格式名称、文件签名和优先级：
- `PRIORITY_MASTER` (1): 最高优先级
- `PRIORITY_DEFAULT` (0): 默认优先级
- `PRIORITY_EXTENTED` (-1): 扩展优先级

#### 支持的格式

| 格式 | 目录 | 编码 | 解码 | 多帧 |
|------|------|------|------|------|
| JPEG | `jpeg/` | ✅ | ✅ | — |
| PNG | `png/` | ✅ | ✅ | — |
| WebP | `webp/` | ✅ | ✅ | — |
| GIF | `gif/` | — | ✅ | ✅ (含帧时长) |
| Apple (macOS) | `apple/` | ✅ | ✅ | — |

### 6.2 SVG 子系统 (`ext/svg/`)

SVG 子系统实现了 SVG Tiny 1.2 规范的解析、渲染和动画播放。

#### 架构

```
SVG 文件/内存数据
      │
      ▼
┌──────────────┐
│ XML 词法分析  │  psx_xml_token.cpp — 将 XML 文本分解为 token
└──────┬───────┘
       ▼
┌──────────────┐
│ SVG 解析器    │  psx_svg_parser.cpp — 构建节点树，提取属性
└──────┬───────┘
       ▼
┌──────────────┐
│ SVG 节点树    │  psx_svg_node.cpp — 树形文档结构
└──────┬───────┘
       │
       ├──────────────────────┐
       ▼                      ▼
┌──────────────┐      ┌──────────────┐
│ SVG 渲染器    │      │ 动画播放器    │
│ psx_svg_render│      │ psx_svg_player│
│ 静态渲染      │      │ 动画状态管理  │
└──────────────┘      └──────────────┘
```

#### SVG 渲染器 (`psx_svg_render.cpp`)

- 遍历节点树，将 SVG 元素转换为 Picasso 绘图调用
- 支持的元素：`<rect>`, `<circle>`, `<ellipse>`, `<line>`, `<polyline>`, `<polygon>`, `<path>`, `<text>`, `<image>`, `<g>`, `<use>`, `<defs>`, `<linearGradient>`, `<radialGradient>`
- 动画模式下查询 `anim_state` 获取覆盖值，非动画节点直接跳过

#### SVG 动画播放器 (`psx_svg_player.cpp`)

动画播放器是最近开发的核心功能，实现了 SVG Tiny 1.2 动画规范约 97% 的覆盖率。

##### 支持的动画元素

| 元素 | 说明 |
|------|------|
| `<animate>` | 数值属性插值（float/int32） |
| `<set>` | 离散属性赋值（枚举、可见性、字体） |
| `<animateColor>` | RGB 颜色插值（fill、stroke） |
| `<animateTransform>` | 变换插值（translate/scale/rotate/skewX/skewY/matrix） |
| `<animateMotion>` | 路径跟随运动，支持自动旋转 |

##### 时序模型

- **开始/结束**: 偏移列表、事件驱动、同步基准、accessKey、indefinite
- **持续与重复**: `dur`、`repeatCount`（含小数）、`repeatDur`、`min`/`max` 约束
- **填充模式**: `remove`（默认）、`freeze`
- **重启策略**: `always`、`whenNotActive`、`never`

##### 插值模式 (`calcMode`)

- `linear`（默认）、`discrete`、`spline`（三次贝塞尔）、`paced`（弧长参数化）

##### 合成模型

- `additive="sum"` — 数值叠加或矩阵合成
- `accumulate="sum"` — 跨重复周期累积
- `animateTransform` 和 `animateMotion` 作为独立层合成

##### 动画状态 (`psx_svg_anim_state.h`)

```
psx_svg_anim_state
├── overrides[]          — 数值覆盖 (float/int32)
├── color_overrides[]    — 颜色覆盖 (fill/stroke)
├── transforms[]         — animateTransform 结果矩阵
├── motion_transforms[]  — animateMotion 结果矩阵
├── dash_overrides[]     — stroke-dasharray 数组
└── active_targets[]     — 活跃目标节点（渲染器快速跳过）
```

最终动画变换 = `motion_transform × animateTransform_result`

##### 性能优化

| 优化项 | 效果 |
|--------|------|
| SBO (Small Buffer Optimization) | 消除每个动画项 2 次堆分配 |
| 追加式写入 + 后排序去重 | 消除 O(n²) 每帧写入扫描 |
| 活跃目标快速跳过 | 跳过约 80% 非动画节点的无效查找 |
| Dash 缓冲区跨帧复用 | 消除每帧 malloc/free |
| 叠加基值预缓存 | 消除每帧 `_find_attr` 扫描 |
| 同步基准 ID 查找表 | O(log n) 二分查找替代 O(n) strcmp |
| 时间窗口快速剔除 | 跳过已过期的 fill=remove 动画 |


---

## 7. 字体系统

字体系统位于 `src/font/`，采用平台抽象设计。

### 架构

```
ps_font (公共 API)
    │
    ▼
font_engine (字体引擎)
    │  管理字体池（最多 MAX_FONTS=16 个缓存字体）
    │  签名匹配，避免重复创建
    ▼
font (字体实例)
    ├── font_desc — 字体描述（名称、大小、粗细、斜体等）
    ├── font_adapter — 平台适配器（实际字体加载）
    ├── glyph_cache_manager — 字形缓存
    └── mono_storage — 单色字形存储
```

### 平台适配

| 平台 | 适配器文件 | 字体后端 |
|------|-----------|---------|
| macOS | `font_adapter_apple.mm` | CoreText |
| Linux | `font_adapter_freetype.cpp` | FreeType2 + FontConfig |
| Windows | `font_adapter_win32.cpp` | GDI |
| Android | `font_adapter_freetype.cpp` | FreeType2 |

### 字形缓存

`glyph_cache_manager` 通过字体签名（包含名称、大小、变换矩阵等）索引缓存的字形数据，避免重复光栅化。每个字形包含：
- 路径数据（矢量轮廓）
- 光栅化位图（单色或抗锯齿）
- 度量信息（advance、bounds）

---

## 8. 构建系统

### CMake 配置

根 `CMakeLists.txt` 定义了所有构建选项：

#### 核心选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `BUILD_SHARED_LIBS` | ON | 构建共享库 |
| `OPT_EXTENSIONS` | ON | 编译扩展子系统（SVG + 图像） |
| `OPT_FREE_TYPE2` | OFF | 启用 FreeType2 字体支持 |
| `OPT_FONT_CONFIG` | OFF | 启用 FontConfig 字体查找 |
| `OPT_FAST_COPY` | OFF | 快速内存拷贝优化 |
| `OPT_LOW_MEMORY` | OFF | 低内存模式 |
| `OPT_SYSTEM_MALLOC` | OFF | 使用系统内存分配器 |

#### 像素格式选项

所有 `OPT_FORMAT_*` 选项默认开启：ABGR、ARGB、BGRA、RGBA、RGB、BGR、RGB565、RGB555、A8。

#### 测试与调试选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `OPT_TESTS` | ON | 编译测试应用 |
| `OPT_DEMOS` | ON | 编译示例程序 |
| `OPT_UNITTEST` | OFF | 编译 GoogleTest 单元测试 |
| `OPT_PERFTEST` | OFF | 编译性能测试 |
| `OPT_SANITIZE` | OFF | 启用 AddressSanitizer |
| `OPT_COVERAGE` | OFF | 启用代码覆盖率 |

#### 第三方库选项

`OPT_SYSTEM_GIF/JPEG/PNG/WEBP/ZLIB` — 使用系统库或内置第三方库。

### 构建产物

| 产物 | 说明 |
|------|------|
| `libpicasso2_sw.so` | 核心图形库 |
| `libpsx_image.so` | 图像编解码扩展库 |
| `libpsx_svg.so` | SVG 扩展库（动态） |
| `libpsx_svg_static.a` | SVG 扩展库（静态，用于单元测试） |
| `libpsx_common.a` | 扩展公共工具库 |
| `libpsxm_image_*.so` | 图像编解码插件模块 |
| `unit_tests` | 单元测试可执行文件 |

### 各平台构建步骤

#### Linux
```bash
./build_linux.sh          # 生成 proj/ 目录
cd proj && make -j$(nproc)
```

#### macOS
```bash
./build_macosx.sh         # 生成 Xcode 项目
cd proj
# 用 Xcode 打开 picasso.xcodeproj
```

#### Windows
```bat
build_windows.bat         # 生成 Visual Studio 解决方案
cd proj
# 用 Visual Studio 打开 picasso.sln
```

#### WebAssembly
```bash
cd wasm
./build_wasm.sh           # 使用 Emscripten 编译
# 产出: picasso.wasm + lib.js
```


---

## 9. 测试体系

### 9.1 单元测试 (`unit_tests/`)

- **框架**: GoogleTest v1.12.1（通过 CMake FetchContent 自动下载）
- **快照比对**: 使用 lodepng 进行 PNG 快照对比测试
- **测试规模**: 864+ 测试用例（含 180 个 SVG 动画测试）

#### 测试文件组织

| 文件模式 | 覆盖范围 |
|---------|---------|
| `ps_*_api.cpp` | 核心 API 测试（上下文、画布、路径、矩阵、渐变等） |
| `ps_*_test.cpp` | 核心功能测试（像素格式、合成、渲染、模糊等） |
| `ext_svg*.cpp` | SVG 扩展测试（解析、渲染、动画播放器） |
| `ext_image.cpp` | 图像编解码测试 |
| `ext_*.cpp` | 其他扩展测试（数组、树、链表、线性分配器） |

#### 运行测试

```bash
# 在 WSL 中运行（项目约定）
wsl -e bash -c "cd /home/jipeng/picasso/proj && ./unit_tests"

# 聚焦运行 SVG 动画测试
wsl -e bash -c "cd /home/jipeng/picasso/proj && ./unit_tests --gtest_filter='SVGPlayerTest.*'"
```

### 9.2 GUI 测试应用 (`test/`)

平台特定的交互式测试程序，每个 `*_func.c` 文件对应一个测试场景：

| 测试 | 文件 | 验证内容 |
|------|------|---------|
| alpha | `alpha_func.c` | Alpha 混合 |
| bitblt | `bitblt_func.c` | 像素块传输 |
| blur | `blur_func.c` | 模糊效果 |
| clip | `clip_func.c` | 路径裁剪 |
| composite | `composite_func.c` | 合成模式 |
| gamma | `gamma_func.c` | Gamma 校正 |
| gcstate | `gcstate_func.c` | 图形状态管理 |
| gradient | `gradient_func.c` | 渐变渲染 |
| mask | `mask_func.c` | 蒙版操作 |
| path | `path_func.c` | 路径操作 |
| pattern | `pattern_func.c` | 图案填充 |
| shadow | `shadow_func.c` | 阴影效果 |
| text | `text_func.c` | 文本渲染 |
| thread | `thread_func.c` | 多线程安全 |

平台入口：
- Linux: `gtk2/testGtk2.c` (GTK2)
- macOS: `mac/testMac.m` (Cocoa)
- Windows: `win32/testWin.c` (Win32 API)
- Android: `android/testAndroid.cpp` (NDK)

### 9.3 示例程序 (`demos/`)

| 示例 | 文件 | 展示内容 |
|------|------|---------|
| clock | `clock.c` | 模拟时钟 — 弧线、渐变、仿射变换 |
| flowers | `flowers.c` | 花朵 — Alpha 混合、多层叠加 |
| tiger | `tiger.c` | SVG 老虎 — 复杂路径和渐变渲染 |
| subwaymap | `subwaymap.c` | 地铁图 — 高密度折线和标签渲染 |

### 9.4 性能测试 (`perf_tests/`)

基准测试套件，覆盖核心操作的性能验证：

| 测试 | 文件 |
|------|------|
| 路径绘制 | `ps_path_drawing_test.cpp` |
| 矩阵运算 | `ps_matrix_test.cpp` |
| 渐变渲染 | `ps_gradient_test.cpp` |
| 图案填充 | `ps_pattern_test.cpp` |
| 图像操作 | `ps_image_test.cpp` |
| 蒙版操作 | `ps_mask_test.cpp` |
| 画布操作 | `ps_canvas_test.cpp` |
| 上下文操作 | `ps_context_test.cpp` |
| 字体渲染 | `ps_font_test.cpp` |
| SVG 解析 | `ext_svg_parser_test.cpp` |
| 复杂绘制 | `ps_complex_draw_test.cpp` |

---

## 10. WebAssembly 支持

Picasso 通过 Emscripten 编译为 WebAssembly，位于 `wasm/` 目录。

### 构建流程

```bash
# 1. emmake 编译静态库
emmake make -f Makefile.em

# 2. emcc 链接为 .wasm
emcc --no-entry \
  -s"ALLOW_MEMORY_GROWTH=1" \
  -s"EXPORTED_FUNCTIONS=@export.list" \
  -s"EXPORTED_RUNTIME_METHODS=@runtime.list" \
  libpicasso.a -o picasso.wasm

# 3. 清理
emmake make -f Makefile.em clean
```

### 关键文件

| 文件 | 说明 |
|------|------|
| `Makefile.em` | Emscripten 构建规则 |
| `export.list` | 导出的 C 函数列表 |
| `runtime.list` | 导出的运行时方法（malloc、free 等） |
| `lib.js` | JavaScript 绑定层 |
| `pconfig.h` | WASM 平台配置 |
| `test.html` | 浏览器测试页面 |
| `picasso.wasm` | 编译产物 |


---

## 11. 工程约束与编码规范

### 语言约束

- **C++98 严格模式**: 不使用 C++11 及以后的特性
- **禁止 STL 容器**: 不使用 `std::vector`、`std::map` 等，使用项目内部 `data_vector`
- **禁止 lambda/闭包**: 使用函数指针和回调
- **显式内存管理**: 使用 `mem_malloc`/`mem_free`，避免隐式所有权转移

### 内存管理

```cpp
// 项目内部分配器
void* mem_malloc(size_t size);
void  mem_free(void* ptr);

// 可通过 picasso_backport.h 自定义
typedef void* (*ps_malloc_func)(size_t);
typedef void  (*ps_free_func)(void*);
typedef void* (*ps_calloc_func)(size_t, size_t);
```

### 引用计数模式

所有公共对象遵循统一的引用计数模式：

```c
ps_object* obj = ps_object_create(...);  // refcount = 1
ps_object_ref(obj);                       // refcount = 2
ps_object_unref(obj);                     // refcount = 1
ps_object_unref(obj);                     // refcount = 0, 释放
```

---

## 12. 快速上手

### 最小示例

```c
#include "picasso.h"

int main(void)
{
    /* 1. 初始化 */
    ps_initialize();

    /* 2. 创建画布 */
    int width = 400, height = 300, stride = width * 4;
    unsigned char* buf = (unsigned char*)malloc(stride * height);
    memset(buf, 0, stride * height);
    ps_canvas* canvas = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA,
                                                   width, height, stride);

    /* 3. 创建绘图上下文 */
    ps_context* ctx = ps_context_create(canvas, NULL);

    /* 4. 绘制渐变矩形 */
    ps_point center = { 200.0f, 150.0f };
    ps_gradient* grad = ps_gradient_create_radial(GRADIENT_SPREAD_PAD,
                                                  &center, 0.0f, &center, 180.0f);
    ps_color c0 = { 0.2f, 0.6f, 1.0f, 1.0f };
    ps_color c1 = { 0.0f, 0.1f, 0.4f, 1.0f };
    ps_gradient_add_color_stop(grad, 0.0f, &c0);
    ps_gradient_add_color_stop(grad, 1.0f, &c1);

    ps_rect rc = { 40.0f, 30.0f, 320.0f, 240.0f };
    ps_rounded_rect(ctx, &rc, 16, 16, 16, 16, 16, 16, 16, 16);
    ps_set_source_gradient(ctx, grad);
    ps_fill(ctx);

    /* 5. 释放资源 */
    ps_gradient_unref(grad);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    free(buf);
    ps_shutdown();
    return 0;
}
```

### SVG 渲染示例

```c
#include "picasso.h"
#include "psx_svg.h"

// 加载并渲染 SVG
psx_svg* doc = psx_svg_load("tiger.svg", NULL);
psx_svg_render* render = psx_svg_render_create(doc, NULL);

ps_size size;
psx_svg_render_get_size(render, &size);
// 创建画布和上下文...
psx_svg_render_draw(ctx, render);

psx_svg_render_destroy(render);
psx_svg_destroy(doc);
```

### SVG 动画播放示例

```c
#include "psx_svg_animation.h"

psx_svg* doc = psx_svg_load("animation.svg", NULL);
psx_svg_player* player = psx_svg_player_create(doc, NULL);

psx_svg_player_play(player);

// 游戏循环中
float dt = 0.016f; // 16ms
psx_svg_player_tick(player, dt);
psx_svg_player_draw(player, ctx);

psx_svg_player_destroy(player);
psx_svg_destroy(doc);
```

---

## 13. API 参考索引

完整 API 文档: http://onecoolx.github.io/picasso/html/modules.html

### 核心 API 分组

| 分组 | 主要函数 |
|------|---------|
| 初始化 | `ps_initialize`, `ps_shutdown`, `ps_version` |
| 上下文 | `ps_context_create`, `ps_save`, `ps_restore`, `ps_set_*` |
| 画布 | `ps_canvas_create*`, `ps_canvas_bitblt`, `ps_canvas_set_mask` |
| 路径 | `ps_path_create`, `ps_path_move_to`, `ps_path_line_to`, `ps_path_curve_to` |
| 矩阵 | `ps_matrix_create*`, `ps_matrix_translate`, `ps_matrix_rotate`, `ps_matrix_scale` |
| 渐变 | `ps_gradient_create_linear/radial/conic`, `ps_gradient_add_color_stop` |
| 图案 | `ps_pattern_create*`, `ps_set_source_pattern` |
| 图像 | `ps_image_create*`, `ps_set_source_image` |
| 蒙版 | `ps_mask_create`, `ps_mask_add_*` |
| 字体 | `ps_font_create*`, `ps_set_text_color`, `ps_text_out_length` |
| 绘制 | `ps_stroke`, `ps_fill`, `ps_paint` |
| 裁剪 | `ps_clip`, `ps_clip_path`, `ps_clip_rect` |

### 扩展 API 分组

| 分组 | 主要函数 |
|------|---------|
| 图像加载 | `psx_image_init/shutdown`, `psx_image_load*`, `psx_image_save*` |
| 图像插件 | `psx_image_register/unregister_operator` |
| SVG | `psx_svg_init/shutdown`, `psx_svg_load*`, `psx_svg_render_*` |
| SVG 动画 | `psx_svg_player_create/destroy`, `psx_svg_player_play/pause/stop`, `psx_svg_player_tick/seek/draw` |

---

> 本文档基于 Picasso v2.9.0 源码分析生成。如有疑问，请参考源码或联系作者 onecoolx@gmail.com。
