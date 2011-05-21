/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _PICASSO_PRIVATE_H_
#define _PICASSO_PRIVATE_H_

#include <stdlib.h>

#include "aggheader.h"

#define MAX_FONT_NAME_LENGTH 128

namespace picasso {

class painter;
class gradient_wrapper;

ps_font* _default_font(void);

// pen object
enum {
    pen_style_solid  = 0,
    pen_style_dash 	 = 1,
};

struct graphic_pen {
    graphic_pen()
    	: style(pen_style_solid)
		, width(1.0)
		, miter_limit(4.0)
		, cap(butt_cap)
    	, join(miter_join)
		, inner(inner_miter)
		, color(0,0,0)
    	, dashes(0)
		, ndashes(0)
		, dstart(0)
    {
    }

    graphic_pen(const graphic_pen& o)
		: style(o.style)
		, width(o.width)
		, miter_limit(o.miter_limit)
		, cap(o.cap)
		, join(o.join)
		, inner(o.inner)
		, color(o.color)
    	, dashes(0)
		, ndashes(0)
		, dstart(0)
	{
		if (style == pen_style_dash) {
			ndashes = o.ndashes;
			dstart = o.dstart;			
			dashes = new double[ndashes];
			if (dashes) {
				for(unsigned i=0; i<ndashes; i++)
					dashes[i] = o.dashes[i]; 
			} else {
				//memory alloc error, ignore this dash
				//I think it is never happen.
				ndashes = 0;
				dstart = 0;
				style = pen_style_solid;
			}
		}

	}

    graphic_pen& operator = (const graphic_pen& o)
	{
		if (this == &o)
			return *this;

		clear_dash();

		style = o.style;
		width = o.width;
		miter_limit = o.miter_limit;
		cap = o.cap;
		join = o.join;
		inner = o.inner;
		color = o.color;

		if (style == pen_style_dash) {
			ndashes = o.ndashes;
			dstart = o.dstart;			
			dashes = new double[ndashes];
			if (dashes) {
				for(unsigned i=0; i<ndashes; i++)
					dashes[i] = o.dashes[i]; 
			} else {
				//memory alloc error, ignore this dash
				//I think it is never happen.
				ndashes = 0;
				dstart = 0;
				style = pen_style_solid;
			}
		}

		return *this;
	}

    ~graphic_pen()
    {
        if (dashes) {
            delete [] dashes;
            dashes = 0;
        }
    }

    void set_dash(double start, double* da, unsigned ndash)
    {
        style = pen_style_dash;
        dstart = start;
        ndashes = (ndash+1)&~1;  
        dashes = new double[ndashes];
		if (dashes) {
			memset(dashes, 0 , ndashes*sizeof(double));
			for(unsigned i=0; i<ndash; i++)
				dashes[i] = da[i]; 
		} else {
			//memory alloc error, ignore this dash
			//I think it is never happen.
			ndashes = 0;
			dstart = 0;
			style = pen_style_solid;
		}
    }
    void clear_dash()
    {
        style = pen_style_solid;
        dstart = 0;
        ndashes = 0;
        if (dashes) {
            delete [] dashes;
            dashes = 0;
        }
    }

    unsigned style; 
    float width;
    float miter_limit;
    line_cap_e cap;
    line_join_e join;
    inner_join_e inner;
    rgba color;  
    //dash 
    double * dashes;
    unsigned ndashes;
    double dstart;
};


// brush object
enum {
	brush_style_solid	 = 0,
	brush_style_image	 = 1,
	brush_style_pattern	 = 2,
	brush_style_gradient = 3,
	brush_style_canvas   = 4,
};

struct graphic_brush {
    graphic_brush() 
		: style(brush_style_solid)
		, data(0)
    	, rule(fill_non_zero)
		, color(0,0,0)
    {
    }

	graphic_brush(const graphic_brush& o)
		: style(o.style)
		, data(0)
		, rule(o.rule)
		, color(o.color)
	{
		if (style == brush_style_image)
			data = static_cast<void*>(ps_image_ref(static_cast<ps_image*>(o.data)));
		else if (style == brush_style_pattern)
			data = static_cast<void*>(ps_pattern_ref(static_cast<ps_pattern*>(o.data)));
		else if (style == brush_style_gradient)
			data = static_cast<void*>(ps_gradient_ref(static_cast<ps_gradient*>(o.data)));
		else if (style == brush_style_canvas)
			data = static_cast<void*>(ps_canvas_ref(static_cast<ps_canvas*>(o.data)));
	}

