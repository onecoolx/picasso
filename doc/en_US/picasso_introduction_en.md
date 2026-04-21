# Picasso Vector Graphics Library

> Version: 2.9.0 | License: BSD 2-Clause | Language: C++98 + C API  
> Author: Zhang Ji Peng (onecoolx@gmail.com)  
> Repository: https://github.com/onecoolx/picasso

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Overall Architecture](#2-overall-architecture)
3. [Directory Structure](#3-directory-structure)
4. [Core Engine (src/)](#4-core-engine)
5. [Rendering Pipeline](#5-rendering-pipeline)
6. [Extension Subsystems (ext/)](#6-extension-subsystems)
7. [Font System](#7-font-system)
8. [Build System](#8-build-system)
9. [Testing Infrastructure](#9-testing-infrastructure)
10. [WebAssembly Support](#10-webassembly-support)
11. [Engineering Constraints & Coding Conventions](#11-engineering-constraints--coding-conventions)
12. [Quick Start](#12-quick-start)
13. [API Reference Index](#13-api-reference-index)

---

## 1. Project Overview

Picasso is a high-quality, low-footprint 2D vector graphics rendering library. It provides a comprehensive set of high-level 2D graphics APIs suitable for GUI systems, PostScript rendering, SVG image rendering, and more.

### Core Features

- **Path Operations**: Fill, stroke, clip, boolean operations
- **Transforms**: Affine matrix transforms (translate, rotate, scale, skew)
- **Fill Styles**: Solid color, linear/radial/conic gradients, pattern fills
- **Stroke Styles**: Dashing, line caps (Butt/Round/Square), line joins (Miter/Round/Bevel)
- **Image Processing**: Multi-format decoding (JPEG, PNG, WebP, GIF), image compositing
- **Text Rendering**: TrueType fonts, kerning, anti-aliasing
- **Effects**: Blur, shadow, gamma correction, alpha compositing
- **SVG**: SVG Tiny 1.2 parsing and rendering, SVG animation playback
- **Pixel Formats**: RGBA, ARGB, ABGR, BGRA, RGB, BGR, RGB565, RGB555, A8

### Platform Support

| Platform | Build Tool | Font Backend | Status |
|----------|-----------|-------------|--------|
| Linux (x86/ARM) | CMake + Make | FreeType2 + FontConfig | ✅ |
| macOS | CMake + Xcode | CoreText | ✅ |
| Windows | CMake + Visual Studio | GDI | ✅ |
| Android | CMake + NDK | FreeType2 | ✅ |
| WebAssembly | Emscripten | — | ✅ |

### Version History Summary

| Version | Date | Milestone |
|---------|------|-----------|
| 1.2.0 | 2011-05 | First GPL-v2 release |
| 2.0.0 | 2013-02 | Architecture rewrite, switched to BSD license |
| 2.1.5 | 2015-03 | Android porting |
| 2.2.0 | 2016-02 | Image codec extension modules |
| 2.5.0 | 2023-02 | Full macOS support |
| 2.8.0 | 2024-07 | Full CMake build system |
| 2.9.0 | Current | SVG Tiny 1.2 animation support |

---

## 2. Overall Architecture

Picasso uses a layered architecture, organized from top to bottom into the public API layer, core engine layer, graphics backend layer, and extension subsystems.

```
┌─────────────────────────────────────────────────────────┐
│                    Application Code                      │
├─────────────────────────────────────────────────────────┤
│                  Public C API (include/)                  │
│   ps_context  ps_canvas  ps_path  ps_gradient  ps_font   │
├──────────────────────┬──────────────────────────────────┤
│   Core Engine (src/) │    Extension Subsystems (ext/)    │
│  ┌────────────────┐  │  ┌─────────────┐ ┌────────────┐ │
│  │ Object Mgmt    │  │  │ SVG System   │ │ Image Codec│ │
│  │ State Stack    │  │  │ Parse/Render │ │ JPEG/PNG   │ │
│  │ Path/Matrix    │  │  │ Anim Player  │ │ WebP/GIF   │ │
│  └────────────────┘  │  └─────────────┘ └────────────┘ │
├──────────────────────┴──────────────────────────────────┤
│              Graphics Backend (src/gfx/)                 │
│  Rasterizer → Scanline Render → Blending → Compositing   │
├─────────────────────────────────────────────────────────┤
│              Infrastructure (src/include/, src/core/)     │
│  Memory Mgmt  Math Types  Matrix  Path Data  Curves      │
└─────────────────────────────────────────────────────────┘
```

### Design Principles

1. **Strict C++98 Compliance**: No STL containers, lambdas, auto, or any C++11+ features
2. **Explicit Memory Management**: Project-internal allocators (`mem_malloc`/`mem_free`), no hidden ownership
3. **Reference Counting**: All opaque objects use refcount for lifecycle management
4. **Modular Extensions**: SVG and image codecs are optional, pluggable subsystems
5. **Performance First**: SIMD optimization, scanline rasterization, efficient pixel blending
6. **Multi-Format Support**: Flexible pixel format handling with format-specific optimizations
7. **Platform Abstraction**: Unified API across Windows, macOS, Linux, Android, WebAssembly

---

## 3. Directory Structure

```
picasso/
├── include/                  # Public API headers
│   ├── picasso.h             # Core API (context, canvas, path, gradient, etc.)
│   ├── picasso_ext.h         # Extension common definitions (result codes)
│   ├── picasso_backport.h    # Memory allocator customization interface
│   ├── images/               # Image extension API
│   │   ├── psx_image.h       # Image load/save interface
│   │   └── psx_image_plugin.h # Image codec plugin interface
│   └── svg/                  # SVG extension API
│       ├── psx_svg.h         # SVG document loading and rendering
│       └── psx_svg_animation.h # SVG animation player
├── src/                      # Core engine implementation
│   ├── include/              # Internal headers (data structures, interface defs)
│   ├── core/                 # Foundation components (path, matrix, clip, memory)
│   ├── gfx/                  # Graphics backend (rasterization, compositing, pixfmt)
│   ├── font/                 # Font adapters (platform-specific)
│   ├── simd/                 # SIMD optimization implementations
│   └── picasso_*.cpp/h       # API implementations and internal objects
├── ext/                      # Extension subsystems
│   ├── common/               # Shared extension utilities
│   ├── image_coders/         # Image codecs
│   │   ├── jpeg/png/gif/webp/apple/  # Format-specific implementations
│   │   └── psx_image_*.c/h   # Loader, module management
│   └── svg/                  # SVG subsystem
│       ├── psx_svg_parser.cpp # XML parser
│       ├── psx_svg_render.cpp # SVG renderer
│       ├── psx_svg_player.cpp # Animation player
│       ├── psx_svg_node.cpp/h # Node tree
│       └── psx_xml_token.cpp/h # XML tokenizer
├── third_party/              # Third-party dependencies
│   ├── zlib-1.2.8/           # Compression library
│   ├── libpng-1.6.17/        # PNG codec
│   ├── libjpeg-turbo-1.4.1/  # JPEG codec
│   ├── giflib-5.1.3/         # GIF codec
│   ├── libwebp-0.5.1/        # WebP codec
│   ├── freetype-2.3.6/       # Font rasterizer
│   └── expat-2.1.0/          # XML parser (Android)
├── unit_tests/               # GoogleTest unit tests
├── perf_tests/               # Performance benchmarks
├── test/                     # Platform GUI test applications
├── demos/                    # Example programs
├── wasm/                     # WebAssembly build
├── tools/                    # Development tool scripts
├── cfg/                      # Configuration files (fonts, etc.)
├── .design_docs/             # Design documents
├── build/                    # CMake build configurations
└── CMakeLists.txt            # Root build file
```


---

## 4. Core Engine

The core engine resides in `src/` and handles graphics object management, state stack, path operations, and render dispatch.

### 4.1 Public API Objects (`include/picasso.h`)

All public objects are opaque types operated through C function interfaces, with reference counting for lifecycle management.

| Object Type | Description | Create / Destroy |
|------------|-------------|-----------------|
| `ps_context` | Drawing context, holds state stack and canvas reference | `ps_context_create` / `ps_context_unref` |
| `ps_canvas` | Pixel buffer, render target | `ps_canvas_create*` / `ps_canvas_unref` |
| `ps_image` | Image object | `ps_image_create*` / `ps_image_unref` |
| `ps_path` | Graphics path (Bézier curves, lines, arcs) | `ps_path_create` / `ps_path_unref` |
| `ps_matrix` | Affine transformation matrix | `ps_matrix_create*` / `ps_matrix_unref` |
| `ps_gradient` | Gradient (linear, radial, conic) | `ps_gradient_create_*` / `ps_gradient_unref` |
| `ps_pattern` | Pattern fill | `ps_pattern_create*` / `ps_pattern_unref` |
| `ps_mask` | Alpha mask | `ps_mask_create` / `ps_mask_unref` |
| `ps_font` | Font object | `ps_font_create*` / `ps_font_unref` |

### 4.2 Internal Objects (`src/picasso_objects.h`)

Internal C++ classes encapsulate various aspects of drawing state:

- **`graphic_pen`**: Stroke style — color/gradient/pattern/image fill, line width, cap, join, dash pattern
- **`graphic_brush`**: Fill style — color/gradient/pattern/image fill, fill rule
- **`shadow_state`**: Shadow effect — offset, blur radius, color
- **`clip_area`**: Clip region — path-based or rectangular clipping
- **`context_state`**: Drawing state — matrix, pen, brush, clip, font, alpha, gamma, blur, composite mode

`context_state` supports save/restore (`ps_save`/`ps_restore`), forming a state stack.

### 4.3 Infrastructure (`src/include/`, `src/core/`)

| File | Responsibility |
|------|---------------|
| `memory_manager.h/cpp` | Custom memory allocator (`mem_malloc`/`mem_free`) |
| `math_type.h` | Math type definitions (`scalar` type, fixed-point support) |
| `matrix.h` / `matrix.cpp` | Affine transformation matrix (`trans_affine`) |
| `graphic_path.h/cpp` | Path data structures and operations |
| `curve.h/cpp` | Bézier curve handling (quadratic/cubic curve flattening) |
| `clipper.h/cpp` | Path boolean clipping operations |
| `data_vector.h` | Project-internal dynamic array (replaces `std::vector`) |
| `color_type.h` | Color type definitions (`rgba`) |
| `geometry.h` | Geometry utility functions |
| `interfaces.h` | Abstract interface definitions (device abstraction layer) |

---

## 5. Rendering Pipeline

The rendering pipeline resides in `src/gfx/` and is the performance core of Picasso. It uses a scanline rasterization architecture.

### 5.1 Pipeline Flow

```
User Draw Calls (ps_fill / ps_stroke)
        │
        ▼
┌─────────────────┐
│  Path Processing │  graphic_path → transform → stroke/fill expansion
│  (raster_adapter)│
└────────┬────────┘
         ▼
┌─────────────────┐
│  Rasterization   │  path → cells → scanlines
│  (rasterizer)    │  gfx_rasterizer_cell → gfx_rasterizer_scanline
└────────┬────────┘
         ▼
┌─────────────────┐
│  Span Generation │  Generate pixel spans based on fill type
│  (span_generator)│  solid / gradient / pattern / image
└────────┬────────┘
         ▼
┌─────────────────┐
│  Compositing &   │  Alpha compositing, blend modes
│  Blending        │  gfx_composite_packed / gfx_blender_packed
└────────┬────────┘
         ▼
┌─────────────────┐
│  Pixel Format    │  Format-specific pixel operations
│  Write (pixfmt)  │  gfx_pixfmt_rgba / rgb / rgb16 / gray
└────────┬────────┘
         ▼
┌─────────────────┐
│  Rendering       │  Final pixel output
│  Buffer          │  Underlying storage of ps_canvas
└─────────────────┘
```

### 5.2 Key Components

#### Rasterizer
- **`gfx_rasterizer_cell.h`**: Cell-level rasterization, decomposes paths into coverage cells
- **`gfx_rasterizer_scanline.h`**: Scanline rasterizer, converts cells to scanline data
- **`gfx_scanline.h`**: Scanline data structures
- **`gfx_scanline_storage.h`**: Scanline storage (for caching rasterization results)

#### Renderer
- **`gfx_renderer.h`**: Basic rendering primitives
- **`gfx_scanline_renderer.h`**: Scanline renderer, writes scanline data to pixel buffer
- **`gfx_painter.h`**: High-level painter interface, orchestrates the entire rendering flow

#### Span Generators
- **`gfx_span_generator.h`**: Span generation framework
- **`gfx_span_image_filters.h`**: Image filtering spans (scaling, interpolation)
- **`gfx_gradient_adapter.h/cpp`**: Gradient rendering adapter

#### Pixel Format Handlers
- **`gfx_pixfmt_rgba.h`**: 32-bit RGBA/ARGB/ABGR/BGRA
- **`gfx_pixfmt_rgb.h`**: 24-bit RGB/BGR
- **`gfx_pixfmt_rgb16.h`**: 16-bit RGB565/RGB555
- **`gfx_pixfmt_gray.h`**: 8-bit grayscale (A8)
- **`gfx_pixfmt_wrapper.h`**: Pixel format abstraction wrapper

#### Compositing & Blending
- **`gfx_composite_packed.h`**: Porter-Duff compositing operations
- **`gfx_blender_packed.h`**: Pixel-level blending operations

#### Effects
- **`gfx_blur.h/cpp`**: Gaussian blur implementation
- **`gfx_image_filters.h/cpp`**: Image filters (bilinear, bicubic interpolation, etc.)
- **`gfx_gamma_function.h`**: Gamma correction functions
- **`gfx_mask_layer.h`**: Alpha mask layer

### 5.3 Painter Class (`src/picasso_painter.h`)

The `painter` class is the entry point to the rendering pipeline:

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

## 6. Extension Subsystems

Extension subsystems reside in `ext/` and include two major modules: image codecs and SVG. They are controlled by the `OPT_EXTENSIONS` CMake option.

### 6.1 Image Codecs (`ext/image_coders/`)

#### Architecture

The image system uses a plugin architecture with signature-based automatic format detection:

```
psx_image_load()
      │
      ▼
┌──────────────┐
│ Signature     │  Read file header bytes, match registered codecs
│ Detection     │
└──────┬───────┘
       ▼
┌──────────────┐
│ Codec         │  Select matching psx_image_operator by priority
│ Selection     │
└──────┬───────┘
       ▼
┌──────────────┐
│ Decode        │  read_header_info → decode_image_data → release
│ Execution     │
└──────┬───────┘
       ▼
   psx_image (multi-frame support)
```

#### Plugin Interface (`psx_image_plugin.h`)

Each codec implements the `psx_image_operator` struct:

```c
typedef struct _psx_image_operator {
    int32_t (*read_header_info)(...);           // Read image header info
    int32_t (*decode_image_data)(...);          // Decode frame data
    int32_t (*release_read_header_info)(...);   // Release read resources
    int32_t (*write_header_info)(...);          // Create write header
    int32_t (*encode_image_data)(...);          // Encode frame data
    int32_t (*release_write_header_info)(...);  // Release write resources
} psx_image_operator;
```

Registration specifies format name, file signature, and priority:
- `PRIORITY_MASTER` (1): Highest priority
- `PRIORITY_DEFAULT` (0): Default priority
- `PRIORITY_EXTENTED` (-1): Extended priority

#### Supported Formats

| Format | Directory | Encode | Decode | Multi-frame |
|--------|-----------|--------|--------|-------------|
| JPEG | `jpeg/` | ✅ | ✅ | — |
| PNG | `png/` | ✅ | ✅ | — |
| WebP | `webp/` | ✅ | ✅ | — |
| GIF | `gif/` | — | ✅ | ✅ (with frame duration) |
| Apple (macOS) | `apple/` | ✅ | ✅ | — |

### 6.2 SVG Subsystem (`ext/svg/`)

The SVG subsystem implements SVG Tiny 1.2 specification parsing, rendering, and animation playback.

#### Architecture

```
SVG File / Memory Data
      │
      ▼
┌──────────────┐
│ XML Tokenizer │  psx_xml_token.cpp — breaks XML text into tokens
└──────┬───────┘
       ▼
┌──────────────┐
│ SVG Parser    │  psx_svg_parser.cpp — builds node tree, extracts attributes
└──────┬───────┘
       ▼
┌──────────────┐
│ SVG Node Tree │  psx_svg_node.cpp — tree document structure
└──────┬───────┘
       │
       ├──────────────────────┐
       ▼                      ▼
┌──────────────┐      ┌──────────────┐
│ SVG Renderer  │      │ Anim Player   │
│ psx_svg_render│      │ psx_svg_player│
│ Static render │      │ State mgmt    │
└──────────────┘      └──────────────┘
```

#### SVG Renderer (`psx_svg_render.cpp`)

- Traverses the node tree, converting SVG elements to Picasso draw calls
- Supported elements: `<rect>`, `<circle>`, `<ellipse>`, `<line>`, `<polyline>`, `<polygon>`, `<path>`, `<text>`, `<image>`, `<g>`, `<use>`, `<defs>`, `<linearGradient>`, `<radialGradient>`
- In animation mode, queries `anim_state` for override values; non-animated nodes are skipped entirely

#### SVG Animation Player (`psx_svg_player.cpp`)

The animation player is the most recently developed core feature, achieving approximately 97% coverage of the SVG Tiny 1.2 animation specification.

##### Supported Animation Elements

| Element | Description |
|---------|-------------|
| `<animate>` | Numeric attribute interpolation (float/int32) |
| `<set>` | Discrete property assignment (enum, visibility, font) |
| `<animateColor>` | RGB color interpolation (fill, stroke) |
| `<animateTransform>` | Transform interpolation (translate/scale/rotate/skewX/skewY/matrix) |
| `<animateMotion>` | Path-following motion with auto-rotate |

##### Timing Model

- **Begin/End**: Offset lists, event-driven, syncbase, accessKey, indefinite
- **Duration & Repetition**: `dur`, `repeatCount` (including fractional), `repeatDur`, `min`/`max` constraints
- **Fill Mode**: `remove` (default), `freeze`
- **Restart Policy**: `always`, `whenNotActive`, `never`

##### Interpolation Modes (`calcMode`)

- `linear` (default), `discrete`, `spline` (cubic Bézier via keySplines), `paced` (arc-length parameterization)

##### Composition Model

- `additive="sum"` — numeric addition or matrix composition
- `accumulate="sum"` — results accumulate across repeat cycles
- `animateTransform` and `animateMotion` compose as independent layers

##### Animation State (`psx_svg_anim_state.h`)

```
psx_svg_anim_state
├── overrides[]          — numeric overrides (float/int32)
├── color_overrides[]    — color overrides (fill/stroke)
├── transforms[]         — animateTransform result matrices
├── motion_transforms[]  — animateMotion result matrices
├── dash_overrides[]     — stroke-dasharray arrays
└── active_targets[]     — active target nodes (renderer fast-skip)
```

Final animated transform = `motion_transform × animateTransform_result`

##### Performance Optimizations

| Optimization | Benefit |
|-------------|---------|
| SBO (Small Buffer Optimization) for begins/ends | Eliminates 2 heap allocs per anim item |
| Append-only writes + post-sort dedup | Removes O(n²) per-frame write scan |
| Active-target fast-skip in renderer | Skips ~80% of wasted binary searches on non-animated nodes |
| Dash buffer reuse across frames | Eliminates per-frame malloc/free |
| Additive base value pre-cache | Eliminates per-frame `_find_attr` scan |
| Syncbase ID lookup table | O(log n) binary search replaces O(n) strcmp scan |
| Time-window fast culling | Skips expired fill=remove animations without entering eval |


---

## 7. Font System

The font system resides in `src/font/` and uses a platform-abstraction design.

### Architecture

```
ps_font (Public API)
    │
    ▼
font_engine (Font Engine)
    │  Manages font pool (up to MAX_FONTS=16 cached fonts)
    │  Signature matching to avoid duplicate creation
    ▼
font (Font Instance)
    ├── font_desc — Font descriptor (name, size, weight, italic, etc.)
    ├── font_adapter — Platform adapter (actual font loading)
    ├── glyph_cache_manager — Glyph cache
    └── mono_storage — Monochrome glyph storage
```

### Platform Adapters

| Platform | Adapter File | Font Backend |
|----------|-------------|-------------|
| macOS | `font_adapter_apple.mm` | CoreText |
| Linux | `font_adapter_freetype.cpp` | FreeType2 + FontConfig |
| Windows | `font_adapter_win32.cpp` | GDI |
| Android | `font_adapter_freetype.cpp` | FreeType2 |

### Glyph Cache

`glyph_cache_manager` indexes cached glyph data by font signature (which includes name, size, transform matrix, etc.) to avoid redundant rasterization. Each glyph contains:
- Path data (vector outlines)
- Rasterized bitmap (monochrome or anti-aliased)
- Metric information (advance, bounds)

---

## 8. Build System

### CMake Configuration

The root `CMakeLists.txt` defines all build options:

#### Core Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | ON | Build shared libraries |
| `OPT_EXTENSIONS` | ON | Compile extension subsystems (SVG + Image) |
| `OPT_FREE_TYPE2` | OFF | Enable FreeType2 font support |
| `OPT_FONT_CONFIG` | OFF | Enable FontConfig font lookup |
| `OPT_FAST_COPY` | OFF | Fast memory copy optimization |
| `OPT_LOW_MEMORY` | OFF | Low memory mode |
| `OPT_SYSTEM_MALLOC` | OFF | Use system memory allocator |

#### Pixel Format Options

All `OPT_FORMAT_*` options are ON by default: ABGR, ARGB, BGRA, RGBA, RGB, BGR, RGB565, RGB555, A8.

#### Test & Debug Options

| Option | Default | Description |
|--------|---------|-------------|
| `OPT_TESTS` | ON | Compile test applications |
| `OPT_DEMOS` | ON | Compile example demos |
| `OPT_UNITTEST` | OFF | Compile GoogleTest unit tests |
| `OPT_PERFTEST` | OFF | Compile performance tests |
| `OPT_SANITIZE` | OFF | Enable AddressSanitizer |
| `OPT_COVERAGE` | OFF | Enable code coverage |

#### Third-Party Library Options

`OPT_SYSTEM_GIF/JPEG/PNG/WEBP/ZLIB` — Use system libraries or bundled third-party libraries.

### Build Artifacts

| Artifact | Description |
|----------|-------------|
| `libpicasso2_sw.so` | Core graphics library |
| `libpsx_image.so` | Image codec extension library |
| `libpsx_svg.so` | SVG extension library (dynamic) |
| `libpsx_svg_static.a` | SVG extension library (static, for unit tests) |
| `libpsx_common.a` | Extension common utilities library |
| `libpsxm_image_*.so` | Image codec plugin modules |
| `unit_tests` | Unit test executable |

### Platform Build Steps

#### Linux
```bash
./build_linux.sh          # Generates proj/ directory
cd proj && make -j$(nproc)
```

#### macOS
```bash
./build_macosx.sh         # Generates Xcode project
cd proj
# Open picasso.xcodeproj with Xcode
```

#### Windows
```bat
build_windows.bat         # Generates Visual Studio solution
cd proj
# Open picasso.sln with Visual Studio
```

#### WebAssembly
```bash
cd wasm
./build_wasm.sh           # Compile with Emscripten
# Output: picasso.wasm + lib.js
```


---

## 9. Testing Infrastructure

### 9.1 Unit Tests (`unit_tests/`)

- **Framework**: GoogleTest v1.12.1 (auto-downloaded via CMake FetchContent)
- **Snapshot Comparison**: Uses lodepng for PNG snapshot comparison tests
- **Test Scale**: 864+ test cases (including 180 SVG animation tests)

#### Test File Organization

| File Pattern | Coverage |
|-------------|---------|
| `ps_*_api.cpp` | Core API tests (context, canvas, path, matrix, gradient, etc.) |
| `ps_*_test.cpp` | Core functionality tests (pixel formats, compositing, rendering, blur, etc.) |
| `ext_svg*.cpp` | SVG extension tests (parsing, rendering, animation player) |
| `ext_image.cpp` | Image codec tests |
| `ext_*.cpp` | Other extension tests (array, tree, list, linear allocator) |

#### Running Tests

```bash
# Run in WSL (project convention)
wsl -e bash -c "cd /home/jipeng/picasso/proj && ./unit_tests"

# Focused SVG animation tests
wsl -e bash -c "cd /home/jipeng/picasso/proj && ./unit_tests --gtest_filter='SVGPlayerTest.*'"
```

### 9.2 GUI Test Applications (`test/`)

Platform-specific interactive test programs. Each `*_func.c` file corresponds to a test scenario:

| Test | File | Validates |
|------|------|-----------|
| alpha | `alpha_func.c` | Alpha blending |
| bitblt | `bitblt_func.c` | Pixel block transfer |
| blur | `blur_func.c` | Blur effects |
| clip | `clip_func.c` | Path clipping |
| composite | `composite_func.c` | Compositing modes |
| gamma | `gamma_func.c` | Gamma correction |
| gcstate | `gcstate_func.c` | Graphics state management |
| gradient | `gradient_func.c` | Gradient rendering |
| mask | `mask_func.c` | Mask operations |
| path | `path_func.c` | Path operations |
| pattern | `pattern_func.c` | Pattern fills |
| shadow | `shadow_func.c` | Shadow effects |
| text | `text_func.c` | Text rendering |
| thread | `thread_func.c` | Multi-thread safety |

Platform entry points:
- Linux: `gtk2/testGtk2.c` (GTK2)
- macOS: `mac/testMac.m` (Cocoa)
- Windows: `win32/testWin.c` (Win32 API)
- Android: `android/testAndroid.cpp` (NDK)

### 9.3 Demo Programs (`demos/`)

| Demo | File | Showcases |
|------|------|-----------|
| clock | `clock.c` | Analog clock — arcs, gradients, affine transforms |
| flowers | `flowers.c` | Flowers — alpha blending, multi-layer compositing |
| tiger | `tiger.c` | SVG tiger — complex path and gradient rendering |
| subwaymap | `subwaymap.c` | Subway map — high-density polyline and label rendering |

### 9.4 Performance Tests (`perf_tests/`)

Benchmark suite covering performance validation of core operations:

| Test | File |
|------|------|
| Path drawing | `ps_path_drawing_test.cpp` |
| Matrix operations | `ps_matrix_test.cpp` |
| Gradient rendering | `ps_gradient_test.cpp` |
| Pattern fills | `ps_pattern_test.cpp` |
| Image operations | `ps_image_test.cpp` |
| Mask operations | `ps_mask_test.cpp` |
| Canvas operations | `ps_canvas_test.cpp` |
| Context operations | `ps_context_test.cpp` |
| Font rendering | `ps_font_test.cpp` |
| SVG parsing | `ext_svg_parser_test.cpp` |
| Complex drawing | `ps_complex_draw_test.cpp` |

---

## 10. WebAssembly Support

Picasso compiles to WebAssembly via Emscripten, located in the `wasm/` directory.

### Build Process

```bash
# 1. emmake compiles static library
emmake make -f Makefile.em

# 2. emcc links to .wasm
emcc --no-entry \
  -s"ALLOW_MEMORY_GROWTH=1" \
  -s"EXPORTED_FUNCTIONS=@export.list" \
  -s"EXPORTED_RUNTIME_METHODS=@runtime.list" \
  libpicasso.a -o picasso.wasm

# 3. Cleanup
emmake make -f Makefile.em clean
```

### Key Files

| File | Description |
|------|-------------|
| `Makefile.em` | Emscripten build rules |
| `export.list` | Exported C function list |
| `runtime.list` | Exported runtime methods (malloc, free, etc.) |
| `lib.js` | JavaScript binding layer |
| `pconfig.h` | WASM platform configuration |
| `test.html` | Browser test page |
| `picasso.wasm` | Compiled output |


---

## 11. Engineering Constraints & Coding Conventions

### Language Constraints

- **Strict C++98 mode**: No C++11 or later features
- **No STL containers**: No `std::vector`, `std::map`, etc. — use project-internal `data_vector`
- **No lambdas/closures**: Use function pointers and callbacks
- **Explicit memory management**: Use `mem_malloc`/`mem_free`, avoid implicit ownership transfer

### Memory Management

```cpp
// Project-internal allocators
void* mem_malloc(size_t size);
void  mem_free(void* ptr);

// Customizable via picasso_backport.h
typedef void* (*ps_malloc_func)(size_t);
typedef void  (*ps_free_func)(void*);
typedef void* (*ps_calloc_func)(size_t, size_t);
```

### Reference Counting Pattern

All public objects follow a uniform reference counting pattern:

```c
ps_object* obj = ps_object_create(...);  // refcount = 1
ps_object_ref(obj);                       // refcount = 2
ps_object_unref(obj);                     // refcount = 1
ps_object_unref(obj);                     // refcount = 0, freed
```

---

## 12. Quick Start

### Minimal Example

```c
#include "picasso.h"

int main(void)
{
    /* 1. Initialize */
    ps_initialize();

    /* 2. Create canvas */
    int width = 400, height = 300, stride = width * 4;
    unsigned char* buf = (unsigned char*)malloc(stride * height);
    memset(buf, 0, stride * height);
    ps_canvas* canvas = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA,
                                                   width, height, stride);

    /* 3. Create drawing context */
    ps_context* ctx = ps_context_create(canvas, NULL);

    /* 4. Draw a gradient rectangle */
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

    /* 5. Release resources */
    ps_gradient_unref(grad);
    ps_context_unref(ctx);
    ps_canvas_unref(canvas);
    free(buf);
    ps_shutdown();
    return 0;
}
```

### SVG Rendering Example

```c
#include "picasso.h"
#include "psx_svg.h"

// Load and render SVG
psx_svg* doc = psx_svg_load("tiger.svg", NULL);
psx_svg_render* render = psx_svg_render_create(doc, NULL);

ps_size size;
psx_svg_render_get_size(render, &size);
// Create canvas and context...
psx_svg_render_draw(ctx, render);

psx_svg_render_destroy(render);
psx_svg_destroy(doc);
```

### SVG Animation Playback Example

```c
#include "psx_svg_animation.h"

psx_svg* doc = psx_svg_load("animation.svg", NULL);
psx_svg_player* player = psx_svg_player_create(doc, NULL);

psx_svg_player_play(player);

// In game loop
float dt = 0.016f; // 16ms
psx_svg_player_tick(player, dt);
psx_svg_player_draw(player, ctx);

psx_svg_player_destroy(player);
psx_svg_destroy(doc);
```

---

## 13. API Reference Index

Full API documentation: http://onecoolx.github.io/picasso/html/modules.html

### Core API Groups

| Group | Key Functions |
|-------|--------------|
| Initialization | `ps_initialize`, `ps_shutdown`, `ps_version` |
| Context | `ps_context_create`, `ps_save`, `ps_restore`, `ps_set_*` |
| Canvas | `ps_canvas_create*`, `ps_canvas_bitblt`, `ps_canvas_set_mask` |
| Path | `ps_path_create`, `ps_path_move_to`, `ps_path_line_to`, `ps_path_curve_to` |
| Matrix | `ps_matrix_create*`, `ps_matrix_translate`, `ps_matrix_rotate`, `ps_matrix_scale` |
| Gradient | `ps_gradient_create_linear/radial/conic`, `ps_gradient_add_color_stop` |
| Pattern | `ps_pattern_create*`, `ps_set_source_pattern` |
| Image | `ps_image_create*`, `ps_set_source_image` |
| Mask | `ps_mask_create`, `ps_mask_add_*` |
| Font | `ps_font_create*`, `ps_set_text_color`, `ps_text_out_length` |
| Drawing | `ps_stroke`, `ps_fill`, `ps_paint` |
| Clipping | `ps_clip`, `ps_clip_path`, `ps_clip_rect` |

### Extension API Groups

| Group | Key Functions |
|-------|--------------|
| Image Loading | `psx_image_init/shutdown`, `psx_image_load*`, `psx_image_save*` |
| Image Plugin | `psx_image_register/unregister_operator` |
| SVG | `psx_svg_init/shutdown`, `psx_svg_load*`, `psx_svg_render_*` |
| SVG Animation | `psx_svg_player_create/destroy`, `psx_svg_player_play/pause/stop`, `psx_svg_player_tick/seek/draw` |

---

> This document was generated based on analysis of the Picasso v2.9.0 source code. For questions, refer to the source code or contact the author at onecoolx@gmail.com.
