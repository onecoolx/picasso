"use strict";

const ENV_IS_WEB = typeof window == 'object';
const ENV_IS_WORKER = typeof importSctipts == 'function';

const wasmImports = {
    proc_exit: ()=>{},
    fd_close: (fd)=>{},
    fd_seek:  (fd, ofl, ofh, w, nof)=>{},
    fd_write: (fd, iov, iovn, pn)=>{},
    emscripten_resize_heap: (d,s,n)=>{},
    emscripten_memcpy_js: (s)=>{},
    emscripten_notify_memory_growth: (s) =>{},
    abort: ()=>{},
};

const info = {
    'env': wasmImports,
    'wasi_snapshot_preview1': wasmImports
};

function _getExportWrapper(instance, name) {
    return instance.exports[name];
}

function _createDataBuffer(vm, bytes) {
    return vm._malloc(bytes);
}

function _createPointBuffer(vm, pt, y) {
    let cv = _createDataBuffer(vm, 8);
    let buffer = vm._HEAPF32.subarray((cv>>2), (cv>>2) + 2);

    if (y !== undefined) {
        buffer[0] = pt;
        buffer[1] = y;
    } else if (typeof pt === "object") {
        if (typeof pt.x !== "number" ||
                typeof pt.y !== "number") {
            throw TypeError("Rect value object must be {x:<number>, y:<number>}");
        }
        buffer[0] = typeof pt.x === "number" ? pt.x : 0.0;
        buffer[1] = typeof pt.y === "number" ? pt.y : 0.0;
    } else {
        throw TypeError("Point values is not numbers or object!");
    }
    return cv;
}

function _createRectBuffer(vm, rc, y, w, h) {
    let cv = _createDataBuffer(vm, 16);
    let buffer = vm._HEAPF32.subarray((cv>>2), (cv>>2) + 4);
    if (y !== undefined && w !== undefined && h !== undefined) {
        buffer[0] = rc;
        buffer[1] = y;
        buffer[2] = w;
        buffer[3] = h;
    } else if (typeof rc === "object") {
        if (typeof rc.x !== "number" ||
                typeof rc.y !== "number" ||
                typeof rc.w !== "number" ||
                typeof rc.h !== "number") {
            throw TypeError("Rect value object must be {x:<number>, y:<number>, w:<number>, h:<number>}");
        }
        buffer[0] = typeof rc.x === "number" ? rc.x : 0.0;
        buffer[1] = typeof rc.y === "number" ? rc.y : 0.0;
        buffer[2] = typeof rc.w === "number" ? rc.w : 0.0;
        buffer[3] = typeof rc.h === "number" ? rc.h : 0.0;
    } else {
        throw TypeError("Rect values is not numbers or object!");
    }
    return cv;
}

function _createColorBuffer(vm, color, g, b, a) {
    let cv = _createDataBuffer(vm, 16);
    let buffer = vm._HEAPF32.subarray((cv>>2), (cv>>2) + 4);
    if (g !== undefined && b !== undefined && a !== undefined) {
        buffer[0] = (color <= 1.0 && color >= 0.0) ? color : color / 255.0;
        buffer[1] = (g <= 1.0 && g >= 0.0) ? g : g / 255.0;
        buffer[2] = (b <= 1.0 && b >= 0.0) ? b : b / 255.0;
        buffer[3] = (a <= 1.0 && a >= 0.0) ? a : a / 255.0;
    } else if (typeof color === "number") {
        buffer[0] = (color >>> 24) / 255.0;
        buffer[1] = (color >> 16 & 0xFF) / 255.0;
        buffer[2] = (color >> 8 & 0xFF) / 255.0;
        buffer[3] = (color & 0xFF) / 255.0;
    } else if (typeof color === "string") {
        color = color.trim().toLowerCase();
        if (color.charCodeAt(0) === 35) {
            let hex = color.substring(1);
            let len = hex.length;
            if (len === 8) {
                let rgba = parseInt(hex, 16);
                buffer[0] = (rgba >>> 24) / 255.0;
                buffer[1] = (rgba >> 16 & 0xFF) / 255.0;
                buffer[2] = (rgba >> 8 & 0xFF) / 255.0;
                buffer[3] = (rgba & 0xFF) / 255.0;
            } else if (len === 6) {
                hex += "FF";
                let rgba = parseInt(hex, 16);
                buffer[0] = (rgba >>> 24) / 255.0;
                buffer[1] = (rgba >> 16 & 0xFF) / 255.0;
                buffer[2] = (rgba >> 8 & 0xFF) / 255.0;
                buffer[3] = (rgba & 0xFF) / 255.0;
            } else {
                throw TypeError("Color value string must be #<XXXXXXXX> or #<XXXXXX>");
            }
        } else {
            throw TypeError("Color value string must be #<XXXXXXXX> or #<XXXXXX>");
        }
    } else if (typeof color === "object") {
        if (typeof color.r !== "number" ||
            typeof color.g !== "number" ||
            typeof color.b !== "number" ||
            typeof color.a !== "number") {
            throw TypeError("Color value object must be {r:<number>, g:<number>, b:<number>, a:<number>}");
        }
        buffer[0] = typeof color.r === "number" ? color.r : 0.0;
        buffer[1] = typeof color.g === "number" ? color.g : 0.0;
        buffer[2] = typeof color.b === "number" ? color.b : 0.0;
        buffer[3] = typeof color.a === "number" ? color.a : 0.0;
    } else {
        throw TypeError("Color value is not a number, string or object!");
    }
    return cv;
}