	graphic_brush& operator = (const graphic_brush& o)
	{
		if (this == &o)
			return *this;

		clear(); // free old data

		style = o.style;

		if (style == brush_style_image)
			data = static_cast<void*>(ps_image_ref(static_cast<ps_image*>(o.data)));
		else if (style == brush_style_pattern)
			data = static_cast<void*>(ps_pattern_ref(static_cast<ps_pattern*>(o.data)));
		else if (style == brush_style_gradient)
			data = static_cast<void*>(ps_gradient_ref(static_cast<ps_gradient*>(o.data)));
		else if (style == brush_style_canvas)
			data = static_cast<void*>(ps_canvas_ref(static_cast<ps_canvas*>(o.data)));

		rule = o.rule;
		color = o.color;

		return *this;
	}

	~graphic_brush()
	{
		if (data) {
			if (style == brush_style_image)
				ps_image_unref(static_cast<ps_image*>(data));
			else if (style == brush_style_pattern)
				ps_pattern_unref(static_cast<ps_pattern*>(data));
			else if (style == brush_style_gradient)
				ps_gradient_unref(static_cast<ps_gradient*>(data));
			else if (style == brush_style_canvas)
				ps_canvas_unref(static_cast<ps_canvas*>(data));
			//add more..
			data = 0;
		}
	}

	void clear()
	{
		if (data) {
			if (style == brush_style_image)
				ps_image_unref(static_cast<ps_image*>(data));
			else if (style == brush_style_pattern)
				ps_pattern_unref(static_cast<ps_pattern*>(data));
			else if (style == brush_style_gradient)
				ps_gradient_unref(static_cast<ps_gradient*>(data));
			else if (style == brush_style_canvas)
				ps_canvas_unref(static_cast<ps_canvas*>(data));
			//add more..
			data = 0;
		}
		style = brush_style_solid;
	}

	void set_image_brush(ps_image * p)
	{
		style = brush_style_image;
		data = ps_image_ref(p);
	}

	void set_pattern_brush(ps_pattern * p)
	{
		style = brush_style_pattern;
		data = ps_pattern_ref(p);
	}

	void set_gradient_brush(ps_gradient * p)
	{
		style = brush_style_gradient;
		data = ps_gradient_ref(p);
	}

	void set_canvas_brush(ps_canvas * p)
	{
		style = brush_style_canvas;
		data = ps_canvas_ref(p);
	}

	unsigned style;
	void* data;
    filling_rule_e rule;
    rgba color;  
};


// font engine object
struct font_engine {
     //--------------------------------------------------------------------
    typedef serialized_integer_path_adaptor<int32, 6>     path_adaptor_type;
	typedef serialized_scanlines_adaptor_aa<int8u>    gray8_adaptor_type;
	typedef serialized_scanlines_adaptor_bin          mono_adaptor_type;
	typedef scanline_storage_aa8                      scanlines_aa_type;
	typedef scanline_storage_bin                      scanlines_bin_type;

	font_engine();
	~font_engine();

    void resolution(unsigned);
    void weight(int);  
	void hinting(bool);
	void height(float); 
	void width(float); 
	void italic(bool); 
	void flip_y(bool); 
	void char_set(ps_charset);

	void set_antialias(bool);
	bool antialias(void);

	const char* font_signature();
	int change_stamp() const;
	bool prepare_glyph(unsigned);
	unsigned glyph_index() const;
	unsigned data_size() const;
	glyph_data_type data_type();
	const rect_i& bounds();
	float advance_x() const;
	float advance_y() const;

	unsigned resolution() const;
	bool hinting() const;
    float height() const;
	float width() const;
	int weight() const;
	bool italic() const;
	bool flip_y() const;
	ps_charset char_set() const;
	const char* name() const;

	float	ascent() const;
	float	descent() const;
	float leading() const;
	unsigned units_per_em() const;
	void write_glyph_to(int8u* data) const;
    bool add_kerning(unsigned, unsigned, float*, float*);

	bool create_font(const char*, glyph_rendering);
    void transform(const trans_affine&);
private:
	font_engine(const font_engine& o);
	font_engine& operator= (const font_engine& o);
#if defined(WIN32) && !ENABLE(FREE_TYPE2)
	font_engine_win32_tt_int32 *m_engine;
#else
	font_engine_freetype_int32 *m_engine;
#endif
};


// clip area object
struct clip_area {
	clip_area()
		: needclip(false)
		, cliprect(false)
		, rule(fill_non_zero)
	{
		fs_rect.x = 0;
		fs_rect.y = 0;
		fs_rect.w = 0;
		fs_rect.h = 0;
	}

