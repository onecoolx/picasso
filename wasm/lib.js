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
        buffer[0] = (color.r <= 1.0 && color.r >= 0.0) ? color.r : color.r / 255.0;
        buffer[1] = (color.g <= 1.0 && color.g >= 0.0) ? color.g : color.g / 255.0;
        buffer[2] = (color.b <= 1.0 && color.b >= 0.0) ? color.b : color.b / 255.0;
        buffer[3] = (color.a <= 1.0 && color.a >= 0.0) ? color.a : color.a / 255.0;
    } else {
        throw TypeError("Color value is not a number, string or object!");
    }
    return cv;
}

function _destoryBuffer(vm, b) {
    vm._free(b);
}

class Path2D {
    constructor(ps) {
        this._ps = ps;
        let instance = ps._instance;
        this._ps_path_create = _getExportWrapper(instance, 'ps_path_create'); // USE
        this._ps_path_create_copy = _getExportWrapper(instance, 'ps_path_create_copy'); // DEL
        this._ps_path_ref = _getExportWrapper(instance, 'ps_path_ref'); // DEL
        this._ps_path_unref = _getExportWrapper(instance, 'ps_path_unref'); // USE
        this._ps_path_move_to = _getExportWrapper(instance, 'ps_path_move_to'); // USE
        this._ps_path_line_to = _getExportWrapper(instance, 'ps_path_line_to'); // USE
        this._ps_path_tangent_arc_to = _getExportWrapper(instance, 'ps_path_tangent_arc_to'); // USE
        this._ps_path_arc_to = _getExportWrapper(instance, 'ps_path_arc_to'); // USE
        this._ps_path_bezier_to = _getExportWrapper(instance, 'ps_path_bezier_to'); // USE
        this._ps_path_quad_to = _getExportWrapper(instance, 'ps_path_quad_to'); // USE
        this._ps_path_sub_close = _getExportWrapper(instance, 'ps_path_sub_close'); // USE
        this._ps_path_get_length = _getExportWrapper(instance, 'ps_path_get_length'); // USE
        this._ps_path_get_vertex_count = _getExportWrapper(instance, 'ps_path_get_vertex_count'); // DEL
        this._ps_path_get_vertex = _getExportWrapper(instance, 'ps_path_get_vertex'); // DEL
        this._ps_path_clear = _getExportWrapper(instance, 'ps_path_clear'); // USE
        this._ps_path_is_empty = _getExportWrapper(instance, 'ps_path_is_empty'); // USE
        this._ps_path_bounding_rect = _getExportWrapper(instance, 'ps_path_bounding_rect'); // USE
        this._ps_path_stroke_contains = _getExportWrapper(instance, 'ps_path_stroke_contains'); // USE
        this._ps_path_contains = _getExportWrapper(instance, 'ps_path_contains'); // USE
        this._ps_path_add_arc = _getExportWrapper(instance, 'ps_path_add_arc'); // USE
        this._ps_path_add_line = _getExportWrapper(instance, 'ps_path_add_line'); // USE
        this._ps_path_add_rect = _getExportWrapper(instance, 'ps_path_add_rect'); // USE
        this._ps_path_add_ellipse = _getExportWrapper(instance, 'ps_path_add_ellipse'); // USE
        this._ps_path_add_rounded_rect = _getExportWrapper(instance, 'ps_path_add_rounded_rect'); // USE
        this._ps_path_add_sub_path = _getExportWrapper(instance, 'ps_path_add_sub_path'); // USE
        this._ps_path_clipping = _getExportWrapper(instance, 'ps_path_clipping'); // USE
        this._data = this._ps_path_create();
    }

    moveTo(pt/*x*/, y) {
        let cv = _createPointBuffer(this._ps, pt, y);
        this._ps_path_move_to(this._data, cv);
        _destoryBuffer(this._ps, cv);
    }

    lineTo(pt/*x*/, y) {
        let cv = _createPointBuffer(this._ps, pt, y);
        this._ps_path_line_to(this._data, cv);
        _destoryBuffer(this._ps, cv);
    }

    quadTo(cp, ep) {
        let cv1 = _createPointBuffer(this._ps, cp);
        let cv2 = _createPointBuffer(this._ps, ep);
        this._ps_path_quad_to(this._data, cv1, cv2);
        _destoryBuffer(this._ps, cv2);
        _destoryBuffer(this._ps, cv1);
    }

    bezierTo(cp1, cp2, ep) {
        let cv1 = _createPointBuffer(this._ps, cp1);
        let cv2 = _createPointBuffer(this._ps, cp2);
        let cv3 = _createPointBuffer(this._ps, ep);
        this._ps_path_bezier_to(this._data, cv1, cv2, cv3);
        _destoryBuffer(this._ps, cv3);
        _destoryBuffer(this._ps, cv2);
        _destoryBuffer(this._ps, cv1);
    }

    tangentArcTo(r, tp, ep) {
        if (typeof r !== "number") {
            throw TypeError("Parameters must be radius:<number>.");
        }
        let cv1 = _createPointBuffer(this._ps, tp);
        let cv2 = _createPointBuffer(this._ps, ep);
        this._ps_path_tangent_arc_to(this._data, r, cv1, cv2);
        _destoryBuffer(this._ps, cv2);
        _destoryBuffer(this._ps, cv1);
    }

