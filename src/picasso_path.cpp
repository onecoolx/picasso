/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <new>
#include <stdlib.h>

#include "pconfig.h"
#include "picasso.h"
#include "picasso_utils.h"
#include "picasso_p.h"

using namespace picasso;

#ifdef __cplusplus
extern "C" {
#endif

ps_path* PICAPI ps_path_create(void)
{
    ps_path *p = (ps_path*)malloc(sizeof(ps_path));
    if (p) {
		p->refcount = 1;
        new ((void*)&(p->path)) path_storage;
		global_status = STATUS_SUCCEED;
        return p;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_path* PICAPI ps_path_create_copy(const ps_path* path)
{
	if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	ps_path *p = (ps_path*)malloc(sizeof(ps_path));
	if (p) {
		p->refcount = 1;
        new ((void*)&(p->path)) path_storage;
		p->path = path->path;
		global_status = STATUS_SUCCEED;
        return p;
	} else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
	}
}

ps_path* PICAPI ps_path_ref(ps_path* path)
{
	if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

	path->refcount++;
	global_status = STATUS_SUCCEED;
	return path;
}

void PICAPI ps_path_unref(ps_path* path)
{
    if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	path->refcount--;
	if (path->refcount <= 0) {
		(&path->path)->path_storage::~path_storage();
		free(path);
	}
	global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_move_to(ps_path* path, const ps_point* p)
{
    if (!path || !p) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    path->path.move_to((float)p->x, (float)p->y);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_line_to(ps_path* path, const ps_point* p)
{
    if (!path || !p) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    path->path.line_to((float)p->x, (float)p->y);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_tangent_arc_to(ps_path* path, double r, const ps_point* tp, const ps_point* ep)
{
	if (!path || !tp || !ep || r<0) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
	}

	ps_point sp;
	sp.x = path->path.last_x();
	sp.y = path->path.last_y();
    if ((tp->x == sp.x && tp->y == sp.y) || (tp->x == ep->x && tp->y == ep->y) || r == 0.f) {
        ps_path_line_to(path, tp);
    	global_status = STATUS_SUCCEED;
        return;
    }
    ps_point p1p0 = {(sp.x - tp->x), (sp.y - tp->y)};
    ps_point p1p2 = {(ep->x - tp->x), (ep->y - tp->y)};
    float p1p0_length = (float)sqrt(p1p0.x * p1p0.x + p1p0.y * p1p0.y);
    float p1p2_length = (float)sqrt(p1p2.x * p1p2.x + p1p2.y * p1p2.y);

    float cos_phi = float((p1p0.x * p1p2.x + p1p0.y * p1p2.y) / (p1p0_length * p1p2_length));
    // all points on a line logic
    if (cos_phi == -1) {
        ps_path_line_to(path, tp);
    	global_status = STATUS_SUCCEED;
        return;
    }

    if (cos_phi == 1) {
        // add infinite far away point
        unsigned int max_length = 65535;
        float factor_max = max_length / p1p0_length;
        ps_point np  = {(sp.x + factor_max * p1p0.x), (sp.y + factor_max * p1p0.y)};
        ps_path_line_to(path, &np);
    	global_status = STATUS_SUCCEED;
        return;
    }


    float tangent = (float)r / (float)tan((float)acos(cos_phi) / 2);
    float factor_p1p0 = tangent / p1p0_length;
    ps_point t_p1p0 = {(tp->x + factor_p1p0 * p1p0.x), (tp->y + factor_p1p0 * p1p0.y)};

    ps_point orth_p1p0 = {p1p0.y, -p1p0.x};
    float orth_p1p0_length = (float)sqrt(orth_p1p0.x * orth_p1p0.x + orth_p1p0.y * orth_p1p0.y);
    float factor_ra = (float)r / orth_p1p0_length;

    // angle between orth_p1p0 and p1p2 to get the right vector orthographic to p1p0
    float cos_alpha = float((orth_p1p0.x * p1p2.x + orth_p1p0.y * p1p2.y) / (orth_p1p0_length * p1p2_length));
    if (cos_alpha < 0.f)
        orth_p1p0.x = -orth_p1p0.x;
	    orth_p1p0.y = -orth_p1p0.y;

    ps_point p = {(t_p1p0.x + factor_ra * orth_p1p0.x), (t_p1p0.y + factor_ra * orth_p1p0.y)};

    // calculate angles for addArc
    orth_p1p0.x = -orth_p1p0.x;
	orth_p1p0.y = -orth_p1p0.y;
    float sa = (float)acos(orth_p1p0.x / orth_p1p0_length);
    if (orth_p1p0.y < 0.f)
        sa = 2 * PI - sa;

    // clockwise logic
    ps_bool clockwise = True;

    float factor_p1p2 = tangent / p1p2_length;
    ps_point t_p1p2 = {(tp->x + factor_p1p2 * p1p2.x), (tp->y + factor_p1p2 * p1p2.y)};
    ps_point orth_p1p2 = {(t_p1p2.x - p.x),(t_p1p2.y - p.y)};
    float orth_p1p2_length = (float)sqrt(orth_p1p2.x * orth_p1p2.x + orth_p1p2.y * orth_p1p2.y);
    float ea = (float)acos(orth_p1p2.x / orth_p1p2_length);
    if (orth_p1p2.y < 0)
        ea = 2 * PI - ea;
    if ((sa > ea) && ((sa - ea) < PI))
        clockwise = False;
    if ((sa < ea) && ((ea - sa) > PI))
        clockwise = False;

    ps_path_line_to(path, &t_p1p0);

	ps_path_add_arc(path, &p, r, sa, ea, clockwise);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_arc_to(ps_path* path, double rx, double ry, double a, ps_bool large, ps_bool cw, const ps_point* ep)
{
    if (!path || !ep || rx <= 0.0 || ry <= 0.0) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    float x1 = path->path.last_x();
    float y1 = path->path.last_y();

    bezier_arc_svg arc(float(x1), float(y1), float(rx), float(ry), float(a), 
						(large?true:false), (cw?true:false), float(ep->x), float(ep->y));
    conv_curve<bezier_arc_svg> cr(arc);
    if (_is_closed_path(path->path))
        path->path.concat_path(cr, 0);
    else
        path->path.join_path(cr, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_bezier_to(ps_path* path, const ps_point* cp1, const ps_point* cp2, const ps_point* ep)
{
    if (!path || !cp1 || !cp2 || !ep) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    curve4 c(path->path.last_x(), path->path.last_y(), 
		float(cp1->x), float(cp1->y), float(cp2->x), float(cp2->y), float(ep->x), float(ep->y));
    if (_is_closed_path(path->path))
        path->path.concat_path(c, 0);
    else
        path->path.join_path(c, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_quad_to(ps_path* path, const ps_point* cp, const ps_point* ep)
{
    if (!path || !cp || !ep) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    curve3 c(path->path.last_x(), path->path.last_y(), 
					float(cp->x), float(cp->y), float(ep->x), float(ep->y));

    if (_is_closed_path(path->path))
        path->path.concat_path(c, 0);
    else
        path->path.join_path(c, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_sub_close(ps_path* path)
{
    if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    path->path.close_polygon();
    global_status = STATUS_SUCCEED;
}

double PICAPI ps_path_get_length(const ps_path* path)
{
    if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return (0.0);
    }

    global_status = STATUS_SUCCEED;
    return path_length<path_storage>(const_cast<ps_path*>(path)->path, 0);
}

unsigned int PICAPI ps_path_get_vertex_count(const ps_path* path)
{
    if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    global_status = STATUS_SUCCEED;
	return path->path.total_vertices();
}

ps_path_cmd PICAPI ps_path_get_vertex(const ps_path* path, unsigned int index, ps_point * point)
{
    if (!path || !point || (index > path->path.total_vertices()-1)) {
		global_status = STATUS_INVALID_ARGUMENT;
        return PATH_CMD_STOP;
    }

	float x = 0;
	float y = 0;
	unsigned cmd = path->path.vertex(index, &x, &y);
	point->x = x;
	point->y = y;
    global_status = STATUS_SUCCEED;
	return (ps_path_cmd)cmd;
}

void PICAPI ps_path_clear(ps_path* path)
{
    if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    path->path.remove_all();
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_path_is_empty(const ps_path* path)
{
    if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return False;
    }

    global_status = STATUS_SUCCEED;
    return (!path->path.total_vertices()) ? True : False;
}

ps_rect PICAPI ps_path_bounding_rect(const ps_path* path)
{
    ps_rect r = {0, 0, 0, 0};
    if (!path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return r;
    }
    global_status = STATUS_SUCCEED;
    return _path_bounding_rect(path->path);
}

ps_bool PICAPI ps_path_contains(const ps_path* path, const ps_point* p, ps_fill_rule rule)
{
    if (!path || !p) {
		global_status = STATUS_INVALID_ARGUMENT;
        return False;
    }

    if (!path->path.total_vertices()) {
		global_status = STATUS_SUCCEED;
        return False;
    }

    ps_rect br = ps_path_bounding_rect(path);
    if ((p->x < br.x) || (p->y < br.y) || (p->x > (br.x+br.w)) || (p->y > (br.y+br.h))) { 
		//out of bounding rect
		global_status = STATUS_SUCCEED;
        return False;
    }

    rasterizer_scanline_aa<> rs;
    rs.add_path(const_cast<ps_path*>(path)->path);
    if (rule == FILL_RULE_EVEN_ODD)
        rs.filling_rule(fill_even_odd);
    else
        rs.filling_rule(fill_non_zero);

    global_status = STATUS_SUCCEED;
    return rs.hit_test((int)iround(float(p->x)), (int)iround(float(p->y))) ? True : False;
}

void PICAPI ps_path_add_line(ps_path* path, const ps_point* p1, const ps_point* p2)
{
    if (!path || !p1 || !p2) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    path->path.move_to(float(p1->x), float(p1->y));
    path->path.line_to(float(p2->x), float(p2->y));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_add_arc(ps_path* path, const ps_point* cp, double r, double sa, double ea, ps_bool cw)
{
    if (!path || !cp) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    if (r <= 0.0) {
    	global_status = STATUS_SUCCEED;
        return; //do nothing
    }

    arc a(float(cp->x), float(cp->y), float(r), float(r), float(sa), float(ea), (cw?true:false));

    if (_is_closed_path(path->path))
        path->path.concat_path(a, 0);
    else
        path->path.join_path(a, 0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_add_rect(ps_path* path, const ps_rect* r)
{
    if (!path || !r) {
	global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    path->path.move_to(float(r->x), float(r->y));
    path->path.hline_rel(float(r->w));
    path->path.vline_rel(float(r->h));
    path->path.hline_rel(-float(r->w));
    path->path.end_poly();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_path_add_ellipse(ps_path* path, const ps_rect* r)
{
    if (!path || !r) {
	global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    ellipse e(float(r->x+r->w/2), float(r->y+r->h/2), float(r->w/2), float(r->h/2));
    if (_is_closed_path(path->path))
        path->path.concat_path(e, 0);
    else
        path->path.join_path(e, 0);
    global_status = STATUS_SUCCEED;
}


void PICAPI ps_path_add_rounded_rect(ps_path*path, const ps_rect* r, double ltx, double lty, double rtx, double rty,
                                                                        double lbx, double lby, double rbx, double rby)
{
    if (!path || !r) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	rounded_rect rr;
	rr.rect(float(r->x), float(r->y), float(r->x+r->w), float(r->y+r->h));
	rr.radius(float(ltx), float(lty), float(rtx), float(rty), 
						float(lbx), float(lby), float(rbx), float(rby));
	rr.normalize_radius();
    if (_is_closed_path(path->path))
        path->path.concat_path(rr, 0);
    else
        path->path.join_path(rr, 0);
    global_status = STATUS_SUCCEED;
}


void PICAPI ps_path_clipping(ps_path* r, ps_path_operation op, const ps_path* a, const ps_path* b)
{
	if (!r) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
	}

	if (!a || !a->path.total_vertices() || !_is_closed_path(a->path)) {//invalid a 
		r->path = b->path;
    	global_status = STATUS_SUCCEED;
		return;
	}

	if (!b || !b->path.total_vertices() || !_is_closed_path(b->path)) {//invalid b 
		r->path = a->path;
    	global_status = STATUS_SUCCEED;
		return;
	}

	_path_operation((gpc_op_e)op, a->path, b->path, r->path);

    global_status = STATUS_SUCCEED;
}

#ifdef __cplusplus
}
#endif

namespace picasso {
//inner functions
ps_rect _path_bounding_rect(const path_storage & path)
{
    ps_rect r = {1, 1, 0, 0};
    float x1 = 1, y1 = 1, x2 = 0 ,y2 = 0;
    if (bounding_rect_single(const_cast<path_storage&>(path), 0, &x1, &y1, &x2, &y2)) {
        r.x = x1; r.y = y1; r.w = x2-x1; r.h = y2-y1; //flat rect
    }

    return r;
}

bool _is_closed_path(const path_storage & path)
{
    float x, y;
    unsigned int flag;
    if (!path.total_vertices ())
        return true;

    flag = path.last_vertex (&x, &y);
    if (flag & path_flags_close)
        return true;

    return false;
}

void _path_operation(gpc_op_e op, const path_storage& a, const path_storage& b, path_storage& r)
{
	conv_gpc<path_storage, path_storage> 
				gpc(const_cast<path_storage&>(a), const_cast<path_storage&>(b), op);
	gpc.rewind(0);
	r.remove_all();
	float x = 0, y = 0;
	unsigned cmd = 0;
	while (!is_stop(cmd = gpc.vertex(&x, &y))) {
		r.add_vertex(x, y, cmd);
	}
}

}