	clip_area(const clip_area& o)
		: needclip(o.needclip)
		, cliprect(o.cliprect)
	{
		path = o.path;
		rule = o.rule;
		fs_rect = o.fs_rect;
	}

	clip_area& operator = (const clip_area& o)
	{
		if (this == &o)
			return *this;

		needclip = o.needclip;
		cliprect = o.cliprect;

		path = o.path;
		rule = o.rule;
		fs_rect = o.fs_rect;

		return *this;
	}

	bool is_not_same(const clip_area& o)
	{
		return (needclip != o.needclip) ||
			   (cliprect != o.cliprect) ||
			   (path != o.path) ||
			   (rule != o.rule) ||
			   (fs_rect.x != o.fs_rect.x) ||
			   (fs_rect.y != o.fs_rect.y) ||
			   (fs_rect.w != o.fs_rect.w) ||
			   (fs_rect.h != o.fs_rect.h);
	}

	bool needclip;
	bool cliprect;// for fast rect
	path_storage path;	
	filling_rule_e rule;
	ps_rect fs_rect; //only for fast clip , will be remove feature. 
};

//shadow object
struct shadow_state {
	shadow_state()
		: use_shadow(false)
		, x_offset(0.0f)
		, y_offset(0.0f)
		, blur(0.375f) /* 0 ~ 1 */
		, color(0,0,0,0.33f) 
	{
	}

	shadow_state(const shadow_state& o)
		: use_shadow(o.use_shadow)
		, x_offset(o.x_offset)
		, y_offset(o.y_offset)
		, blur(o.blur)
		, color(o.color)
	{
	}

	shadow_state& operator = (const shadow_state& o)
	{
		if (this == &o)
			return *this;

		use_shadow = o.use_shadow;
		x_offset = o.x_offset;
		y_offset = o.y_offset;
		blur = o.blur;
		color = o.color;
		return *this;
	}

	bool use_shadow;
	float x_offset;
	float y_offset;
	float blur;
	rgba color;
};

// context state object
struct context_state {
	context_state()
		: next(0)
		, filter(FILTER_BILINEAR)
		, font(ps_font_ref(_default_font()))
		, antialias(true)
		, gamma(1.0)
		, alpha(1.0)
		, blur(0.0)
		, font_scolor(0,0,0)
		, font_fcolor(0,0,0)
		, composite(comp_op_src_over)
	{
	}

	~context_state()
	{
		ps_font_unref(font);
	}

	context_state(const context_state& o)
		: next(0)
		, filter(o.filter)
		, font(ps_font_ref(o.font))
		, antialias(o.antialias)
		, gamma(o.gamma)
		, alpha(o.alpha)
		, blur(o.blur)
		, font_scolor(o.font_scolor)
		, font_fcolor(o.font_fcolor)
		, composite(o.composite)
		, world_matrix(o.world_matrix)
		, pen(o.pen)
		, brush(o.brush)
		, clip(o.clip)
		, shadow(o.shadow)
	{
	}

	context_state& operator = (const context_state& o)
	{
		if (this == &o)
			return *this;

		next = 0;
		filter = o.filter;
		ps_font_unref(font); //free old font
		font = ps_font_ref(o.font);//reference new one
		antialias = o.antialias;
		gamma = o.gamma;
		alpha = o.alpha;
		blur = o.blur;
		font_scolor = o.font_scolor;
		font_fcolor = o.font_fcolor;
		composite = o.composite;
		world_matrix = o.world_matrix;
		pen = o.pen;
		brush = o.brush;
		clip = o.clip;
		shadow = o.shadow;

		return *this;
	}

	struct context_state * next;

    ps_filter filter;
	ps_font * font;
	bool antialias;
    float gamma;
    float alpha;
	float blur;
	rgba font_scolor;
	rgba font_fcolor;
	comp_op_e composite;
    trans_affine world_matrix;
	graphic_pen pen;
    graphic_brush brush;
    clip_area clip;
	shadow_state shadow;
};

}