    arcTo(rx, ry, a, l, c, ep) {
        if (typeof rx !== "number" || typeof ry !== "number" || typeof a !== "number"
            || typeof l !== "boolean" || typeof c !== "boolean") {
            throw TypeError("Parameters must be rx:<number>, ry:<number>, angle:<number>, large_arc:<boolean>, clockwise:<boolean>, ep:<object>");
        }
        let cv = _createPointBuffer(this._ps, ep);
        this._ps_path_arc_to(this._data, rx, ry, a, (l ? 1 : 0), (c ? 1 : 0), cv);
        _destoryBuffer(this._ps, cv);
    }

    subClose() {
        this._ps_path_sub_close(this._data);
    }

    getLength() {
        return this._ps_path_get_length(this._data);
    }

    boundRect() {
        let cv = _createDataBuffer(this._ps, 16);
        let buffer = this._ps._HEAPF32.subarray((cv>>2), (cv>>2) + 4);
        let r = this._ps_path_bounding_rect(this._data, cv);
        let rect = {x: 0, y: 0, w: 0, h: 0};
        if (r) {
            rect.x = buffer[0];
            rect.y = buffer[1];
            rect.w = buffer[2];
            rect.h = buffer[3];
        }
        _destoryBuffer(this._ps, cv);
        return rect;
    }

    clear() {
        this._ps_path_clear(this._data);
    }

    isEmpty() {
        return this._ps_path_is_empty(this._data) ? true : false;
    }

    strokeContains(p, w) {
        if (typeof w !== "number") {
            throw TypeError("Parameters must be width:<number>.");
        }
        let cv = _createPointBuffer(this._ps, p);
        let r = this._ps_path_stroke_contains(this._data, cv, w);
        _destoryBuffer(this._ps, cv);
        return r ? true : false;
    }

    contains(p, rl) {
        if (typeof rl !== "string") {
            throw TypeError("Parameters must be rule:<string>.");
        }
        let w = 0;
        switch (rl.toLowerCase()) {
            case "evenodd":
                w = 1;
                break;
            case "nonzero":
                w = 0;
                break;
        }
        let cv = _createPointBuffer(this._ps, p);
        let r = this._ps_path_contains(this._data, cv, w);
        _destoryBuffer(this._ps, cv);
        return r ? true : false;
    }

    addArc(cp, r, sa, ea, c) {
        if (typeof r !== "number" || typeof sa !== "number" || typeof ea !== "number" || typeof c !== "boolean") {
            throw TypeError("Parameters must be radius:<number>, sangle:<number>, eangle:<number>, clockwise:<boolean>");
        }
        let cv = _createPointBuffer(this._ps, cp);
        this._ps_path_add_arc(this._data, cv, r, sa, ea, (c ? 1 : 0));
        _destoryBuffer(this._ps, cv);
    }

    addLine(sp, ep) {
        let cv1 = _createPointBuffer(this._ps, sp);
        let cv2 = _createPointBuffer(this._ps, ep);
        this._ps_path_add_line(this._data, cv1, cv2);
        _destoryBuffer(this._ps, cv2);
        _destoryBuffer(this._ps, cv1);
    }

    addRect(rc/*x*/, y, w, h) {
        let cv = _createRectBuffer(this._ps, rc, y, w, h);
        this._ps_path_add_rect(this._data, cv);
        _destoryBuffer(this._ps, cv);
    }

    addEllipse(rc/*x*/, y, w, h) {
        let cv = _createRectBuffer(this._ps, rc, y, w, h);
        this._ps_path_add_ellipse(this._data, cv);
        _destoryBuffer(this._ps, cv);
    }

    addRoundedRect(rrc) {
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

        this._ps_path_add_rounded_rect(this._data, cv, ltx, lty, rtx, rty, lbx, lby, rbx, rby);
        _destoryBuffer(this._ps, cv);
    }

    addSubPath(path) {
        if (!(path instanceof Path2D)) {
            throw TypeError("Parameters must be path:<Path2D>.");
        }
        this._ps_path_add_sub_path(this._data, path._data);
    }

    pathClip(op, p) {
        if (typeof op !== "string" || !(p instanceof Path2D)) {
            throw TypeError("Parameters must be op:<string>, path:<Path2D>.");
        }

        let o = 1; // intersect
        switch (op.toLowerCase()) {
            case "union":
                o = 0; break;
            case "intersect":
                o = 1; break;
            case "xor":
                o = 2; break;
            case "diff":
                o = 3; break;
        }
        let np = new Path2D(this._ps);
        this._ps_path_clipping(np._data, o, this._data, p._data);
        return np;
    }

    destroy() {
        if (this._data != undefined) {
            this._ps_path_unref(this._data);
            this._data = undefined;
        }
    }
}

