/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2011 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GRAPHIC_BASE_H_
#define _GRAPHIC_BASE_H_

#include "common.h"

namespace picasso {

// path command
typedef enum {
	path_cmd_stop     = 0,        //path_cmd_stop    
	path_cmd_move_to  = 1,        //path_cmd_move_to 
	path_cmd_line_to  = 2,        //path_cmd_line_to 
	path_cmd_curve3   = 3,        //path_cmd_curve3  
	path_cmd_curve4   = 4,        //path_cmd_curve4  
	path_cmd_curveN   = 5,        //path_cmd_curveN
	path_cmd_catrom   = 6,        //path_cmd_catrom
	path_cmd_ubspline = 7,        //path_cmd_ubspline
	path_cmd_end_poly = 0x0F,     //path_cmd_end_poly
	path_cmd_mask     = 0x0F      //path_cmd_mask    
} path_cmd;

typedef enum {
	path_flags_none  = 0,         //path_flags_none 
	path_flags_ccw   = 0x10,      //path_flags_ccw  
	path_flags_cw    = 0x20,      //path_flags_cw   
	path_flags_close = 0x40,      //path_flags_close
	path_flags_mask  = 0xF0       //path_flags_mask 
} path_flags;


inline bool is_vertex(unsigned int c)
{
	return c >= path_cmd_move_to && c < path_cmd_end_poly;
}

inline bool is_drawing(unsigned int c)
{
	return c >= path_cmd_line_to && c < path_cmd_end_poly;
}

inline bool is_stop(unsigned int c)
{ 
	return c == path_cmd_stop;
}

inline bool is_move_to(unsigned int c)
{
	return c == path_cmd_move_to;
}

inline bool is_line_to(unsigned int c)
{
	return c == path_cmd_line_to;
}

inline bool is_curve(unsigned int c)
{
	return c == path_cmd_curve3 || c == path_cmd_curve4;
}

inline bool is_curve3(unsigned int c)
{
	return c == path_cmd_curve3;
}

inline bool is_curve4(unsigned int c)
{
	return c == path_cmd_curve4;
}

inline bool is_end_poly(unsigned int c)
{
	return (c & path_cmd_mask) == path_cmd_end_poly;
}

inline bool is_close(unsigned int c)
{
	return (c & ~(path_flags_cw | path_flags_ccw)) ==
		(path_cmd_end_poly | path_flags_close); 
}

inline bool is_next_poly(unsigned int c)
{
	return is_stop(c) || is_move_to(c) || is_end_poly(c);
}

inline bool is_cw(unsigned int c)
{
	return (c & path_flags_cw) != 0;
}

inline bool is_ccw(unsigned int c)
{
	return (c & path_flags_ccw) != 0;
}

inline bool is_oriented(unsigned int c)
{
	return (c & (path_flags_cw | path_flags_ccw)) != 0; 
}

inline bool is_closed(unsigned int c)
{
	return (c & path_flags_close) != 0; 
}

inline unsigned int get_close_flag(unsigned int c)
{
	return c & path_flags_close; 
}

inline unsigned int clear_orientation(unsigned int c)
{
	return c & ~(path_flags_cw | path_flags_ccw);
}

inline unsigned int get_orientation(unsigned int c)
{
	return c & (path_flags_cw | path_flags_ccw);
}

inline unsigned int set_orientation(unsigned int c, unsigned int o)
{
	return clear_orientation(c) | o;
}

// Coinciding points maximal distance (Epsilon)
const scalar vertex_dist_epsilon = DBL_TO_SCALAR(1e-14);

// math function
inline scalar calc_distance(scalar x1, scalar y1, scalar x2, scalar y2)
{
	scalar dx = x2-x1;
	scalar dy = y2-y1;
	return Sqrt(dx * dx + dy * dy);
}

inline scalar calc_sq_distance(scalar x1, scalar y1, scalar x2, scalar y2)
{
	scalar dx = x2-x1;
	scalar dy = y2-y1;
	return dx * dx + dy * dy;
}

// value calc
inline int iround(scalar v)
{
	return _iround(v);
}

inline unsigned uround(scalar v)
{
	return _uround(v);
}

// line style define
typedef enum {
	butt_cap,
	square_cap,
	round_cap,
} line_cap;

typedef enum {
	miter_join         = 0,
	miter_join_revert  = 1,
	round_join         = 2,
	bevel_join         = 3,
	miter_join_round   = 4,
} line_join;

typedef enum {
	inner_bevel,
	inner_miter,
	inner_jag,
	inner_round
} inner_join;

// fill rule
typedef enum {
	fill_non_zero,
	fill_even_odd
} filling_rule;

// raster method
typedef enum {
	raster_stroke = 1,
	raster_fill = 2,	
} raster_method;

// composite operators
typedef enum {
	comp_op_clear,         //comp_op_clear
	comp_op_src,           //comp_op_src
	comp_op_src_over,      //comp_op_src_over
	comp_op_src_in,        //comp_op_src_in
	comp_op_src_out,       //comp_op_src_out
	comp_op_src_atop,      //comp_op_src_atop
	comp_op_dst,           //comp_op_dst
	comp_op_dst_over,      //comp_op_dst_over
	comp_op_dst_in,        //comp_op_dst_in
	comp_op_dst_out,       //comp_op_dst_out
	comp_op_dst_atop,      //comp_op_dst_atop
	comp_op_xor,           //comp_op_xor
	comp_op_darken,        //comp_op_darken
	comp_op_lighten,       //comp_op_lighten
	comp_op_overlay,       //comp_op_overlay
	comp_op_screen,        //comp_op_screen
	comp_op_multiply,      //comp_op_multiply
	comp_op_plus,          //comp_op_plus
	comp_op_minus,         //comp_op_minus
	comp_op_exclusion,     //comp_op_exclusion
	comp_op_difference,    //comp_op_difference
	comp_op_soft_light,    //comp_op_soft_light
	comp_op_hard_light,    //comp_op_hard_light
	comp_op_color_burn,    //comp_op_color_burn
	comp_op_color_dodge,   //comp_op_color_dodge
	comp_op_contrast,      //comp_op_contrast
	comp_op_invert,        //comp_op_invert
	comp_op_invert_rgb,    //comp_op_invert_rgb
	end_of_comp_op,
} comp_op;

// rectangle
template<typename T> struct rect_base
{
	typedef rect_base<T> self_type;
	T x1; // left
	T y1; // top
	T x2; // right
	T y2; // bottom

	rect_base() 
		: x1(0), y1(0), x2(0), y2(0)
	{
	}

	rect_base(T _x1, T _y1, T _x2, T _y2)
		: x1(_x1), y1(_y1), x2(_x2), y2(_y2) 
	{
	} 

	bool clip(const self_type& r)
	{
		if(x2 > r.x2) x2 = r.x2;
		if(y2 > r.y2) y2 = r.y2;
		if(x1 < r.x1) x1 = r.x1;
		if(y1 < r.y1) y1 = r.y1;
		return x1 <= x2 && y1 <= y2;
	}

    T x() { return x1; }
    T y() { return y1; }
    T width() { return (x2-x1); }
    T height() { return (y2-y1); }

};

typedef rect_base<int> 	  rect; //integer
typedef rect_base<scalar> rect_s; //scalar

// color cover
typedef unsigned char cover_type; 
typedef enum {
    cover_shift = 8,                 //cover_shift
    cover_size  = 1 << cover_shift,  //cover_size 
    cover_mask  = cover_size - 1,    //cover_mask 
    cover_none  = 0,                 //cover_none 
    cover_full  = cover_mask         //cover_full 
}cover_scale;

// glyph
typedef enum {
    glyph_type_invalid = 0,
    glyph_type_mono    = 1,
    glyph_type_gray    = 2,
    glyph_type_outline = 3,    
} glyph_type;

typedef struct {
    unsigned int  code;
    unsigned int  index;
    glyph_type    type;
    byte*         data;
    unsigned int  data_size;
    rect          bounds;
    scalar        height;
    scalar        advance_x;
    scalar        advance_y;
} glyph;

}
#endif/*_GRAPHIC_BASE_H_*/