#ifdef __cplusplus
extern "C" {
#endif


struct _ps_context {
    int refcount;
    ps_canvas* canvas;
	picasso::context_state* state;
	ps_bool font_antialias;
	picasso::font_engine* fonts;
    //agg object
	font_cache_manager<picasso::font_engine>* cache;
    path_storage path;
    rasterizer_scanline_aa<> raster;
    trans_affine text_matrix;
};

enum {
	buffer_alloc_none	   = 0,
	buffer_alloc_surface   = 1,
	buffer_alloc_malloc    = 2,
	buffer_alloc_image	   = 3,
	buffer_alloc_canvas	   = 4,
};

struct _ps_canvas {
    int refcount;
    ps_color_format fmt;
    picasso::painter *p;
	unsigned flage;
	void 	 *host;
	ps_mask* mask;
    //agg object
    rendering_buffer buffer;
};

struct _ps_image {
    int refcount;
    ps_color_format fmt;
	unsigned flage;
	void 	 *host;
	bool     transparent;
	bool     colorkey;
    //agg object
	rgba8	key;
    rendering_buffer buffer;
};

struct _ps_matrix {
    int refcount;
    //agg object
    trans_affine matrix;
};

struct _ps_path {
    int refcount;
    //agg object
    path_storage path;
};

struct _ps_pattern {
    int refcount;
    //agg object
    trans_affine matrix;
	ps_wrap_type xtype;
	ps_wrap_type ytype;
    ps_image *img;
};

struct _ps_gradient {
    int refcount;
	picasso::gradient_wrapper* wrapper;
    //agg object
	float start;
	float length;
    trans_affine matrix;
	bool build;
	gradient_lut<color_interpolator<rgba8> > colors;
};

struct _ps_mask {
    int refcount;
	bool color_mask;
    //agg object
	pod_bvector<rgba8> colors; 
    rendering_buffer 	mask_buffer;
};

struct _ps_font {
    int refcount;
	char name[MAX_FONT_NAME_LENGTH];
	ps_charset charset;
	ps_font_type type;
	unsigned int resolution;
	float height;
	float width;
	int weight;
	bool italic;
	bool hint;
	bool flip_y;
	bool kerning;
};

// global error code
extern ps_status global_status;
#ifdef __cplusplus
}
#endif

namespace picasso {
//gradient abstract
class gradient_wrapper 
{
public:
	gradient_wrapper() {}
	virtual ~gradient_wrapper() {}
	virtual void init(float r, float x, float y) = 0;
	virtual int calculate (int x, int y, int) const = 0; 
};

//painter abstract
class painter {
public:
    painter() {}
    virtual ~painter() {}

	virtual void attach(rendering_buffer&) = 0;
	//graphic object renderer
    virtual void render_stroke(ps_context*, path_storage&) = 0;
    virtual void render_fill(ps_context*, path_storage&) = 0;
	virtual void render_shadow(ps_context*, path_storage&, bool, bool) = 0;
	virtual void render_clip(ps_context*, bool) = 0;
	virtual void render_clear(ps_context*) = 0;
	virtual void render_blur(ps_context*) = 0;
	virtual void render_gamma(ps_context*) = 0;

	virtual void render_mask(ps_canvas*, bool) = 0;
	virtual void render_copy(ps_canvas*, rect_i*, ps_canvas*, int, int)=0;
	//agg object renderer
	virtual void render_font_cache(ps_context*, font_cache_manager<font_engine>&, glyph_data_type) = 0;
	virtual void render_font_raster(ps_context*) = 0;
};

template<typename Pixfmt>
class pattern_wrapper
{
public:
	typedef Pixfmt   pixfmt_type;
	typedef typename pixfmt_type::color_type color_type;
	typedef typename pixfmt_type::order_type order_type;
	typedef typename pixfmt_type::value_type value_type;
	typedef typename pixfmt_type::pixel_type pixel_type;
	enum pix_width_e { pix_width = pixfmt_type::pix_width };

	pattern_wrapper() {}
	virtual ~pattern_wrapper() {}
    virtual const int8u* span(int x, int y, unsigned) = 0;
    virtual const int8u* next_x() = 0;
    virtual const int8u* next_y() = 0;
};

//inner functions
#if ENABLE(FREE_TYPE2)
bool _load_fonts(void);

void _free_fonts(void);

char * _font_by_name(const char* face);
#endif

void _clip_path(ps_context* ctx, const path_storage& p, filling_rule_e r);

bool _is_closed_path(const path_storage & path);

int _byte_pre_color(ps_color_format fmt);

void _matrix_transform_rect(const trans_affine& matrix, ps_rect* rect);

ps_rect _path_bounding_rect(const path_storage& path);

void _path_operation(gpc_op_e op, const path_storage& a, const path_storage& b, path_storage& r);

}
#endif/*_PICASSO_PRIVATE_H_*/