class Matrix2D {
    constructor(ps) {
        this._ps = ps;
        let instance = ps._instance;
        this._ps_matrix_create_init = _getExportWrapper(instance, 'ps_matrix_create_init'); // DEL
        this._ps_matrix_create = _getExportWrapper(instance, 'ps_matrix_create'); // USE
        this._ps_matrix_create_copy = _getExportWrapper(instance, 'ps_matrix_create_copy'); // DEL
        this._ps_matrix_ref = _getExportWrapper(instance, 'ps_matrix_ref'); // DEL
        this._ps_matrix_unref = _getExportWrapper(instance, 'ps_matrix_unref'); // USE
        this._ps_matrix_init= _getExportWrapper(instance, 'ps_matrix_init'); // USE
        this._ps_matrix_translate = _getExportWrapper(instance, 'ps_matrix_translate'); // USE
        this._ps_matrix_scale = _getExportWrapper(instance, 'ps_matrix_scale'); // USE
        this._ps_matrix_rotate = _getExportWrapper(instance, 'ps_matrix_rotate'); // USE
        this._ps_matrix_shear = _getExportWrapper(instance, 'ps_matrix_shear'); // USE
        this._ps_matrix_invert = _getExportWrapper(instance, 'ps_matrix_invert'); // USE
        this._ps_matrix_flip_x = _getExportWrapper(instance, 'ps_matrix_flip_x'); // USE
        this._ps_matrix_flip_y = _getExportWrapper(instance, 'ps_matrix_flip_y'); //USE
        this._ps_matrix_identity = _getExportWrapper(instance, 'ps_matrix_identity'); // USE
        this._ps_matrix_multiply= _getExportWrapper(instance, 'ps_matrix_multiply'); // USE
        this._ps_matrix_is_equal = _getExportWrapper(instance, 'ps_matrix_is_equal'); // USE
        this._ps_matrix_is_identity = _getExportWrapper(instance, 'ps_matrix_is_identity'); // USE
        this._ps_matrix_get_determinant = _getExportWrapper(instance, 'ps_matrix_get_determinant'); // USE
        this._ps_matrix_set_translate_factor = _getExportWrapper(instance, 'ps_matrix_set_translate_factor'); // DEL
        this._ps_matrix_get_translate_factor = _getExportWrapper(instance, 'ps_matrix_get_translate_factor'); // DEL
        this._ps_matrix_set_scale_factor = _getExportWrapper(instance, 'ps_matrix_set_scale_factor'); // DEL
        this._ps_matrix_get_scale_factor = _getExportWrapper(instance, 'ps_matrix_get_scale_factor'); // DEL
        this._ps_matrix_set_shear_factor = _getExportWrapper(instance, 'ps_matrix_set_shear_factor'); // DEL
        this._ps_matrix_get_shear_factor = _getExportWrapper(instance, 'ps_matrix_get_shear_factor'); // DEL
        this._ps_matrix_transform_point = _getExportWrapper(instance, 'ps_matrix_transform_point'); // USE
        this._ps_matrix_transform_rect = _getExportWrapper(instance, 'ps_matrix_transform_rect'); // USE
        this._ps_matrix_transform_path = _getExportWrapper(instance, 'ps_matrix_transform_path'); // DEL
        this._data = this._ps_matrix_create();
    }

    init(sx, shy, shx, sy, tx, ty) {
        if (typeof sx !== "number" || typeof shy !== "number" || typeof shx !== "number" ||
            typeof sy !== "number" || typeof tx !== "number" || typeof ty !== "number") {
            throw TypeError("Parameters must be sx:<number>, shy:<number>, shx:<number>, sy:<number>, tx:<number>, ty:<number>");
        }
        this._ps_matrix_init(this._data, sx, shy, shx, sy, tx, ty);
    }

    translate(tx, ty) {
        if (typeof tx !== "number" || typeof ty !== "number") {
            throw TypeError("Parameters must be tx:<number>, ty:<number>");
        }
        this._ps_matrix_translate(this._data, tx, ty);
    }

    scale(sx, sy) {
        if (typeof sx !== "number" || typeof sy !== "number") {
            throw TypeError("Parameters must be sx:<number>, sy:<number>");
        }
        this._ps_matrix_scale(this._data, sx, sy);
    }

    rotate(r) {
        if (typeof r !== "number") {
            throw TypeError("Parameters must be radian:<number>");
        }
        this._ps_matrix_rotate(this._data, r);
    }

    shear(shx, shy) {
        if (typeof shy !== "number" || typeof shx !== "number") {
            throw TypeError("Parameters must be shx:<number>, shy:<number>");
        }
        this._ps_matrix_shear(this._data, shx, shy);
    }

    isEqual(m) {
        if (!(m instanceof Matrix2D)) {
            throw TypeError("Parameters must be matrix:<Matrix2D>");
        }
        return this._ps_matrix_is_equal(this._data, m._data) ? true : false;
    }

    isIdentity() {
        return this._ps_matrix_is_identity(this._data) ? true : false;
    }

    det() {
        return this._ps_matrix_get_determinant(this._data);
    }

    invert() {
        this._ps_matrix_invert(this._data);
    }

    flipX() {
        this._ps_matrix_flip_x(this._data);
    }

    flipY() {
        this._ps_matrix_flip_y(this._data);
    }

    identity() {
        this._ps_matrix_identity(this._data);
    }

    multiply(m) {
        if (!(m instanceof Matrix2D)) {
            throw TypeError("Parameters must be matrix:<Matrix2D>");
        }
        let r = new Matrix2D(this._ps);
        this._ps_matrix_multiply(r._data, this._data, m._data);
        return r;
    }

    transformPoint(pt/*x*/, y) {
        let cv = _createPointBuffer(this._ps, pt, y);
        this._ps_matrix_transform_point(this._data, cv);
        let buffer = this._ps._HEAPF32.subarray((cv>>2), (cv>>2) + 2);
        let ret = {x: buffer[0], y: buffer[1]};
        _destoryBuffer(this._ps, cv);
        return ret;
    }