function _destoryBuffer(vm, b) {
    vm._free(b);
}

class Context {
    constructor(ps) {
        this._ps = ps;
        let instance = ps._instance;
        this._ps_text_out_length = _getExportWrapper(instance, 'ps_text_out_length');
        this._ps_wide_text_out_length = _getExportWrapper(instance, 'ps_wide_text_out_length');
        this._ps_draw_text = _getExportWrapper(instance, 'ps_draw_text');
        this._ps_get_text_extent = _getExportWrapper(instance, 'ps_get_text_extent');
        this._ps_show_glyphs = _getExportWrapper(instance, 'ps_show_glyphs');
        this._ps_get_path_from_glyph = _getExportWrapper(instance, 'ps_get_path_from_glyph');
        this._ps_get_glyph = _getExportWrapper(instance, 'ps_get_glyph');
        this._ps_glyph_get_extent = _getExportWrapper(instance, 'ps_glyph_get_extent');
        this._ps_get_font_info = _getExportWrapper(instance, 'ps_get_font_info');
        this._ps_set_font = _getExportWrapper(instance, 'ps_set_font');
        this._ps_set_text_render_type = _getExportWrapper(instance, 'ps_set_text_render_type');
        this._ps_set_text_kerning = _getExportWrapper(instance, 'ps_set_text_kerning');
        this._ps_set_text_antialias = _getExportWrapper(instance, 'ps_set_text_antialias');
        this._ps_set_text_color = _getExportWrapper(instance, 'ps_set_text_color');
        this._ps_set_text_stroke_color = _getExportWrapper(instance, 'ps_set_text_stroke_color');
        this._ps_text_transform = _getExportWrapper(instance, 'ps_text_transform');
        this._ps_set_text_matrix = _getExportWrapper(instance, 'ps_set_text_matrix');
        this._ps_mask_create_with_data = _getExportWrapper(instance, 'ps_mask_create_with_data');
        this._ps_mask_ref = _getExportWrapper(instance, 'ps_mask_ref');
        this._ps_mask_unref = _getExportWrapper(instance, 'ps_mask_unref');
        this._ps_mask_add_color_filter = _getExportWrapper(instance, 'ps_mask_add_color_filter');
        this._ps_context_create = _getExportWrapper(instance, 'ps_context_create'); // RED
        this._ps_context_ref = _getExportWrapper(instance, 'ps_context_ref'); // DEL
        this._ps_context_set_canvas = _getExportWrapper(instance, 'ps_context_set_canvas'); // DEL
        this._ps_context_get_canvas = _getExportWrapper(instance, 'ps_context_get_canvas'); // DEL
        this._ps_context_unref = _getExportWrapper(instance, 'ps_context_unref'); // RED
        this._ps_set_source_gradient = _getExportWrapper(instance, 'ps_set_source_gradient');
        this._ps_set_source_pattern = _getExportWrapper(instance, 'ps_set_source_pattern');
        this._ps_set_source_image = _getExportWrapper(instance, 'ps_set_source_image');
        this._ps_set_source_color = _getExportWrapper(instance, 'ps_set_source_color'); // RED
        this._ps_set_source_canvas = _getExportWrapper(instance, 'ps_set_source_canvas');
        this._ps_set_stroke_color = _getExportWrapper(instance, 'ps_set_stroke_color'); // RED
        this._ps_set_stroke_image = _getExportWrapper(instance, 'ps_set_stroke_image');
        this._ps_set_stroke_pattern = _getExportWrapper(instance, 'ps_set_stroke_pattern');
        this._ps_set_stroke_gradient = _getExportWrapper(instance, 'ps_set_stroke_gradient');
        this._ps_set_stroke_canvas = _getExportWrapper(instance, 'ps_set_stroke_canvas');
        this._ps_set_filter = _getExportWrapper(instance, 'ps_set_filter');
        this._ps_stroke = _getExportWrapper(instance, 'ps_stroke'); // RED
        this._ps_fill = _getExportWrapper(instance, 'ps_fill'); // RED
        this._ps_paint = _getExportWrapper(instance, 'ps_paint'); // RED
        this._ps_clear = _getExportWrapper(instance, 'ps_clear'); // RED
        this._ps_set_alpha = _getExportWrapper(instance, 'ps_set_alpha'); // RED
        this._ps_set_blur = _getExportWrapper(instance, 'ps_set_blur'); // RED
        this._ps_set_gamma = _getExportWrapper(instance, 'ps_set_gamma'); // RED
        this._ps_set_antialias = _getExportWrapper(instance, 'ps_set_antialias'); // RED
        this._ps_set_shadow = _getExportWrapper(instance, 'ps_set_shadow'); // RED
        this._ps_set_shadow_color = _getExportWrapper(instance, 'ps_set_shadow_color'); // RED
        this._ps_reset_shadow = _getExportWrapper(instance, 'ps_reset_shadow'); // RED
        this._ps_set_fill_rule = _getExportWrapper(instance, 'ps_set_fill_rule'); // RED
        this._ps_set_line_cap = _getExportWrapper(instance, 'ps_set_line_cap');
        this._ps_set_line_inner_join = _getExportWrapper(instance, 'ps_set_line_inner_join');
        this._ps_set_line_join = _getExportWrapper(instance, 'ps_set_line_join');
        this._ps_set_line_width = _getExportWrapper(instance, 'ps_set_line_width'); // RED
        this._ps_set_miter_limit = _getExportWrapper(instance, 'ps_set_miter_limit'); // RED
        this._ps_set_line_dash = _getExportWrapper(instance, 'ps_set_line_dash'); // RED
        this._ps_reset_line_dash = _getExportWrapper(instance, 'ps_reset_line_dash'); // RED
        this._ps_new_path = _getExportWrapper(instance, 'ps_new_path'); // RED
        this._ps_add_sub_path = _getExportWrapper(instance, 'ps_add_sub_path');
        this._ps_new_sub_path = _getExportWrapper(instance, 'ps_new_sub_path'); // RED
        this._ps_rectangle = _getExportWrapper(instance, 'ps_rectangle'); // RED
        this._ps_rounded_rect = _getExportWrapper(instance, 'ps_rounded_rect'); // RED
        this._ps_ellipse = _getExportWrapper(instance, 'ps_ellipse'); // RED
        this._ps_close_path = _getExportWrapper(instance, 'ps_close_path'); // RED
        this._ps_move_to = _getExportWrapper(instance, 'ps_move_to'); // RED
        this._ps_line_to = _getExportWrapper(instance, 'ps_line_to'); // RED
        this._ps_bezier_curve_to = _getExportWrapper(instance, 'ps_bezier_curve_to'); // RED
        this._ps_quad_curve_to = _getExportWrapper(instance, 'ps_quad_curve_to'); // RED
        this._ps_arc = _getExportWrapper(instance, 'ps_arc'); // RED
        this._ps_tangent_arc = _getExportWrapper(instance, 'ps_tangent_arc'); // RED
        this._ps_set_path = _getExportWrapper(instance, 'ps_set_path');
        this._ps_get_path = _getExportWrapper(instance, 'ps_get_path'); // DEL
        this._ps_translate = _getExportWrapper(instance, 'ps_translate'); // RED
        this._ps_scale = _getExportWrapper(instance, 'ps_scale'); // RED
        this._ps_shear = _getExportWrapper(instance, 'ps_shear'); // RED
        this._ps_rotate = _getExportWrapper(instance, 'ps_rotate'); // RED
        this._ps_identity = _getExportWrapper(instance, 'ps_identity'); // RED
        this._ps_transform = _getExportWrapper(instance, 'ps_transform');
        this._ps_set_matrix = _getExportWrapper(instance, 'ps_set_matrix');
        this._ps_get_matrix = _getExportWrapper(instance, 'ps_get_matrix');
        this._ps_world_to_viewport = _getExportWrapper(instance, 'ps_world_to_viewport');
        this._ps_viewport_to_world = _getExportWrapper(instance, 'ps_viewport_to_world');
        this._ps_set_composite_operator = _getExportWrapper(instance, 'ps_set_composite_operator');
        this._ps_clip = _getExportWrapper(instance, 'ps_clip'); // RED
        this._ps_clip_path = _getExportWrapper(instance, 'ps_clip_path');
        this._ps_clip_device_rect = _getExportWrapper(instance, 'ps_clip_device_rect');
        this._ps_clip_rect = _getExportWrapper(instance, 'ps_clip_rect');
        this._ps_clip_rects = _getExportWrapper(instance, 'ps_clip_rects');
        this._ps_reset_clip = _getExportWrapper(instance, 'ps_reset_clip'); // RED
        this._ps_save = _getExportWrapper(instance, 'ps_save'); // RED
        this._ps_restore = _getExportWrapper(instance, 'ps_restore'); // RED
    }

