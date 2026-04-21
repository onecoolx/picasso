# Picasso Programming Guide

> Version: 2.9.0 | License: BSD 2-Clause  
> This guide covers Picasso programming methods grouped by API, with SVG-simulated visual results for each example.

---

## Table of Contents

1. [Initialization & Lifecycle](#1-initialization--lifecycle)
2. [Canvas](#2-canvas)
3. [Context](#3-context)
4. [Path](#4-path)
5. [Drawing Operations](#5-drawing-operations)
6. [Stroke Style](#6-stroke-style)
7. [Fill Source](#7-fill-source)
8. [Gradient](#8-gradient)
9. [Pattern](#9-pattern)
10. [Matrix Transform](#10-matrix-transform)
11. [Clipping](#11-clipping)
12. [Image](#12-image)
13. [Mask](#13-mask)
14. [Font & Text](#14-font--text)
15. [Extension: Image Loading](#15-extension-image-loading)
16. [Extension: SVG Rendering](#16-extension-svg-rendering)
17. [Extension: SVG Animation](#17-extension-svg-animation)

---

## 1. Initialization & Lifecycle

Picasso must be initialized before use and shut down afterwards. All API calls must occur between `ps_initialize()` and `ps_shutdown()`.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_initialize()` | Initialize the Picasso environment; must be called first |
| `ps_shutdown()` | Shut down the Picasso environment; must be called last |
| `ps_version()` | Return the current version number |
| `ps_last_status()` | Return the status code of the most recent operation |

### Example

```c
#include "picasso.h"
#include <stdio.h>

int main(void)
{
    if (!ps_initialize()) {
        printf("Failed to initialize Picasso!\n");
        return -1;
    }
    printf("Picasso version: %d\n", ps_version());
    /* ... all drawing operations ... */
    ps_shutdown();
    return 0;
}
```

### Status Codes

| Status Code | Meaning |
|-------------|---------|
| `STATUS_SUCCEED` | Success |
| `STATUS_OUT_OF_MEMORY` | Out of memory |
| `STATUS_INVALID_ARGUMENT` | Invalid argument |
| `STATUS_NOT_SUPPORT` | Unsupported feature |
| `STATUS_DEVICE_ERROR` | Device not ready |
| `STATUS_MISMATCHING_FORMAT` | Pixel format mismatch |

---

## 2. Canvas

The canvas is Picasso's rendering target — essentially a pixel buffer. All drawing operations ultimately write to a canvas.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_canvas_create(fmt, w, h)` | Create a canvas (auto-allocated memory) |
| `ps_canvas_create_with_data(data, fmt, w, h, pitch)` | Create a canvas using external memory |
| `ps_canvas_create_compatible(canvas, w, h)` | Create a compatible canvas |
| `ps_canvas_create_from_canvas(canvas, rect)` | Create from a sub-region of an existing canvas (shared pixel buffer) |
| `ps_canvas_create_from_image(img, rect)` | Create a canvas from an image |
| `ps_canvas_ref(canvas)` / `ps_canvas_unref(canvas)` | Reference counting |
| `ps_canvas_get_size(canvas, &size)` | Get canvas dimensions |
| `ps_canvas_get_format(canvas)` | Get pixel format |
| `ps_canvas_bitblt(src, rect, dst, location)` | Pixel block transfer |
| `ps_canvas_set_mask(canvas, mask)` | Set mask |
| `ps_canvas_reset_mask(canvas)` | Clear mask |

### Pixel Formats

| Format | Bit Depth | Description |
|--------|-----------|-------------|
| `COLOR_FORMAT_RGBA` | 32bpp | Most common, RGBA order |
| `COLOR_FORMAT_ARGB` | 32bpp | Common on Windows |
| `COLOR_FORMAT_BGRA` | 32bpp | Windows GDI compatible |
| `COLOR_FORMAT_RGB` | 24bpp | No alpha channel |
| `COLOR_FORMAT_RGB565` | 16bpp | Common on embedded devices |
| `COLOR_FORMAT_A8` | 8bpp | Grayscale / mask |

### Example: Create a Canvas and Transfer Pixels

```c
/* Create a 400x300 RGBA canvas */
int w = 400, h = 300, stride = w * 4;
ps_byte* buf = (ps_byte*)malloc(stride * h);
memset(buf, 255, stride * h);  /* white background */

ps_canvas* canvas = ps_canvas_create_with_data(buf, COLOR_FORMAT_RGBA, w, h, stride);

/* Create a sub-region canvas */
ps_rect sub = {50, 50, 200, 150};
ps_canvas* sub_canvas = ps_canvas_create_from_canvas(canvas, &sub);

/* Pixel block transfer */
ps_point dst_pt = {100, 100};
ps_canvas_bitblt(sub_canvas, NULL, canvas, &dst_pt);

ps_canvas_unref(sub_canvas);
ps_canvas_unref(canvas);
free(buf);
```

---

## 3. Context

The context is Picasso's core object, holding a drawing state stack (transform matrix, pen, brush, clipping region, etc.).

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_context_create(canvas, shared)` | Create a drawing context |
| `ps_context_ref/unref` | Reference counting |
| `ps_context_set_canvas(ctx, canvas)` | Switch canvas (returns the old canvas) |
| `ps_context_get_canvas(ctx)` | Get the current canvas |
| `ps_save(ctx)` | Save the current state |
| `ps_restore(ctx)` | Restore the previously saved state |

### State Stack

`ps_save`/`ps_restore` save and restore the following state: transform matrix, fill color/gradient/pattern, stroke style, clipping region, alpha, gamma, blur, compositing mode, and fill rule.

### Example: Save and Restore State

```c
ps_context* ctx = ps_context_create(canvas, NULL);

/* Set red fill */
ps_color red = {1, 0, 0, 1};
ps_set_source_color(ctx, &red);

ps_save(ctx);  /* save state */

    /* Change to blue in the saved state */
    ps_color blue = {0, 0, 1, 1};
    ps_set_source_color(ctx, &blue);
    ps_rect r1 = {10, 10, 80, 80};
    ps_rectangle(ctx, &r1);
    ps_fill(ctx);  /* blue rectangle */

ps_restore(ctx);  /* restore to red */

ps_rect r2 = {100, 10, 80, 80};
ps_rectangle(ctx, &r2);
ps_fill(ctx);  /* red rectangle */

ps_context_unref(ctx);
```

Visual Result:

<svg width="200" height="100" xmlns="http://www.w3.org/2000/svg">
  <rect x="10" y="10" width="80" height="80" fill="blue"/>
  <rect x="100" y="10" width="80" height="80" fill="red"/>
</svg>

---

## 4. Path

Paths are the foundation of all vector graphics. Picasso supports two ways to work with paths: building paths directly on the context, or creating standalone `ps_path` objects.

### Context Path Functions

| Function | Description |
|----------|-------------|
| `ps_new_path(ctx)` | Clear the current path and start a new one |
| `ps_new_sub_path(ctx)` | Start a new sub-path within the current path |
| `ps_close_path(ctx)` | Close the current sub-path |
| `ps_move_to(ctx, &pt)` | Move to the specified point (no line drawn) |
| `ps_line_to(ctx, &pt)` | Draw a straight line from the current point to the specified point |
| `ps_bezier_to(ctx, &fcp, &scp, &ep)` | Cubic Bézier curve |
| `ps_quad_curve_to(ctx, &cp, &ep)` | Quadratic Bézier curve |
| `ps_arc(ctx, &cp, radius, sangle, eangle, clockwise)` | Arc |
| `ps_tangent_arc(ctx, &r, &t, radius)` | Tangent arc |
| `ps_rectangle(ctx, &rect)` | Rectangle |
| `ps_rounded_rect(ctx, &rect, ltx,lty,rtx,rty,rbx,rby,lbx,lby)` | Rounded rectangle |
| `ps_ellipse(ctx, &rect)` | Ellipse |
| `ps_set_path(ctx, &path)` | Set a standalone path onto the context |
| `ps_get_path(ctx)` | Get a copy of the current path |

### Standalone Path Object Functions

| Function | Description |
|----------|-------------|
| `ps_path_create()` | Create an empty path |
| `ps_path_create_copy(path)` | Copy a path |
| `ps_path_ref/unref` | Reference counting |
| `ps_path_move_to(path, &pt)` | Move to |
| `ps_path_line_to(path, &pt)` | Line to |
| `ps_path_bezier_to(path, &fcp, &scp, &ep)` | Cubic Bézier |
| `ps_path_quad_curve_to(path, &cp, &ep)` | Quadratic Bézier |
| `ps_path_arc(path, &cp, r, sa, ea, cw)` | Arc |
| `ps_path_tangent_arc(path, &r, &t, radius)` | Tangent arc |
| `ps_path_sub_close(path)` | Close sub-path |
| `ps_path_get_vertex_count(path)` | Get vertex count |
| `ps_path_bounding_rect(path, &rect)` | Get bounding rectangle |
| `ps_path_contains(path, &pt, rule)` | Test if a point is inside the path |
| `ps_path_stroke_contains(path, &pt, width)` | Test if a point is on the stroke |
| `ps_path_is_empty(path)` | Test if the path is empty |

### Example: Drawing Various Basic Shapes

```c
ps_context* ctx = ps_context_create(canvas, NULL);
ps_color stroke_color = {0.2f, 0.2f, 0.2f, 1.0f};
ps_set_stroke_color(ctx, &stroke_color);
ps_set_line_width(ctx, 2.0f);

/* 1. Rectangle */
ps_color c1 = {0.53f, 0.81f, 0.92f, 1.0f};
ps_set_source_color(ctx, &c1);
ps_rect r1 = {20, 20, 100, 70};
ps_rectangle(ctx, &r1);
ps_paint(ctx);

/* 2. Rounded rectangle */
ps_color c2 = {1.0f, 0.71f, 0.76f, 1.0f};
ps_set_source_color(ctx, &c2);
ps_rect r2 = {140, 20, 100, 70};
ps_rounded_rect(ctx, &r2, 12,12, 12,12, 12,12, 12,12);
ps_paint(ctx);

/* 3. Ellipse */
ps_color c3 = {0.56f, 0.93f, 0.56f, 1.0f};
ps_set_source_color(ctx, &c3);
ps_rect r3 = {260, 20, 100, 70};
ps_ellipse(ctx, &r3);
ps_paint(ctx);

/* 4. Triangle */
ps_color c4 = {1.0f, 0.84f, 0.0f, 1.0f};
ps_set_source_color(ctx, &c4);
ps_point p1 = {70, 110}, p2 = {20, 190}, p3 = {120, 190};
ps_move_to(ctx, &p1);
ps_line_to(ctx, &p2);
ps_line_to(ctx, &p3);
ps_close_path(ctx);
ps_paint(ctx);

/* 5. Bézier curve */
ps_color c5 = {0.58f, 0.44f, 0.86f, 1.0f};
ps_set_stroke_color(ctx, &c5);
ps_set_line_width(ctx, 3.0f);
ps_point bp0={150,190}, bp1={180,110}, bp2={220,190}, bp3={250,110};
ps_move_to(ctx, &bp0);
ps_bezier_to(ctx, &bp1, &bp2, &bp3);
ps_stroke(ctx);
```

Visual Result:

<svg width="380" height="210" xmlns="http://www.w3.org/2000/svg">
  <rect x="20" y="20" width="100" height="70" fill="#87CEEB" stroke="#333" stroke-width="2"/>
  <rect x="140" y="20" width="100" height="70" rx="12" ry="12" fill="#FFB6C1" stroke="#333" stroke-width="2"/>
  <ellipse cx="310" cy="55" rx="50" ry="35" fill="#90EE90" stroke="#333" stroke-width="2"/>
  <polygon points="70,110 20,190 120,190" fill="#FFD700" stroke="#333" stroke-width="2"/>
  <path d="M150,190 C180,110 220,190 250,110" fill="none" stroke="#9370DB" stroke-width="3"/>
</svg>

---

## 5. Drawing Operations

Picasso provides four basic drawing operations that act on the current path in the context.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_stroke(ctx)` | Stroke the current path |
| `ps_fill(ctx)` | Fill the current path |
| `ps_paint(ctx)` | Fill and stroke simultaneously |
| `ps_clear(ctx)` | Clear the entire canvas with the current fill color |

### Fill Rules

Set via `ps_set_fill_rule(ctx, rule)`:

| Rule | Description |
|------|-------------|
| `FILL_RULE_WINDING` | Non-zero winding rule (default) |
| `FILL_RULE_EVEN_ODD` | Even-odd rule |

### Example: Fill Rule Comparison

```c
/* Create a five-pointed star path */
ps_path* star = ps_path_create();
ps_point pts[5] = {
    {100, 10}, {40, 190}, {190, 70}, {10, 70}, {160, 190}
};
ps_path_move_to(star, &pts[0]);
int i;
for (i = 1; i < 5; i++) ps_path_line_to(star, &pts[i]);
ps_path_sub_close(star);

/* Non-zero winding rule */
ps_set_fill_rule(ctx, FILL_RULE_WINDING);
ps_set_path(ctx, star);
ps_fill(ctx);

/* Even-odd rule (offset 200px) */
ps_set_fill_rule(ctx, FILL_RULE_EVEN_ODD);
ps_set_path(ctx, star);
ps_fill(ctx);

ps_path_unref(star);
```

Visual Result:

<svg width="400" height="210" xmlns="http://www.w3.org/2000/svg">
  <polygon points="100,10 40,190 190,70 10,70 160,190" fill="#FFD700" fill-opacity="0.8" fill-rule="nonzero" stroke="#333" stroke-width="1"/>
  <text x="60" y="208" font-size="12" fill="#333">Winding</text>
  <polygon points="300,10 240,190 390,70 210,70 360,190" fill="#FFD700" fill-opacity="0.8" fill-rule="evenodd" stroke="#333" stroke-width="1"/>
  <text x="260" y="208" font-size="12" fill="#333">Even-Odd</text>
</svg>

---

## 6. Stroke Style

Stroke controls the appearance of path outlines.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_set_line_width(ctx, width)` | Set line width (returns old value) |
| `ps_set_line_cap(ctx, cap)` | Set line cap style |
| `ps_set_line_join(ctx, join)` | Set line join style |
| `ps_set_miter_limit(ctx, limit)` | Set miter limit |
| `ps_set_line_dash(ctx, start, dashes, n)` | Set dash pattern |
| `ps_reset_line_dash(ctx)` | Clear dash pattern |
| `ps_set_stroke_color(ctx, &color)` | Set stroke color |
| `ps_set_stroke_image/pattern/gradient/canvas` | Set stroke source |

### Line Cap Styles (`ps_line_cap`)

| Value | Description |
|-------|-------------|
| `LINE_CAP_BUTT` | Flat cap (default) |
| `LINE_CAP_ROUND` | Round cap |
| `LINE_CAP_SQUARE` | Square cap (extends by half line width) |

### Line Join Styles (`ps_line_join`)

| Value | Description |
|-------|-------------|
| `LINE_JOIN_MITER` | Miter join (default) |
| `LINE_JOIN_ROUND` | Round join |
| `LINE_JOIN_BEVEL` | Bevel join |

### Example: Line Caps, Joins, and Dashes

```c
ps_set_stroke_color(ctx, &dark);
ps_set_line_width(ctx, 10.0f);

/* Line cap comparison */
ps_set_line_cap(ctx, LINE_CAP_BUTT);
ps_move_to(ctx, &a1); ps_line_to(ctx, &a2); ps_stroke(ctx);

ps_set_line_cap(ctx, LINE_CAP_ROUND);
ps_move_to(ctx, &b1); ps_line_to(ctx, &b2); ps_stroke(ctx);

ps_set_line_cap(ctx, LINE_CAP_SQUARE);
ps_move_to(ctx, &c1); ps_line_to(ctx, &c2); ps_stroke(ctx);

/* Dashes */
float dashes[] = {15.0f, 8.0f, 5.0f, 8.0f};
ps_set_line_dash(ctx, 0, dashes, 4);
ps_move_to(ctx, &d1); ps_line_to(ctx, &d2); ps_stroke(ctx);
ps_reset_line_dash(ctx);

/* Join styles */
ps_set_line_join(ctx, LINE_JOIN_MITER);
/* ... draw V shape ... */
ps_set_line_join(ctx, LINE_JOIN_ROUND);
/* ... draw V shape ... */
```

Visual Result:

<svg width="400" height="160" xmlns="http://www.w3.org/2000/svg">
  <line x1="30" y1="30" x2="170" y2="30" stroke="#333" stroke-width="10" stroke-linecap="butt"/>
  <text x="180" y="35" font-size="11" fill="#666">Butt</text>
  <line x1="30" y1="60" x2="170" y2="60" stroke="#333" stroke-width="10" stroke-linecap="round"/>
  <text x="180" y="65" font-size="11" fill="#666">Round</text>
  <line x1="30" y1="90" x2="170" y2="90" stroke="#333" stroke-width="10" stroke-linecap="square"/>
  <text x="180" y="95" font-size="11" fill="#666">Square</text>
  <line x1="30" y1="130" x2="350" y2="130" stroke="#333" stroke-width="10" stroke-dasharray="15,8,5,8" stroke-linecap="butt"/>
  <text x="10" y="152" font-size="11" fill="#666">Dash: 15,8,5,8</text>
  <polyline points="220,20 250,70 280,20" fill="none" stroke="#E63333" stroke-width="8" stroke-linejoin="miter"/>
  <text x="230" y="88" font-size="10" fill="#666">Miter</text>
  <polyline points="300,20 330,70 360,20" fill="none" stroke="#33B333" stroke-width="8" stroke-linejoin="round"/>
  <text x="310" y="88" font-size="10" fill="#666">Round</text>
</svg>

---

## 7. Fill Source

Picasso supports multiple fill sources, set via the `ps_set_source_*` family of functions.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_set_source_color(ctx, &color)` | Solid color fill |
| `ps_set_source_gradient(ctx, gradient)` | Gradient fill |
| `ps_set_source_pattern(ctx, pattern)` | Pattern fill |
| `ps_set_source_image(ctx, image)` | Image fill |
| `ps_set_source_canvas(ctx, canvas)` | Canvas fill |

### Color Structure

```c
typedef struct _ps_color {
    float r;  /* red component   (0.0 ~ 1.0) */
    float g;  /* green component (0.0 ~ 1.0) */
    float b;  /* blue component  (0.0 ~ 1.0) */
    float a;  /* alpha           (0.0 ~ 1.0) */
} ps_color;
```

### Example: Multiple Fill Sources

```c
/* Solid color fill */
ps_color coral = {1.0f, 0.5f, 0.31f, 1.0f};
ps_set_source_color(ctx, &coral);
ps_rect r1 = {20, 20, 80, 80};
ps_rectangle(ctx, &r1);
ps_fill(ctx);

/* Semi-transparent fill */
ps_color semi = {0.0f, 0.5f, 1.0f, 0.5f};
ps_set_source_color(ctx, &semi);
ps_rect r2 = {60, 40, 80, 80};
ps_rectangle(ctx, &r2);
ps_fill(ctx);
```

Visual Result:

<svg width="200" height="140" xmlns="http://www.w3.org/2000/svg">
  <rect x="20" y="20" width="80" height="80" fill="rgb(255,128,79)"/>
  <rect x="60" y="40" width="80" height="80" fill="rgba(0,128,255,0.5)"/>
</svg>

---

## 8. Gradient

Picasso supports three gradient types: linear, radial, and conic.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_gradient_create_linear(spread, &start, &end)` | Create a linear gradient |
| `ps_gradient_create_radial(spread, &start, sr, &end, er)` | Create a radial gradient |
| `ps_gradient_create_conic(spread, &origin, angle)` | Create a conic gradient |
| `ps_gradient_add_color_stop(grad, offset, &color)` | Add a color stop |
| `ps_gradient_clear_color_stops(grad)` | Clear all color stops |
| `ps_gradient_transform(grad, &matrix)` | Apply a transform to the gradient |
| `ps_gradient_ref/unref` | Reference counting |

### Spread Modes (`ps_gradient_spread`)

| Value | Description |
|-------|-------------|
| `GRADIENT_SPREAD_PAD` | Extend edge color (default) |
| `GRADIENT_SPREAD_REPEAT` | Repeat |
| `GRADIENT_SPREAD_REFLECT` | Reflect |

### Example: Three Gradient Types

```c
/* 1. Linear gradient */
ps_point ls = {20, 20}, le = {180, 20};
ps_gradient* lg = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &ls, &le);
ps_color lc0 = {1, 0, 0, 1}, lc1 = {0, 0, 1, 1};
ps_gradient_add_color_stop(lg, 0.0f, &lc0);
ps_gradient_add_color_stop(lg, 1.0f, &lc1);
ps_set_source_gradient(ctx, lg);
ps_rect lr = {20, 20, 160, 80};
ps_rectangle(ctx, &lr);
ps_fill(ctx);
ps_gradient_unref(lg);

/* 2. Radial gradient */
ps_point rc = {280, 60};
ps_gradient* rg = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &rc, 0, &rc, 60);
ps_color rc0 = {1, 1, 0, 1}, rc1 = {1, 0.27f, 0, 1};
ps_gradient_add_color_stop(rg, 0.0f, &rc0);
ps_gradient_add_color_stop(rg, 1.0f, &rc1);
ps_set_source_gradient(ctx, rg);
ps_rect rr = {220, 10, 120, 100};
ps_rectangle(ctx, &rr);
ps_fill(ctx);
ps_gradient_unref(rg);

/* 3. Conic gradient */
ps_point cc = {420, 60};
ps_gradient* cg = ps_gradient_create_conic(GRADIENT_SPREAD_PAD, &cc, 0);
ps_color cc0={1,0,0,1}, cc1={0,1,0,1}, cc2={0,0,1,1}, cc3={1,0,0,1};
ps_gradient_add_color_stop(cg, 0.0f, &cc0);
ps_gradient_add_color_stop(cg, 0.33f, &cc1);
ps_gradient_add_color_stop(cg, 0.66f, &cc2);
ps_gradient_add_color_stop(cg, 1.0f, &cc3);
ps_set_source_gradient(ctx, cg);
ps_rect cr = {370, 10, 100, 100};
ps_ellipse(ctx, &cr);
ps_fill(ctx);
ps_gradient_unref(cg);
```

Visual Result:

<svg width="500" height="130" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <linearGradient id="lg1" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="red"/>
      <stop offset="100%" stop-color="blue"/>
    </linearGradient>
    <radialGradient id="rg1" cx="50%" cy="50%" r="50%">
      <stop offset="0%" stop-color="yellow"/>
      <stop offset="100%" stop-color="#FF4500"/>
    </radialGradient>
  </defs>
  <rect x="20" y="10" width="160" height="80" fill="url(#lg1)"/>
  <text x="60" y="108" font-size="11" fill="#333">Linear</text>
  <rect x="220" y="10" width="120" height="100" fill="url(#rg1)"/>
  <text x="250" y="125" font-size="11" fill="#333">Radial</text>
  <text x="390" y="65" font-size="11" fill="#333">Conic</text>
  <circle cx="420" cy="60" r="50" fill="url(#rg1)" stroke="#ccc" stroke-width="1"/>
</svg>

---

## 9. Pattern

Patterns use images as repeating fill sources.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_pattern_create_image(img, x_wrap, y_wrap, &matrix)` | Create a pattern from an image |
| `ps_pattern_transform(pattern, &matrix)` | Apply a transform to the pattern |
| `ps_pattern_ref/unref` | Reference counting |

### Tiling Modes (`ps_wrap_type`)

| Value | Description |
|-------|-------------|
| `WRAP_TYPE_REPEAT` | Repeat tiling |
| `WRAP_TYPE_REFLECT` | Reflect tiling |
| `WRAP_TYPE_CLAMP` | Edge clamping |

### Example

```c
ps_image* tile = ps_image_create(COLOR_FORMAT_RGBA, 20, 20);
/* ... fill checkerboard pixel data ... */

ps_pattern* pat = ps_pattern_create_image(tile, WRAP_TYPE_REPEAT,
                                          WRAP_TYPE_REPEAT, NULL);
ps_set_source_pattern(ctx, pat);
ps_rect area = {20, 20, 200, 150};
ps_rectangle(ctx, &area);
ps_fill(ctx);

ps_pattern_unref(pat);
ps_image_unref(tile);
```

Visual Result:

<svg width="240" height="190" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <pattern id="checker" x="0" y="0" width="20" height="20" patternUnits="userSpaceOnUse">
      <rect width="10" height="10" fill="#ccc"/>
      <rect x="10" y="10" width="10" height="10" fill="#ccc"/>
      <rect x="10" width="10" height="10" fill="#fff"/>
      <rect y="10" width="10" height="10" fill="#fff"/>
    </pattern>
  </defs>
  <rect x="20" y="20" width="200" height="150" fill="url(#checker)" stroke="#999" stroke-width="1"/>
</svg>

---

## 10. Matrix Transform

Affine transformation matrices are used for translation, rotation, scaling, and skewing of graphics.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_matrix_create()` | Create an identity matrix |
| `ps_matrix_create_init(sx,shy,shx,sy,tx,ty)` | Create a matrix with specified values |
| `ps_matrix_create_copy(m)` | Copy a matrix |
| `ps_matrix_translate(m, tx, ty)` | Translate |
| `ps_matrix_rotate(m, angle)` | Rotate (radians) |
| `ps_matrix_scale(m, sx, sy)` | Scale |
| `ps_matrix_shear(m, shx, shy)` | Shear |
| `ps_matrix_invert(m)` | Invert |
| `ps_matrix_identity(m)` | Reset to identity matrix |
| `ps_matrix_multiply(result, a, b)` | Matrix multiplication |
| `ps_matrix_transform_point(m, &pt)` | Transform a point |
| `ps_matrix_transform_rect(m, &rect)` | Transform a rectangle |
| `ps_matrix_transform_path(m, path)` | Transform a path |
| `ps_matrix_ref/unref` | Reference counting |

### Context Transform Functions

| Function | Description |
|----------|-------------|
| `ps_set_matrix(ctx, &matrix)` | Set the current transform matrix |
| `ps_translate(ctx, tx, ty)` | Translate the context |
| `ps_rotate(ctx, angle)` | Rotate the context |
| `ps_scale(ctx, sx, sy)` | Scale the context |
| `ps_shear(ctx, shx, shy)` | Shear the context |
| `ps_identity(ctx)` | Reset to identity matrix |
| `ps_world_to_viewport(ctx, &pt)` | Convert world coordinates to viewport coordinates |
| `ps_viewport_to_world(ctx, &pt)` | Convert viewport coordinates to world coordinates |

### Example: Combined Transforms

```c
ps_color blue = {0.26f, 0.52f, 0.96f, 0.7f};
ps_set_source_color(ctx, &blue);
ps_rect r = {0, 0, 60, 40};

/* 1. No transform */
ps_save(ctx);
ps_translate(ctx, 50, 50);
ps_rectangle(ctx, &r); ps_paint(ctx);
ps_restore(ctx);

/* 2. Rotate 30 degrees */
ps_save(ctx);
ps_translate(ctx, 180, 50);
ps_rotate(ctx, 0.524f);
ps_rectangle(ctx, &r); ps_paint(ctx);
ps_restore(ctx);

/* 3. Scale 1.5x */
ps_save(ctx);
ps_translate(ctx, 300, 50);
ps_scale(ctx, 1.5f, 1.0f);
ps_rectangle(ctx, &r); ps_paint(ctx);
ps_restore(ctx);

/* 4. Shear */
ps_save(ctx);
ps_translate(ctx, 50, 140);
ps_shear(ctx, 0.3f, 0.0f);
ps_rectangle(ctx, &r); ps_paint(ctx);
ps_restore(ctx);
```

Visual Result:

<svg width="420" height="200" xmlns="http://www.w3.org/2000/svg">
  <rect x="50" y="50" width="60" height="40" fill="rgba(66,133,244,0.7)" stroke="#1A1A66" stroke-width="2"/>
  <text x="55" y="108" font-size="10" fill="#333">Original</text>
  <g transform="translate(180,50) rotate(30)">
    <rect width="60" height="40" fill="rgba(66,133,244,0.7)" stroke="#1A1A66" stroke-width="2"/>
  </g>
  <text x="170" y="108" font-size="10" fill="#333">Rotate 30</text>
  <g transform="translate(300,50) scale(1.5,1)">
    <rect width="60" height="40" fill="rgba(66,133,244,0.7)" stroke="#1A1A66" stroke-width="2"/>
  </g>
  <text x="310" y="108" font-size="10" fill="#333">Scale 1.5x</text>
  <g transform="translate(50,140) skewX(17)">
    <rect width="60" height="40" fill="rgba(66,133,244,0.7)" stroke="#1A1A66" stroke-width="2"/>
  </g>
  <text x="55" y="198" font-size="10" fill="#333">Skew X</text>
</svg>

---

## 11. Clipping

Clipping restricts the visible area of subsequent drawing operations.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_clip(ctx)` | Clip using the current path |
| `ps_clip_path(ctx, path, rule)` | Clip using a specified path |
| `ps_clip_rect(ctx, &rect)` | Rectangular clip |
| `ps_clip_rects(ctx, rects, n)` | Multi-rectangle clip |
| `ps_scissor_rect(ctx, &rect)` | Scissor clip (hard boundary) |
| `ps_reset_clip(ctx)` | Clear the clipping region |

### Example: Circular Clipping

```c
ps_save(ctx);

/* Set a circular clipping region */
ps_rect clip_r = {50, 20, 120, 120};
ps_ellipse(ctx, &clip_r);
ps_clip(ctx);

/* Draw a gradient rectangle inside the clipping region */
ps_point gs = {50, 20}, ge = {170, 140};
ps_gradient* g = ps_gradient_create_linear(GRADIENT_SPREAD_PAD, &gs, &ge);
ps_color g0 = {1,0.4f,0,1}, g1 = {0.6f,0,0.8f,1};
ps_gradient_add_color_stop(g, 0, &g0);
ps_gradient_add_color_stop(g, 1, &g1);
ps_set_source_gradient(ctx, g);
ps_rect full = {0, 0, 300, 200};
ps_rectangle(ctx, &full);
ps_fill(ctx);
ps_gradient_unref(g);

ps_restore(ctx);  /* clipping region is restored with state */
```

Visual Result:

<svg width="220" height="160" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <clipPath id="circleClip">
      <ellipse cx="110" cy="80" rx="60" ry="60"/>
    </clipPath>
    <linearGradient id="clipGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#FF6600"/>
      <stop offset="100%" stop-color="#9900CC"/>
    </linearGradient>
  </defs>
  <rect width="220" height="160" fill="url(#clipGrad)" clip-path="url(#circleClip)"/>
  <ellipse cx="110" cy="80" rx="60" ry="60" fill="none" stroke="#999" stroke-width="1" stroke-dasharray="4,3"/>
</svg>

---

## 12. Image

Image objects store pixel data and can be used as fill sources or drawn onto a canvas.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_image_create(fmt, w, h)` | Create an empty image |
| `ps_image_create_with_data(data, fmt, w, h, pitch)` | Create using external memory (shared) |
| `ps_image_create_from_data(data, fmt, w, h, pitch)` | Create using external memory (copied) |
| `ps_image_create_compatible(canvas, w, h)` | Create a compatible image |
| `ps_image_create_from_canvas(canvas, &rect)` | Create from a canvas |
| `ps_image_create_from_image(img, &rect)` | Create from an image sub-region |
| `ps_image_set_allow_transparent(img, allow)` | Enable transparency support |
| `ps_image_set_transparent_color(img, &color)` | Set the transparent color |
| `ps_image_get_size(img, &size)` | Get dimensions |
| `ps_image_get_format(img)` | Get format |
| `ps_image_ref/unref` | Reference counting |

### Example: Image as Fill Source

```c
/* Create a 50x50 checkerboard image */
int iw = 50, ih = 50, ip = iw * 4;
ps_byte* idata = (ps_byte*)malloc(ip * ih);
int x, y;
for (y = 0; y < ih; y++) {
    for (x = 0; x < iw; x++) {
        int idx = (y * ip) + (x * 4);
        int checker = ((x / 10) + (y / 10)) % 2;
        idata[idx+0] = checker ? 200 : 60;
        idata[idx+1] = checker ? 200 : 60;
        idata[idx+2] = checker ? 220 : 80;
        idata[idx+3] = 255;
    }
}
ps_image* img = ps_image_create_from_data(idata, COLOR_FORMAT_RGBA, iw, ih, ip);
free(idata);

ps_set_source_image(ctx, img);
ps_rect area = {20, 20, 50, 50};
ps_rectangle(ctx, &area);
ps_fill(ctx);

ps_image_unref(img);
```

---

## 13. Mask

A mask is an 8-bit grayscale buffer used to control the transparency of drawing operations.

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_mask_create(w, h)` | Create a mask |
| `ps_mask_create_with_data(data, w, h)` | Create using external data |
| `ps_mask_add_color_filter(mask, &color)` | Add a color filter |
| `ps_mask_clear_color_filters(mask)` | Clear color filters |
| `ps_mask_ref/unref` | Reference counting |
| `ps_canvas_set_mask(canvas, mask)` | Apply a mask to a canvas |
| `ps_canvas_reset_mask(canvas)` | Remove the mask |

### Example

```c
int mw = 200, mh = 150;
ps_byte* mdata = (ps_byte*)malloc(mw * mh);
int mx, my;
for (my = 0; my < mh; my++)
    for (mx = 0; mx < mw; mx++)
        mdata[my * mw + mx] = (ps_byte)(mx * 255 / mw);

ps_mask* mask = ps_mask_create_with_data(mdata, mw, mh);
ps_canvas_set_mask(canvas, mask);

ps_color red = {1, 0, 0, 1};
ps_set_source_color(ctx, &red);
ps_rect r = {0, 0, 200, 150};
ps_rectangle(ctx, &r);
ps_fill(ctx);

ps_canvas_reset_mask(canvas);
ps_mask_unref(mask);
free(mdata);
```

Visual Result:

<svg width="220" height="170" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <linearGradient id="maskGrad" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="white" stop-opacity="0"/>
      <stop offset="100%" stop-color="white" stop-opacity="1"/>
    </linearGradient>
    <mask id="alphaMask">
      <rect width="200" height="150" fill="url(#maskGrad)"/>
    </mask>
  </defs>
  <rect x="10" y="10" width="200" height="150" fill="#eee" stroke="#ccc"/>
  <rect x="10" y="10" width="200" height="150" fill="red" mask="url(#alphaMask)"/>
  <text x="60" y="180" font-size="11" fill="#666">Gradient mask: left to right</text>
</svg>

---

## 14. Font & Text

### Key Functions

| Function | Description |
|----------|-------------|
| `ps_font_create(name, charset, size, weight, italic)` | Create a font |
| `ps_font_create_copy(font)` | Copy a font |
| `ps_font_set_size/weight/italic/charset/hint/flip` | Modify font properties |
| `ps_font_ref/unref` | Reference counting |
| `ps_set_font(ctx, font)` | Set the current font |
| `ps_set_text_color(ctx, &color)` | Set text color |
| `ps_set_text_stroke_color(ctx, &color)` | Set text stroke color |
| `ps_set_text_kerning(ctx, kerning)` | Set kerning adjustment |
| `ps_text_transform(ctx, &matrix)` | Text transform |
| `ps_text_out_length(ctx, x, y, text, len)` | Output text |
| `ps_wide_text_out_length(ctx, x, y, text, len)` | Output wide-character text |
| `ps_get_text_extent(ctx, text, len, &size)` | Get text dimensions |
| `ps_draw_text(ctx, &area, text, len, type, align)` | Draw text within an area |

### Character Sets (`ps_charset`)

| Value | Description |
|-------|-------------|
| `CHARSET_ANSI` | ANSI |
| `CHARSET_UNICODE` | Unicode |
| `CHARSET_LATIN1` | Latin-1 (default) |

### Example

```c
ps_font* font = ps_font_create("Arial", CHARSET_LATIN1, 24, 400, False);
ps_set_font(ctx, font);

ps_color text_color = {0.1f, 0.1f, 0.1f, 1.0f};
ps_set_text_color(ctx, &text_color);
ps_text_out_length(ctx, 20, 40, "Hello, Picasso!", 15);

/* Bold, larger size */
ps_font_set_size(font, 36);
ps_font_set_weight(font, 700);
ps_set_font(ctx, font);
ps_color blue = {0.2f, 0.4f, 0.8f, 1.0f};
ps_set_text_color(ctx, &blue);
ps_text_out_length(ctx, 20, 90, "Bold Text", 9);

ps_font_unref(font);
```

Visual Result:

<svg width="300" height="120" xmlns="http://www.w3.org/2000/svg">
  <text x="20" y="40" font-family="Arial" font-size="24" fill="#1A1A1A">Hello, Picasso!</text>
  <text x="20" y="90" font-family="Arial" font-size="36" font-weight="bold" fill="#3366CC">Bold Text</text>
</svg>

---

## 15. Extension: Image Loading

The image loading extension provides multi-format image file read/write capabilities.

### Key Functions

| Function | Description |
|----------|-------------|
| `psx_image_init()` | Initialize the image extension |
| `psx_image_shutdown()` | Shut down the image extension |
| `psx_image_load(filename, &err)` | Load an image from a file |
| `psx_image_load_from_memory(data, len, &err)` | Load an image from memory |
| `psx_image_create_from_data(data, fmt, w, h, pitch, &err)` | Create from raw data |
| `psx_image_save(img, func, param, type, quality)` | Save via callback |
| `psx_image_save_to_file(img, filename, type, quality)` | Save to a file |
| `psx_image_destroy(img)` | Destroy the image |

### Example: Load and Display an Image

```c
#include "psx_image.h"

psx_image_init();

psx_result err;
psx_image* img = psx_image_load("photo.png", &err);
if (!img) {
    printf("Load failed: %s\n", psx_result_get_string(err));
    return;
}

ps_image* ps_img = IMG_OBJ(img);
ps_set_source_image(ctx, ps_img);
ps_rect dst = {10, 10, (float)img->width, (float)img->height};
ps_rectangle(ctx, &dst);
ps_fill(ctx);

psx_image_save_to_file(img, "output.jpg", "jpg", 0.85f);

psx_image_destroy(img);
psx_image_shutdown();
```

### Multi-Frame Images (GIF)

```c
psx_image* gif = psx_image_load("animation.gif", NULL);
size_t i;
for (i = 0; i < gif->num_frames; i++) {
    ps_image* frame = IMG_OBJ_AT_INDEX(gif, i);
    int duration_ms = IMG_DURATION_AT_INDEX(gif, i);
    /* display frame and wait duration_ms milliseconds */
}
psx_image_destroy(gif);
```

---

## 16. Extension: SVG Rendering

The SVG extension provides parsing and rendering of SVG Tiny 1.2 documents.

### Key Functions

| Function | Description |
|----------|-------------|
| `psx_svg_init()` | Initialize the SVG extension |
| `psx_svg_shutdown()` | Shut down the SVG extension |
| `psx_svg_load(filename, &err)` | Load an SVG from a file |
| `psx_svg_load_from_memory(data, len, &err)` | Load an SVG from memory |
| `psx_svg_destroy(doc)` | Destroy the SVG document |
| `psx_svg_render_create(doc, &err)` | Create a renderer |
| `psx_svg_render_destroy(render)` | Destroy the renderer |
| `psx_svg_render_get_size(render, &size)` | Get SVG dimensions |
| `psx_svg_render_draw(ctx, render)` | Render to the context |

### Complete Example

```c
#include "picasso.h"
#include "psx_svg.h"

ps_initialize();
psx_svg_init();

psx_result err;
psx_svg* doc = psx_svg_load("tiger.svg", &err);
psx_svg_render* render = psx_svg_render_create(doc, NULL);

ps_size svg_size;
psx_svg_render_get_size(render, &svg_size);

int w = (int)svg_size.w, h = (int)svg_size.h;
ps_canvas* canvas = ps_canvas_create(COLOR_FORMAT_RGBA, w, h);
ps_context* ctx = ps_context_create(canvas, NULL);

ps_color white = {1, 1, 1, 1};
ps_set_source_color(ctx, &white);
ps_clear(ctx);

psx_svg_render_draw(ctx, render);

psx_svg_render_destroy(render);
psx_svg_destroy(doc);
ps_context_unref(ctx);
ps_canvas_unref(canvas);
psx_svg_shutdown();
ps_shutdown();
```

---

## 17. Extension: SVG Animation

The SVG animation player supports the SVG Tiny 1.2 animation specification, including `<animate>`, `<set>`, `<animateColor>`, `<animateTransform>`, and `<animateMotion>`.

### Key Functions

| Function | Description |
|----------|-------------|
| `psx_svg_player_create(doc, &err)` | Create an animation player |
| `psx_svg_player_destroy(player)` | Destroy the player |
| `psx_svg_player_play(player)` | Start playback |
| `psx_svg_player_pause(player)` | Pause |
| `psx_svg_player_stop(player)` | Stop |
| `psx_svg_player_tick(player, dt)` | Advance time (seconds) |
| `psx_svg_player_seek(player, time)` | Seek to a specific time |
| `psx_svg_player_draw(player, ctx)` | Render the current frame |
| `psx_svg_player_get_state(player)` | Get playback state |
| `psx_svg_player_get_duration(player)` | Get total duration |
| `psx_svg_player_set_loop(player, loop)` | Set looping |
| `psx_svg_player_set_dpi(player, dpi)` | Set DPI |
| `psx_svg_player_trigger(player, id, event)` | Trigger an event |
| `psx_svg_player_send_key(player, key)` | Send a key press |
| `psx_svg_player_set_event_callback(player, cb, data)` | Set event callback |

### Player States

| State | Description |
|-------|-------------|
| `PSX_SVG_PLAYER_STOPPED` | Stopped |
| `PSX_SVG_PLAYER_PLAYING` | Playing |
| `PSX_SVG_PLAYER_PAUSED` | Paused |

### Complete Example: Animation Playback Loop

```c
#include "psx_svg_animation.h"

psx_svg* doc = psx_svg_load("animated.svg", NULL);
psx_svg_player* player = psx_svg_player_create(doc, NULL);

psx_svg_player_set_loop(player, true);
psx_svg_player_set_dpi(player, 96);

/* Event callback */
void on_anim_event(psx_svg_anim_event_type type, const char* id, void* data) {
    if (type == PSX_SVG_ANIM_EVENT_BEGIN)
        printf("Animation '%s' started\n", id ? id : "unnamed");
    else if (type == PSX_SVG_ANIM_EVENT_END)
        printf("Animation '%s' ended\n", id ? id : "unnamed");
}
psx_svg_player_set_event_callback(player, on_anim_event, NULL);

psx_svg_player_play(player);

/* Render loop */
while (running) {
    float dt = get_frame_delta_time();
    psx_svg_player_tick(player, dt);

    ps_color bg = {1, 1, 1, 1};
    ps_set_source_color(ctx, &bg);
    ps_clear(ctx);

    psx_svg_player_draw(player, ctx);
    present_canvas(canvas);
}

psx_svg_player_destroy(player);
psx_svg_destroy(doc);
```

### Interactive Control

```c
/* Seek to 2 seconds */
psx_svg_player_seek(player, 2.0f);

/* Pause / resume */
if (psx_svg_player_get_state(player) == PSX_SVG_PLAYER_PLAYING)
    psx_svg_player_pause(player);
else
    psx_svg_player_play(player);

/* Trigger an indefinite animation */
psx_svg_player_trigger(player, "myAnim", "click");

/* Send an accessKey */
psx_svg_player_send_key(player, 's');
```

Visual Result (SVG animation example):

<svg width="200" height="200" xmlns="http://www.w3.org/2000/svg">
  <rect x="75" y="75" width="50" height="50" fill="#4285F4" rx="5">
    <animateTransform attributeName="transform" type="rotate"
      from="0 100 100" to="360 100 100" dur="3s" repeatCount="indefinite"/>
  </rect>
  <circle cx="100" cy="100" r="10" fill="red">
    <animate attributeName="r" from="10" to="40" dur="2s" repeatCount="indefinite"/>
    <animate attributeName="fill" from="red" to="blue" dur="2s" repeatCount="indefinite"/>
  </circle>
</svg>

---

> This programming guide is based on the Picasso v2.9.0 API. Full API reference at:
> http://onecoolx.github.io/picasso/html/modules.html