    transformRect(rc/*x*/, y, w, h) {
        let cv = _createRectBuffer(this._ps, rc, y, w, h);
        this._ps_matrix_transform_rect(this._data, cv);
        let buffer = this._ps._HEAPF32.subarray((cv>>2), (cv>>2) + 4);
        let ret = {x: buffer[0], y: buffer[1], w: buffer[2], h: buffer[3]};
        _destoryBuffer(this._ps, cv);
        return ret;
    }

    destroy() {
        if (this._data != undefined) {
            this._ps_matrix_unref(this._data);
            this._data = undefined;
        }
    }
}

class ImageTexture {
    _createImg(img) {
        let tcanvas = new OffscreenCanvas(img.width, img.height);
        let tcontext = tcanvas.getContext('2d');
        tcontext.drawImage(img, 0, 0);
        let pixels = tcontext.getImageData(0, 0, img.width, img.height).data;
        let bytes = pixels.length;
        this._imgData = _createDataBuffer(this._ps, bytes);
        let buffer = this._ps._HEAPU8.subarray(this._imgData, this._imgData + bytes);
        let imgBuffer = new Uint8ClampedArray(buffer.buffer, buffer.byteOffset, buffer.length);
        imgBuffer.set(pixels);
        this._data = this._ps_image_create_with_data(this._imgData, 0, img.width, img.height, img.width * 4);
        this._width = img.width;
        this._height = img.height;
    }

    constructor(ps, h5Image) {
        this._ps = ps;
        let instance = ps._instance;
        this._ps_image_create = _getExportWrapper(instance, 'ps_image_create'); // DEL
        this._ps_image_create_from_data = _getExportWrapper(instance, 'ps_image_create_from_data'); // DEL
        this._ps_image_create_with_data = _getExportWrapper(instance, 'ps_image_create_with_data'); // USE
        this._ps_image_create_compatible = _getExportWrapper(instance, 'ps_image_create_compatible'); // DEL
        this._ps_image_create_from_image = _getExportWrapper(instance, 'ps_image_create_from_image'); // DEL
        this._ps_image_ref = _getExportWrapper(instance, 'ps_image_ref'); // DEL
        this._ps_image_create_from_canvas = _getExportWrapper(instance, 'ps_image_create_from_canvas'); // DEL
        this._ps_image_unref = _getExportWrapper(instance, 'ps_image_unref'); // USE
        this._ps_image_get_size= _getExportWrapper(instance, 'ps_image_get_size'); // DEL
        this._ps_image_get_format = _getExportWrapper(instance, 'ps_image_get_format'); // DEL
        this._ps_image_set_allow_transparent = _getExportWrapper(instance, 'ps_image_set_allow_transparent'); // USE
        this._ps_image_set_transparent_color = _getExportWrapper(instance, 'ps_image_set_transparent_color'); // USE
        this._createImg(h5Image);
    }

    getSize() {
        return {w: this._width, h: this._height};
    }

    setAllowTransparent(b) {
        if (typeof b !== "boolean") {
            throw TypeError("Parameters must be allow:<boolean>");
        }
        this._ps_image_set_allow_transparent(this._data, b);
    }

    setTransparentColor(color/*r*/, g, b, a) {
        let cv = _createColorBuffer(this._ps, color, g, b, a);
        this._ps_image_set_transparent_color(this._data, cv);
        _destoryBuffer(this._ps, cv);
    }

    destroy() {
        if (this._data != undefined) {
            this._ps_image_unref(this._data);
            this._data = undefined;
            _destoryBuffer(this._ps, this._imgData);
            this._imgData = undefined;
        }
    }
}


class ImagePattern {
    _getWrap(wrap) {
        if (typeof wrap !== "string") {
            throw TypeError("Parameters must be xwrap:<string>, ywrap:<string>");
        }
        let r = 0;
        switch (wrap.toLowerCase()) {
            case "repeat":
                r = 0;
                break;
            case "reflect":
                r = 1;
                break;
        }
        return r;
    }

    constructor(ps, img, xWrap, yWrap, mtx) {
        this._ps = ps;
        let instance = ps._instance;
        this._ps_pattern_create_image= _getExportWrapper(instance, 'ps_pattern_create_image'); // USE
        this._ps_pattern_ref = _getExportWrapper(instance, 'ps_pattern_ref'); // DEL
        this._ps_pattern_unref = _getExportWrapper(instance, 'ps_pattern_unref'); // USE
        this._ps_pattern_transform = _getExportWrapper(instance, 'ps_pattern_transform'); // USE
        this._data = this._ps_pattern_create_image(img._data, this._getWrap(xWrap), this._getWrap(yWrap), mtx._data);
    }

    transform(m) {
        if (!(m instanceof Matrix2D)) {
            throw TypeError("Parameters must be matrix:<Matrix2D>");
        }
        this._ps_pattern_transform(this._data, m._data);
    }

    destroy() {
        if (this._data != undefined) {
            this._ps_pattern_unref(this._data);
            this._data = undefined;
            this._img = undefined;
        }
    }
}