    create(canvas) {
        this._ctx = this._ps_context_create(canvas._canvas, 0);
    }

    clear() {
        this._ps_clear(this._ctx);
    }

    stroke() {
        this._ps_stroke(this._ctx);
    }

    fill() {
        this._ps_fill(this._ctx);
    }

    paint() {
        this._ps_paint(this._ctx);
    }

    clip() {
        this._ps_clip(this._ctx);
    }

    save() {
        this._ps_save(this._ctx);
    }

    restore() {
        this._ps_restore(this._ctx);
    }

    resetClip() {
        this._ps_reset_clip(this._ctx);
    }

    resetShadow() {
        this._ps_reset_shadow(this._ctx);
    }

    setFillRule(rule) {
        if (typeof rule !== "string") {
            throw TypeError("Parameters must be rule:<string>");
        }

        switch (rule) {
            case "evenodd":
                this._ps_set_fill_rule(this._ctx, 1);
                return;
            case "nonzero":
                this._ps_set_fill_rule(this._ctx, 0);
                return;
        }
    }

    setLineDash(s, dashes) {
        if (typeof s !== "number" || !(dashes instanceof Array)) {
            throw TypeError("Parameters must be start:<number>, dashes:<array>.");
        }
        let n = dashes.length;
        let cv = _createDataBuffer(this._ps, n * 4);
        let buffer = this._ps._HEAPF32.subarray((cv>>2), (cv>>2) + n);
        for (let i = 0; i < n; i++) {
            if (typeof dashes[i] !== "number") {
                throw TypeError("The dashes array must be: array<number>.");
            }
            buffer[i] = dashes[i];
        }
        this._ps_set_line_dash(this._ctx, s, cv, n);
        _destoryBuffer(this._ps, cv);
    }

    resetLineDash() {
        this._ps_reset_line_dash(this._ctx);
    }

    identity() {
        this._ps_identity(this._ctx);
    }

    translate(x, y) {
        this._ps_translate(this._ctx, x, y);
    }

    scale(sx, sy) {
        this._ps_scale(this._ctx, sx, sy);
    }

    shear(shx, shy) {
        this._ps_shear(this._ctx, shx, shy);
    }

    rotate(a) {
        this._ps_rotate(this._ctx, a);
    }

    moveTo(pt/*x*/, y) {
        let cv = _createPointBuffer(this._ps, pt, y);
        this._ps_move_to(this._ctx, cv);
        _destoryBuffer(this._ps, cv);
    }

    lineTo(pt/*x*/, y) {
        let cv = _createPointBuffer(this._ps, pt, y);
        this._ps_line_to(this._ctx, cv);
        _destoryBuffer(this._ps, cv);
    }

    quadCurveTo(cp, ep) {
        let cv1 = _createPointBuffer(this._ps, cp);
        let cv2 = _createPointBuffer(this._ps, ep);
        this._ps_quad_curve_to(this._ctx, cv1, cv2);
        _destoryBuffer(this._ps, cv2);
        _destoryBuffer(this._ps, cv1);
    }

    bezierCurveTo(cp1, cp2, ep) {
        let cv1 = _createPointBuffer(this._ps, cp1);
        let cv2 = _createPointBuffer(this._ps, cp2);
        let cv3 = _createPointBuffer(this._ps, ep);
        this._ps_bezier_curve_to(this._ctx, cv1, cv2, cv3);
        _destoryBuffer(this._ps, cv3);
        _destoryBuffer(this._ps, cv2);
        _destoryBuffer(this._ps, cv1);
    }

    arc(cp, r, sa, ea, c) {
        if (typeof r !== "number" || typeof sa !== "number" || typeof ea !== "number" || typeof c !== "boolean") {
            throw TypeError("Parameters must be radius:<number>, sangle:<number>, eangle:<number>, colckwise:<boolean>");
        }
        let cv = _createPointBuffer(this._ps, cp);
        this._ps_arc(this._ctx, cv, r, sa, ea, c);
        _destoryBuffer(this._ps, cv);
    }

    tangentArc(rc, sa, sw) {
        if (typeof sa !== "number" || typeof sw !== "number") {
            throw TypeError("Parameters must be sangle:<number>, sweep:<number>");
        }
        let cv = _createRectBuffer(this._ps, rc);
        this._ps_tangent_arc(this._ctx, cv, sa, sw);
        _destoryBuffer(this._ps, cv);
    }