class Gradient {
    constructor(ps) {
        this._ps = ps;
        let instance = ps._instance;
        this._ps_gradient_create_linear = _getExportWrapper(instance, 'ps_gradient_create_linear'); // USE
        this._ps_gradient_create_radial = _getExportWrapper(instance, 'ps_gradient_create_radial'); // USE
        this._ps_gradient_create_conic = _getExportWrapper(instance, 'ps_gradient_create_conic'); // USE
        this._ps_gradient_ref = _getExportWrapper(instance, 'ps_gradient_ref'); // DEL
        this._ps_gradient_unref = _getExportWrapper(instance, 'ps_gradient_unref'); // USE
        this._ps_gradient_clear_color_stops = _getExportWrapper(instance, 'ps_gradient_clear_color_stops'); // USE
        this._ps_gradient_add_color_stop = _getExportWrapper(instance, 'ps_gradient_add_color_stop'); // USE
        this._ps_gradient_transform = _getExportWrapper(instance, 'ps_gradient_transform'); // USE
    }

    _getSpread(s) {
        if (typeof s !== "string") {
            throw TypeError("Parameters must be spread:<string>");
        }
        let r = 0;
        switch (s.toLowerCase()) {
            case "pad":
                r = 0; break;
            case "repeat":
                r = 1; break;
            case "reflect":
                r = 2; break;
        }
        return r;
    }

    createLinear(spread, sp, ep) {
        let s = this._getSpread(spread);
        let cv1 = _createPointBuffer(this._ps, sp);
        let cv2 = _createPointBuffer(this._ps, ep);
        this._data = this._ps_gradient_create_linear(s, cv1, cv2);
        _destoryBuffer(this._ps, cv2);
        _destoryBuffer(this._ps, cv1);
    }

    createRadial(spread, sp, sr, ep, er) {
        if (typeof sr !== "number" || typeof er !== "number") {
            throw TypeError("Parameters must be sradius:<number>, eradius:<number>");
        }
        let s = this._getSpread(spread);
        let cv1 = _createPointBuffer(this._ps, sp);
        let cv2 = _createPointBuffer(this._ps, ep);
        this._data = this._ps_gradient_create_radial(s, cv1, sr, cv2, er);
        _destoryBuffer(this._ps, cv2);
        _destoryBuffer(this._ps, cv1);
    }

    createConic(spread, cp, sa) {
        if (typeof sa !== "number") {
            throw TypeError("Parameters must be sangle:<number>");
        }
        let s = this._getSpread(spread);
        let cv = _createPointBuffer(this._ps, cp);
        this._data = this._ps_gradient_create_conic(s, cv, sa);
        _destoryBuffer(this._ps, cv);
    }

    addColorStop(offset, color/*r*/, g, b, a) {
        if (typeof offset !== "number") {
            throw TypeError("Parameters must be offset:<number>");
        }
        let cv = _createColorBuffer(this._ps, color, g, b, a);
        this._ps_gradient_add_color_stop(this._data, offset, cv);
        _destoryBuffer(this._ps, cv);
    }

    clear() {
        this._ps_gradient_clear_color_stops(this._data);
    }

    transform(m) {
        if (!(m instanceof Matrix2D)) {
            throw TypeError("Parameters must be matrix:<Matrix2D>");
        }
        this._ps_gradient_transform(this._data, m._data);
    }