    closePath() {
        this._ps_close_path(this._ctx);
    }

    rectangle(rc/* x */, y, w, h) {
        let cv = _createRectBuffer(this._ps, rc, y, w, h);
        this._ps_rectangle(this._ctx, cv);
        _destoryBuffer(this._ps, cv);
    }

    roundedRect(rrc) {
        let cv = _createDataBuffer(this._ps, 16);
        let buffer = this._ps._HEAPF32.subarray((cv>>2), (cv>>2) + 4);
        if (typeof rrc === "object") {
            if (typeof rrc.x !== "number" ||
                    typeof rrc.y !== "number" ||
                    typeof rrc.w !== "number" ||
                    typeof rrc.h !== "number") {
                throw TypeError("RoundedRect value object must be {x:<number>, y:<number>, w:<number>, h:<number>, "+
                       "[optional]ltx:<number>, [optional]lty:<number>, [optional]rtx:<number>, [optional]rty:<number>, "+
                       "[optional]lbx:<numner>, [optional]lby:<number>, [optional]rbx:<number>, [optional]rby:<number>}");
            }
            buffer[0] = typeof rrc.x === "number" ? rrc.x : 0.0;
            buffer[1] = typeof rrc.y === "number" ? rrc.y : 0.0;
            buffer[2] = typeof rrc.w === "number" ? rrc.w : 0.0;
            buffer[3] = typeof rrc.h === "number" ? rrc.h : 0.0;
        } else {
            throw TypeError("Rect values is not numbers or object!");
        }

        let ltx = typeof rrc.ltx === "number" ? rrc.ltx : 0.0;
        let lty = typeof rrc.lty === "number" ? rrc.lty : 0.0;
        let rtx = typeof rrc.rtx === "number" ? rrc.rtx : 0.0;
        let rty = typeof rrc.rty === "number" ? rrc.rty : 0.0;
        let lbx = typeof rrc.lbx === "number" ? rrc.lbx : 0.0;
        let lby = typeof rrc.lby === "number" ? rrc.lby : 0.0;
        let rbx = typeof rrc.rbx === "number" ? rrc.rbx : 0.0;
        let rby = typeof rrc.rby === "number" ? rrc.rby : 0.0;

        this._ps_rounded_rect(this._ctx, cv, ltx, lty, rtx, rty, lbx, lby, rbx, rby);
        _destoryBuffer(this._ps, cv);
    }

    ellipse(rc/* x */, y, w, h) {
        let cv = _createRectBuffer(this._ps, rc, y, w, h);
        this._ps_ellipse(this._ctx, cv);
        _destoryBuffer(this._ps, cv);
    }

    newPath() {
        this._ps_new_path(this._ctx);
    }

    newSubPath() {
        this._ps_new_sub_path(this._ctx);
    }

    setSourceColor(color/* r */, g, b, a) {
        let cv = _createColorBuffer(this._ps, color, g, b, a);
        this._ps_set_source_color(this._ctx, cv);
        _destoryBuffer(this._ps, cv);
    }

    setStrokeColor(color/* r */, g, b, a) {
        let cv = _createColorBuffer(this._ps, color, g, b, a);
        this._ps_set_stroke_color(this._ctx, cv);
        _destoryBuffer(this._ps, cv);
    }

    setShadowColor(color/* r */, g, b, a) {
        let cv = _createColorBuffer(this._ps, color, g, b, a);
        this._ps_set_shadow_color(this._ctx, cv);
        _destoryBuffer(this._ps, cv);
    }

    setShadow(x, y, b) {
        if (typeof x !== "number" || typeof y !== "number" || typeof b !== "number") {
            throw TypeError("Parameters must be <number>");
        }
        this._ps_set_shadow(this._ctx, x, y, b);
    }

    setAlpha(a) {
        if (typeof a !== "number" || a < 0.0 || a > 1.0) {
            throw TypeError("Parameter must be <number> [0 ~ 1] default 1.0");
        }
        this._ps_set_alpha(this._ctx, a);
    }

    setLineWidth(w) {
        if (typeof w !== "number") {
            throw TypeError("Parameter must be <number> [greater than 0] default 1");
        }
        this._ps_set_line_width(this._ctx, w);
    }

    setGamma(g) {
        if (typeof g !== "number" || g < 0.0 || g > 3.0) {
            throw TypeError("Parameter must be <number> [0 ~ 3] default 1.0");
        }
        this._ps_set_gamma(this._ctx, g);
    }

    setBlur(b) {
        if (typeof b !== "number" || b < 0.0 || b > 1.0) {
            throw TypeError("Parameter must be <number> [0 ~ 1] default 0.0");
        }
        this._ps_set_blur(this._ctx, b);
    }

    setMiterLimit(l) {
        if (typeof l !== "number") {
            throw TypeError("Parameter must be <number> [greater than 0] default 4.0");
        }
        this._ps_set_miter_limit(this._ctx, l);
    }

    setAntialias(a) {
        if (typeof a !== "boolean") {
            throw TypeError("Parameter must be <boolean> default true");
        }
        this._ps_set_antialias(this._ctx, a);
    }

    destroy() {
        if (this._ctx != undefined) {
            this._ps_context_unref(this._ctx);
            this._ctx = undefined;
        }
    }
}

class Canvas {
    constructor(ps) {
        this._ps = ps;
        let instance = ps._instance;
        this._ps_canvas_create = _getExportWrapper(instance, 'ps_canvas_create');
        this._ps_canvas_create_compatible = _getExportWrapper(instance, 'ps_canvas_create_compatible');
        this._ps_canvas_create_from_canvas = _getExportWrapper(instance, 'ps_canvas_create_from_canvas');
        this._ps_canvas_ref= _getExportWrapper(instance, 'ps_canvas_ref');
        this._ps_canvas_create_from_image = _getExportWrapper(instance, 'ps_canvas_create_from_image');
        this._ps_canvas_create_with_data = _getExportWrapper(instance, 'ps_canvas_create_with_data');
        this._ps_canvas_replace_data = _getExportWrapper(instance, 'ps_canvas_replace_data');
        this._ps_canvas_unref = _getExportWrapper(instance, 'ps_canvas_unref');
        this._ps_canvas_get_size = _getExportWrapper(instance, 'ps_canvas_get_size');
        this._ps_canvas_get_format = _getExportWrapper(instance, 'ps_canvas_get_format');
        this._ps_canvas_set_mask = _getExportWrapper(instance, 'ps_canvas_set_mask');
        this._ps_canvas_reset_mask = _getExportWrapper(instance, 'ps_canvas_reset_mask');
        this._ps_canvas_bitblt = _getExportWrapper(instance, 'ps_canvas_bitblt');
        this._h5Context = null;
        this._canvas = null;
        this._rawBuffer = null;
        this._targetImg = null;
        this._width = 0;
        this._height = 0;
    }

    create(h5canvas) {
        this._h5Context = h5canvas.getContext("2d");
        this._width = h5canvas.width;
        this._height = h5canvas.height;

        this._rawBuffer = this._ps._malloc(this._width * this._height * 4);
        this._canvas = this._ps_canvas_create_with_data(this._rawBuffer, 0, this._width, this._height, this._width * 4);

        let buffer = this._ps._HEAPU8.subarray(this._rawBuffer, this._rawBuffer + this._width * this._height * 4);
        let img = new Uint8ClampedArray(buffer.buffer, buffer.byteOffset, buffer.length);
        this._targetImg = new ImageData(img, this._width, this._height);
    }

    flush() {
        this._h5Context.putImageData(this._targetImg, 0, 0);
    }

    destroy() {
        if (this._canvas != undefined) {
            this._ps_canvas_unref(this._canvas);
            this._canvas = undefined;
            this._ps._free(this._rawBuffer);
        }
    }
}

class Path2D {

    destroy() {
    }
}

export default class Picasso {
    _setEnv(instance) {
        this._memory = _getExportWrapper(instance, 'memory');
        let b = this._memory.buffer;
        this._HEAP8 = new Int8Array(b);
        this._HEAP16 = new Int16Array(b);
        this._HEAPU8 = new Uint8Array(b);
        this._HEAPU16 = new Uint16Array(b);
        this._HEAP32 = new Int32Array(b);
        this._HEAPU32 = new Uint32Array(b);
        this._HEAPF32 = new Float32Array(b);
        this._HEAPF64 = new Float64Array(b);
    }

    _setupFuncs(instance) {
        this._ps_version = _getExportWrapper(instance, 'ps_version');
        this._ps_initialize = _getExportWrapper(instance, 'ps_initialize');
        this._ps_shutdown = _getExportWrapper(instance, 'ps_shutdown');
        this._ps_last_status = _getExportWrapper(instance, 'ps_last_status');
        this._malloc = _getExportWrapper(instance, 'malloc');
        this._free = _getExportWrapper(instance, 'free');

        this._ps_initialize();
    }

    _init() {
        return fetch("picasso.wasm")
        .then((response) => response.arrayBuffer())
        .then((binary) => {
            let module = new WebAssembly.Module(binary);
            this._instance = new WebAssembly.Instance(module, info);
            this._setEnv(this._instance);
            this._setupFuncs(this._instance);
        });
    }