    destroy() {
        if (this._data != undefined) {
            this._ps_gradient_unref(this._data);
            this._data = undefined;
        }
    }
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
        this._ps_context_create = _getExportWrapper(instance, 'ps_context_create'); // USE
        this._ps_context_ref = _getExportWrapper(instance, 'ps_context_ref'); // DEL
        this._ps_context_set_canvas = _getExportWrapper(instance, 'ps_context_set_canvas'); // DEL
        this._ps_context_get_canvas = _getExportWrapper(instance, 'ps_context_get_canvas'); // DEL
        this._ps_context_unref = _getExportWrapper(instance, 'ps_context_unref'); // USE
        this._ps_set_source_gradient = _getExportWrapper(instance, 'ps_set_source_gradient'); // USE
        this._ps_set_source_pattern = _getExportWrapper(instance, 'ps_set_source_pattern'); // USE
        this._ps_set_source_image = _getExportWrapper(instance, 'ps_set_source_image'); // USE
        this._ps_set_source_color = _getExportWrapper(instance, 'ps_set_source_color'); // USE
        this._ps_set_source_canvas = _getExportWrapper(instance, 'ps_set_source_canvas');
        this._ps_set_stroke_color = _getExportWrapper(instance, 'ps_set_stroke_color'); // USE
        this._ps_set_stroke_image = _getExportWrapper(instance, 'ps_set_stroke_image'); // USE
        this._ps_set_stroke_pattern = _getExportWrapper(instance, 'ps_set_stroke_pattern'); // USE
        this._ps_set_stroke_gradient = _getExportWrapper(instance, 'ps_set_stroke_gradient'); // USE
        this._ps_set_stroke_canvas = _getExportWrapper(instance, 'ps_set_stroke_canvas');
        this._ps_set_filter = _getExportWrapper(instance, 'ps_set_filter'); // USE
        this._ps_stroke = _getExportWrapper(instance, 'ps_stroke'); // USE
        this._ps_fill = _getExportWrapper(instance, 'ps_fill'); // USE
        this._ps_paint = _getExportWrapper(instance, 'ps_paint'); // USE
        this._ps_clear = _getExportWrapper(instance, 'ps_clear'); // USE
        this._ps_set_alpha = _getExportWrapper(instance, 'ps_set_alpha'); // USE
        this._ps_set_blur = _getExportWrapper(instance, 'ps_set_blur'); // USE
        this._ps_set_gamma = _getExportWrapper(instance, 'ps_set_gamma'); // USE
        this._ps_set_antialias = _getExportWrapper(instance, 'ps_set_antialias'); // USE
        this._ps_set_shadow = _getExportWrapper(instance, 'ps_set_shadow'); // USE
        this._ps_set_shadow_color = _getExportWrapper(instance, 'ps_set_shadow_color'); // USE
        this._ps_reset_shadow = _getExportWrapper(instance, 'ps_reset_shadow'); // USE
        this._ps_set_fill_rule = _getExportWrapper(instance, 'ps_set_fill_rule'); // USE
        this._ps_set_line_cap = _getExportWrapper(instance, 'ps_set_line_cap'); // USE
        this._ps_set_line_inner_join = _getExportWrapper(instance, 'ps_set_line_inner_join'); // USE
        this._ps_set_line_join = _getExportWrapper(instance, 'ps_set_line_join'); // USE
        this._ps_set_line_width = _getExportWrapper(instance, 'ps_set_line_width'); // USE
        this._ps_set_miter_limit = _getExportWrapper(instance, 'ps_set_miter_limit'); // USE
        this._ps_set_line_dash = _getExportWrapper(instance, 'ps_set_line_dash'); // USE
        this._ps_reset_line_dash = _getExportWrapper(instance, 'ps_reset_line_dash'); // USE
        this._ps_new_path = _getExportWrapper(instance, 'ps_new_path'); // USE
        this._ps_add_sub_path = _getExportWrapper(instance, 'ps_add_sub_path'); // USE
        this._ps_new_sub_path = _getExportWrapper(instance, 'ps_new_sub_path'); // USE
        this._ps_rectangle = _getExportWrapper(instance, 'ps_rectangle'); // USE
        this._ps_rounded_rect = _getExportWrapper(instance, 'ps_rounded_rect'); // USE
        this._ps_ellipse = _getExportWrapper(instance, 'ps_ellipse'); // USE
        this._ps_close_path = _getExportWrapper(instance, 'ps_close_path'); // USE
        this._ps_move_to = _getExportWrapper(instance, 'ps_move_to'); // USE
        this._ps_line_to = _getExportWrapper(instance, 'ps_line_to'); // USE
        this._ps_bezier_curve_to = _getExportWrapper(instance, 'ps_bezier_curve_to'); // USE
        this._ps_quad_curve_to = _getExportWrapper(instance, 'ps_quad_curve_to'); // USE
        this._ps_arc = _getExportWrapper(instance, 'ps_arc'); // USE
        this._ps_tangent_arc = _getExportWrapper(instance, 'ps_tangent_arc'); // USE
        this._ps_set_path = _getExportWrapper(instance, 'ps_set_path'); // USE
        this._ps_get_path = _getExportWrapper(instance, 'ps_get_path'); // USE
        this._ps_translate = _getExportWrapper(instance, 'ps_translate'); // USE
        this._ps_scale = _getExportWrapper(instance, 'ps_scale'); // USE
        this._ps_shear = _getExportWrapper(instance, 'ps_shear'); // USE
        this._ps_rotate = _getExportWrapper(instance, 'ps_rotate'); // USE
        this._ps_identity = _getExportWrapper(instance, 'ps_identity'); // USE
        this._ps_transform = _getExportWrapper(instance, 'ps_transform'); // USE
        this._ps_set_matrix = _getExportWrapper(instance, 'ps_set_matrix'); // USE
        this._ps_get_matrix = _getExportWrapper(instance, 'ps_get_matrix'); // DEL
        this._ps_world_to_viewport = _getExportWrapper(instance, 'ps_world_to_viewport'); // USE
        this._ps_viewport_to_world = _getExportWrapper(instance, 'ps_viewport_to_world'); // USE
        this._ps_set_composite_operator = _getExportWrapper(instance, 'ps_set_composite_operator'); // USE
        this._ps_clip = _getExportWrapper(instance, 'ps_clip'); // USE
        this._ps_clip_path = _getExportWrapper(instance, 'ps_clip_path'); // USE
        //this._ps_clip_device_rect = _getExportWrapper(instance, 'ps_clip_device_rect'); // DEL
        this._ps_clip_rect = _getExportWrapper(instance, 'ps_clip_rect'); // USE
        this._ps_clip_rects = _getExportWrapper(instance, 'ps_clip_rects'); // USE
        this._ps_reset_clip = _getExportWrapper(instance, 'ps_reset_clip'); // USE
        this._ps_save = _getExportWrapper(instance, 'ps_save'); // USE
        this._ps_restore = _getExportWrapper(instance, 'ps_restore'); // USE
    }

    create(canvas) {
        this._ctx = this._ps_context_create(canvas._canvas, 0);
    }

    createPath2D() {
        return new Path2D(this._ps);
    }

    createMatrix() {
        return new Matrix2D(this._ps);
    }

    createImage(h5Image) {
        if (!(h5Image instanceof Image)) {
            throw TypeError("Parameters must be image:<Image>");
        }
        return new ImageTexture(this._ps, h5Image);
    }

    createPattern(image, xwrap, ywrap, m) {
        if (m !== undefined && !(m instanceof Matrix2D)) {
            throw TypeError("Parameters must be matrix:<Matrix2D>");
        }

        if (image instanceof Image) {
            let img = new ImageTexture(this._ps, image);
            return new ImagePattern(this._ps, img, xwrap, ywrap, m);
        } else if (image instanceof ImageTexture) {
            return new ImagePattern(this._ps, image, xwrap, ywrap, m);
        } else {
            throw TypeError("Parameters must be image:<Image> or <ImageTexture>");
        }
    }

    createLinearGradient(s, sp, ep) {
        let g = new Gradient(this._ps);
        g.createLinear(s, sp, ep);
        return g;
    }

    createRadialGradient(s, sp, sr, ep, er) {
        let g = new Gradient(this._ps);
        g.createRadial(s, sp, sr, ep, er);
        return g;
    }

    createConicGradient(s, cp, sa) {
        let g = new Gradient(this._ps);
        g.createConic(s, cp, sa);
        return g;
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

    setCompositeOperation(cp) {
        if (typeof cp !== "string") {
            throw TypeError("Parameters must be composite:<string>");
        }

        let op = 2; // SRC_OVER
        switch (cp.toLowerCase()) {
            case "clear":
                op = 0; break;
            case "source":
                op = 1; break;
            case "source-over":
                op = 2; break;
            case "source-in":
                op = 3; break;
            case "source-out":
                op = 4; break;
            case "source-atop":
                op = 5; break;
            case "destination":
                op = 6; break;
            case "destination-over":
                op = 7; break;
            case "destination-in":
                op = 8; break;
            case "destination-out":
                op = 9; break;
            case "destination-atop":
                op = 10; break;
            case "xor":
                op = 11; break;
            case "darken":
                op = 12; break;
            case "lighten":
                op = 13; break;
            case "overlay":
                op = 14; break;
            case "screen":
                op = 15; break;
            case "multiply":
                op = 16; break;
            case "plus":
                op = 17; break;
            case "minus":
                op = 18; break;
            case "exclusion":
                op = 19; break;
            case "difference":
                op = 20; break;
            case "soft-light":
                op = 21; break;
            case "hard-light":
                op = 22; break;
            case "color-burn":
                op = 23; break;
            case "color-dodge":
                op = 24; break;
            case "contrast":
                op = 25; break;
            case "invert":
                op = 26; break;
            case "invert-blend":
                op = 27; break;
            case "hue":
                op = 28; break;
            case "saturation":
                op = 29; break;
            case "color":
                op = 30; break;
            case "luminosity":
                op = 31; break;
        }
        this._ps_set_composite_operator(this._ctx, op);
    }

    setFillRule(rule) {
        if (typeof rule !== "string") {
            throw TypeError("Parameters must be rule:<string>");
        }

        switch (rule.toLowerCase()) {
            case "evenodd":
                this._ps_set_fill_rule(this._ctx, 1);
                return;
            case "nonzero":
                this._ps_set_fill_rule(this._ctx, 0);
                return;
        }
    }

    setLineCap(cap) {
        if (typeof cap !== "string") {
            throw TypeError("Parameters must be cap:<string>");
        }

        switch (cap.toLowerCase()) {
            case "butt":
                this._ps_set_line_cap(this._ctx, 0);
                return;
            case "round":
                this._ps_set_line_cap(this._ctx, 1);
                return;
            case "square":
                this._ps_set_line_cap(this._ctx, 2);
                return;
        }
    }

    setLineJoin(join) {
        if (typeof join !== "string") {
            throw TypeError("Parameters must be join:<string>");
        }

        switch (join.toLowerCase()) {
            case "miter":
                this._ps_set_line_join(this._ctx, 0);
                return;
            case "miter-revert":
                this._ps_set_line_join(this._ctx, 1);
                return;
            case "miter-round":
                this._ps_set_line_join(this._ctx, 2);
                return;
            case "round":
                this._ps_set_line_join(this._ctx, 3);
                return;
            case "bevel":
                this._ps_set_line_join(this._ctx, 4);
                return;
        }
    }

    setLineInnerJoin(join) {
        if (typeof join !== "string") {
            throw TypeError("Parameters must be join:<string>");
        }

        switch (join.toLowerCase()) {
            case "miter":
                this._ps_set_line_inner_join(this._ctx, 0);
                return;
            case "bevel":
                this._ps_set_line_inner_join(this._ctx, 1);
                return;
            case "jag":
                this._ps_set_line_inner_join(this._ctx, 2);
                return;
            case "round":
                this._ps_set_line_inner_join(this._ctx, 3);
                return;
        }
    }

    setFilter(f) {
        if (typeof f !== "string") {
            throw TypeError("Parameters must be filter:<string>");
        }

        switch (f.toLowerCase()) {
            case "nearest":
                this._ps_set_filter(this._ctx, 0);
                return;
            case "bilinear":
                this._ps_set_filter(this._ctx, 1);
                return;
            case "gaussian":
                this._ps_set_filter(this._ctx, 2);
                return;
            case "bicubic":
                this._ps_set_filter(this._ctx, 3);
                return;
            case "quadric":
                this._ps_set_filter(this._ctx, 4);
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

    setPath(p) {
        if (!(p instanceof Path2D)) {
            throw TypeError("Parameters must be path:<Path2D>.");
        }
        this._ps_set_path(this._ctx, p._data);
    }

    getPath() {
        let np = new Path2D(this._ps);
        this._ps_get_path(this._ctx, np._data);
        return np;
    }

    clipRects(rcs) {
        if (!(rcs instanceof Array)) {
            throw TypeError("Parameters must be rects:<array>.");
        }
        let n = rcs.length;
        let cv = _createDataBuffer(this._ps, n * 16);
        let buffer = this._ps._HEAPF32.subarray((cv>>2), (cv>>2) + (n * 4));
        for (let i = 0; i < n; i++) {
            if (typeof rcs[i] === "object") {
                if (typeof rcs[i].x !== "number" ||
                        typeof rcs[i].y !== "number" ||
                        typeof rcs[i].w !== "number" ||
                        typeof rcs[i].h !== "number") {
                    throw TypeError("Rect value object must be {x:<number>, y:<number>, w:<number>, h:<number>}");
                }
                buffer[i * 4] = rcs[i].x;
                buffer[i * 4 + 1] = rcs[i].y;
                buffer[i * 4 + 2] = rcs[i].w;
                buffer[i * 4 + 3] = rcs[i].h;
            } else {
                throw TypeError("Rect values is not object!");
            }
        }
        this._ps_clip_rects(this._ctx, cv, n);
        _destoryBuffer(this._ps, cv);
    }

    clipPath(p, r) {
        if (!(p instanceof Path2D) || typeof r !== "string") {
            throw TypeError("Parameters must be path:<Path2D>, rule:<string>.");
        }
        let rule = 0;
        switch (r.toLowerCase()) {
            case "evenodd":
                rule = 1;
                break;
            case "nonzero":
                rule = 0;
                break;
        }
        this._ps_clip_path(this._ctx, p._data, rule);
    }

    clipRect(rc/* x */, y, w, h) {
        let cv = _createRectBuffer(this._ps, rc, y, w, h);
        this._ps_clip_rect(this._ctx, cv);
        _destoryBuffer(this._ps, cv);
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

    addSubPath(p) {
        if (!(p instanceof Path2D)) {
            throw TypeError("Parameters must be path:<Path2D>.");
        }
        this._ps_add_sub_path(this._ctx, p._data);
    }

    setSourceGradient(g) {
        if (!(g instanceof Gradient)) {
            throw TypeError("Parameters must be gradient:<Gradient>.");
        }
        this._ps_set_source_gradient(this._ctx, g._data);
    }

    setSourcePattern(p) {
        if (!(p instanceof ImagePattern)) {
            throw TypeError("Parameters must be image:<ImagePattern>.");
        }
        this._ps_set_source_pattern(this._ctx, p._data);
    }

    setSourceImage(img) {
        if (!(img instanceof ImageTexture)) {
            throw TypeError("Parameters must be image:<ImageTexture>.");
        }
        this._ps_set_source_image(this._ctx, img._data);
    }

    setStrokeGradient(g) {
        if (!(g instanceof Gradient)) {
            throw TypeError("Parameters must be gradient:<Gradient>.");
        }
        this._ps_set_stroke_gradient(this._ctx, g._data);
    }

    setStrokeImage(img) {
        if (!(img instanceof ImageTexture)) {
            throw TypeError("Parameters must be image:<ImageTexture>.");
        }
        this._ps_set_stroke_image(this._ctx, img._data);
    }

    setStrokePattern(p) {
        if (!(p instanceof ImagePattern)) {
            throw TypeError("Parameters must be image:<ImagePattern>.");
        }
        this._ps_set_stroke_pattern(this._ctx, p._data);
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

    transform(m) {
        if (!(m instanceof Matrix2D)) {
            throw TypeError("Parameters must be matrix:<Matrix2D>");
        }
        this._ps_transform(this._ctx, m._data);
    }

    setMatrix(m) {
        if (!(m instanceof Matrix2D)) {
            throw TypeError("Parameters must be matrix:<Matrix2D>");
        }
        this._ps_set_matrix(this._ctx, m._data);
    }

    worldToViewport(pt/*x*/, y) {
        let cv = _createPointBuffer(this._ps, pt, y);
        this._ps_world_to_viewport(this._ctx, cv);
        let buffer = this._ps._HEAPF32.subarray((cv>>2), (cv>>2) + 2);
        let ret = {x: buffer[0], y: buffer[1]};
        _destoryBuffer(this._ps, cv);
        return ret;
    }

    viewportToWorld(pt/*x*/, y) {
        let cv = _createPointBuffer(this._ps, pt, y);
        this._ps_viewport_to_world(this._ctx, cv);
        let buffer = this._ps._HEAPF32.subarray((cv>>2), (cv>>2) + 2);
        let ret = {x: buffer[0], y: buffer[1]};
        _destoryBuffer(this._ps, cv);
        return ret;
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
        .then((binary) => WebAssembly.instantiate(binary, info))
        .then((result) => {
            this._instance = result.instance;
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

        this._ps_mask_create_with_data = _getExportWrapper(instance, 'ps_mask_create_with_data');
        this._ps_mask_ref = _getExportWrapper(instance, 'ps_mask_ref');
        this._ps_mask_unref = _getExportWrapper(instance, 'ps_mask_unref');
        this._ps_mask_add_color_filter = _getExportWrapper(instance, 'ps_mask_add_color_filter');
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