    constructor() {
        this._h5canvas = null;
    }

    init(canvas, onSuccess, onFail) {
        this._h5canvas = canvas;
        this._init().then(()=>{
            onSuccess(this);
        })
        .catch((e)=>{
            throw e;
            if (onFail != undefined)
                onFail();
        });
    }

    get version() {
        return this._ps_version();
    }

    get lastStatus() {
        return this._ps_last_status();
    }

    get canvas() {
        if (this._ps_canvas == undefined) {
            this._ps_canvas = new Canvas(this);
            this._ps_canvas.create(this._h5canvas);
        }
        return this._ps_canvas;
    }

    createContext(canvas) {
        let ctx = new Context(this);
        ctx.create(canvas);
        return ctx;
    }

    destroy() {
        this._canvas.destroy();
        this._canvas = undefined;
        this._ps_shutdown();
    }
}

/*

this._ps_matrix_create_init = _getExportWrapper(instance, 'ps_matrix_create_init');
this._ps_matrix_create = _getExportWrapper(instance, 'ps_matrix_create');
this._ps_matrix_create_copy = _getExportWrapper(instance, 'ps_matrix_create_copy');
this._ps_matrix_ref = _getExportWrapper(instance, 'ps_matrix_ref');
this._ps_matrix_unref = _getExportWrapper(instance, 'ps_matrix_unref');
this._ps_matrix_init= _getExportWrapper(instance, 'ps_matrix_init');
this._ps_matrix_translate = _getExportWrapper(instance, 'ps_matrix_translate');
this._ps_matrix_scale = _getExportWrapper(instance, 'ps_matrix_scale');
this._ps_matrix_rotate = _getExportWrapper(instance, 'ps_matrix_rotate');
this._ps_matrix_shear = _getExportWrapper(instance, 'ps_matrix_shear');
this._ps_matrix_invert = _getExportWrapper(instance, 'ps_matrix_invert');
this._ps_matrix_flip_x = _getExportWrapper(instance, 'ps_matrix_flip_x');
this._ps_matrix_flip_y = _getExportWrapper(instance, 'ps_matrix_flip_y');
this._ps_matrix_reset = _getExportWrapper(instance, 'ps_matrix_reset');
this._ps_matrix_multiply= _getExportWrapper(instance, 'ps_matrix_multiply');
this._ps_matrix_transform_point = _getExportWrapper(instance, 'ps_matrix_transform_point');
this._ps_matrix_is_equal = _getExportWrapper(instance, 'ps_matrix_is_equal');
this._ps_matrix_is_identity = _getExportWrapper(instance, 'ps_matrix_is_identity');
this._ps_matrix_get_determinant = _getExportWrapper(instance, 'ps_matrix_get_determinant');
this._ps_matrix_set_translate_factor = _getExportWrapper(instance, 'ps_matrix_set_translate_factor');
this._ps_matrix_get_translate_factor = _getExportWrapper(instance, 'ps_matrix_get_translate_factor');
this._ps_matrix_set_scale_factor = _getExportWrapper(instance, 'ps_matrix_set_scale_factor');
this._ps_matrix_get_scale_factor = _getExportWrapper(instance, 'ps_matrix_get_scale_factor');
this._ps_matrix_set_shear_factor = _getExportWrapper(instance, 'ps_matrix_set_shear_factor');
this._ps_matrix_get_shear_factor = _getExportWrapper(instance, 'ps_matrix_get_shear_factor');
this._ps_matrix_transform_rect = _getExportWrapper(instance, 'ps_matrix_transform_rect');
this._ps_matrix_transform_path = _getExportWrapper(instance, 'ps_matrix_transform_path');
this._ps_image_create = _getExportWrapper(instance, 'ps_image_create');
this._ps_image_create_from_data = _getExportWrapper(instance, 'ps_image_create_from_data');
this._ps_image_create_with_data = _getExportWrapper(instance, 'ps_image_create_with_data');
this._ps_image_create_compatible = _getExportWrapper(instance, 'ps_image_create_compatible');
this._ps_image_create_from_image = _getExportWrapper(instance, 'ps_image_create_from_image');
this._ps_image_ref = _getExportWrapper(instance, 'ps_image_ref');
this._ps_image_create_from_canvas = _getExportWrapper(instance, 'ps_image_create_from_canvas');
this._ps_image_unref = _getExportWrapper(instance, 'ps_image_unref');
this._ps_image_get_size= _getExportWrapper(instance, 'ps_image_get_size');
this._ps_image_get_format = _getExportWrapper(instance, 'ps_image_get_format');
this._ps_image_set_allow_transparent = _getExportWrapper(instance, 'ps_image_set_allow_transparent');
this._ps_image_set_transparent_color = _getExportWrapper(instance, 'ps_image_set_transparent_color');
this._ps_pattern_create_image= _getExportWrapper(instance, 'ps_pattern_create_image');
this._ps_pattern_ref = _getExportWrapper(instance, 'ps_pattern_ref');
this._ps_pattern_unref = _getExportWrapper(instance, 'ps_pattern_unref');
this._ps_pattern_transform = _getExportWrapper(instance, 'ps_pattern_transform');
this._ps_path_create = _getExportWrapper(instance, 'ps_path_create');
this._ps_path_create_copy = _getExportWrapper(instance, 'ps_path_create_copy');
this._ps_path_ref = _getExportWrapper(instance, 'ps_path_ref');
this._ps_path_unref = _getExportWrapper(instance, 'ps_path_unref');
this._ps_path_move_to = _getExportWrapper(instance, 'ps_path_move_to');
this._ps_path_line_to = _getExportWrapper(instance, 'ps_path_line_to');
this._ps_path_tangent_arc_to = _getExportWrapper(instance, 'ps_path_tangent_arc_to');
this._ps_path_add_arc = _getExportWrapper(instance, 'ps_path_add_arc');
this._ps_path_arc_to = _getExportWrapper(instance, 'ps_path_arc_to');
this._ps_path_bezier_to = _getExportWrapper(instance, 'ps_path_bezier_to');
this._ps_path_quad_to = _getExportWrapper(instance, 'ps_path_quad_to');
this._ps_path_sub_close = _getExportWrapper(instance, 'ps_path_sub_close');
this._ps_path_get_length = _getExportWrapper(instance, 'ps_path_get_length');
this._ps_path_get_vertex_count = _getExportWrapper(instance, 'ps_path_get_vertex_count');
this._ps_path_get_vertex = _getExportWrapper(instance, 'ps_path_get_vertex');
this._ps_path_clear = _getExportWrapper(instance, 'ps_path_clear');
this._ps_path_is_empty = _getExportWrapper(instance, 'ps_path_is_empty');
this._ps_path_bounding_rect = _getExportWrapper(instance, 'ps_path_bounding_rect');
this._ps_path_stroke_contains = _getExportWrapper(instance, 'ps_path_stroke_contains');
this._ps_path_contains = _getExportWrapper(instance, 'ps_path_contains');
this._ps_path_add_line = _getExportWrapper(instance, 'ps_path_add_line');
this._ps_path_add_rect = _getExportWrapper(instance, 'ps_path_add_rect');
this._ps_path_add_ellipse = _getExportWrapper(instance, 'ps_path_add_ellipse');
this._ps_path_add_rounded_rect = _getExportWrapper(instance, 'ps_path_add_rounded_rect');
this._ps_path_add_sub_path = _getExportWrapper(instance, 'ps_path_add_sub_path');
this._ps_path_clipping = _getExportWrapper(instance, 'ps_path_clipping');
this._ps_gradient_create_linear = _getExportWrapper(instance, 'ps_gradient_create_linear');
this._ps_gradient_create_radial = _getExportWrapper(instance, 'ps_gradient_create_radial');
this._ps_gradient_create_conic = _getExportWrapper(instance, 'ps_gradient_create_conic');
this._ps_gradient_ref = _getExportWrapper(instance, 'ps_gradient_ref');
this._ps_gradient_unref = _getExportWrapper(instance, 'ps_gradient_unref');
this._ps_gradient_clear_color_stops = _getExportWrapper(instance, 'ps_gradient_clear_color_stops');
this._ps_gradient_add_color_stop = _getExportWrapper(instance, 'ps_gradient_add_color_stop');
this._ps_gradient_transform = _getExportWrapper(instance, 'ps_gradient_transform');
this._ps_font_unref = _getExportWrapper(instance, 'ps_font_unref');
this._ps_font_create_copy = _getExportWrapper(instance, 'ps_font_create_copy');
this._ps_font_create = _getExportWrapper(instance, 'ps_font_create');
this._ps_font_ref = _getExportWrapper(instance, 'ps_font_ref');
this._ps_font_set_size = _getExportWrapper(instance, 'ps_font_set_size');
this._ps_font_set_weight = _getExportWrapper(instance, 'ps_font_set_weight');
this._ps_font_set_italic = _getExportWrapper(instance, 'ps_font_set_italic');
this._ps_font_set_hint = _getExportWrapper(instance, 'ps_font_set_hint');
this._ps_font_set_flip = _getExportWrapper(instance, 'ps_font_set_flip');
this._ps_font_set_charset = _getExportWrapper(instance, 'ps_font_set_charset');

*/
