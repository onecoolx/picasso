/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2011 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _CONVERT_H_
#define _CONVERT_H_

#include "common.h"
#include "vertex.h"
#include "geometry.h"
#include "data_vector.h"
#include "picasso_matrix.h"
#include "picasso_gpc.h"

namespace picasso {

// Convert curve
class conv_curve : public vertex_source
{
public:
	conv_curve(const vertex_source& v) 
		: m_source(const_cast<vertex_source*>(&v)) 
    { 
    }

	virtual void rewind(unsigned int id) 
    { 
        m_source->rewind(id); 
        m_last_x = FLT_TO_SCALAR(0.0f);
        m_last_y = FLT_TO_SCALAR(0.0f);
        m_curve3.reset();
        m_curve4.reset();
    }

	virtual unsigned int vertex(scalar* x, scalar* y) 
    { 
        if (!is_stop(m_curve3.vertex(x, y))) {
            m_last_x = *x;
            m_last_y = *y;
            return path_cmd_line_to;
        }

        if (!is_stop(m_curve4.vertex(x, y))) {
            m_last_x = *x;
            m_last_y = *y;
            return path_cmd_line_to;
        }

        scalar ct2_x = FLT_TO_SCALAR(0.0f);
        scalar ct2_y = FLT_TO_SCALAR(0.0f);
        scalar end_x = FLT_TO_SCALAR(0.0f);
        scalar end_y = FLT_TO_SCALAR(0.0f);

        unsigned int cmd = m_source->vertex(x, y);
        switch(cmd) {
            case path_cmd_curve3:
                m_source->vertex(&end_x, &end_y);
                m_curve3.init(m_last_x, m_last_y, *x, *y, end_x, end_y);
                m_curve3.vertex(x, y);  
                m_curve3.vertex(x, y);  
                cmd = path_cmd_line_to;
                break;
            case path_cmd_curve4:
                m_source->vertex(&ct2_x, &ct2_y);
                m_source->vertex(&end_x, &end_y);
                m_curve4.init(m_last_x, m_last_y, *x, *y, ct2_x, ct2_y, end_x, end_y);
                m_curve4.vertex(x, y); 
                m_curve4.vertex(x, y); 
                cmd = path_cmd_line_to;
                break;
        }
        m_last_x = *x;
        m_last_y = *y;
        return cmd;
    }

private:
    conv_curve(const conv_curve&);
    conv_curve& operator=(const conv_curve&);

	vertex_source* m_source;
    scalar m_last_x;
    scalar m_last_y;
    curve3 m_curve3;
    curve4 m_curve4;    
};

// Convert transformer
class conv_transform : public vertex_source
{
public:
	conv_transform(const vertex_source& v, const trans_affine& m) 
		: m_source(const_cast<vertex_source*>(&v)), m_trans(&m) { }

	void attach(const vertex_source& v) { m_source = const_cast<vertex_source*>(&v); }

	void transformer(const trans_affine& t) { m_trans = &t; }

	virtual void rewind(unsigned int id) { m_source->rewind(id); }

	virtual unsigned int vertex(scalar* x, scalar* y) 
	{
		unsigned int cmd = m_source->vertex(x, y);
		if (is_vertex(cmd)) {
			m_trans->transform(x, y);
		}
		return cmd;
   	}
private:
	conv_transform(const conv_transform&);
	conv_transform& operator=(const conv_transform&);

	vertex_source* m_source;
	const trans_affine* m_trans;
};

// Convert clipper
class conv_clipper : public vertex_source
{
public:
	typedef enum {
		clip_union,
		clip_intersect,
		clip_xor,
		clip_diff,
	} clip_op;
	
    typedef enum
    {
        status_move_to,
        status_line_to,
        status_stop,
    } status;

    typedef struct 
    {
        int num_vertices;
        int hole_flag;
        gpc_vertex* vertices;
    } contour_header;

	conv_clipper(const vertex_source& a, const vertex_source& b, clip_op op) 
        : m_src_a(const_cast<vertex_source*>(&a))
        , m_src_b(const_cast<vertex_source*>(&b))
        , m_status(status_move_to)
        , m_vertex(-1)
        , m_contour(-1)
        , m_operation(op)
    {
        memset(&m_poly_a, 0, sizeof(m_poly_a));
        memset(&m_poly_b, 0, sizeof(m_poly_b));
        memset(&m_result, 0, sizeof(m_result));
    }

    virtual ~conv_clipper()
    {
        free_all();
    }

	virtual void rewind(unsigned int id) 
    {
        free_result();
        m_src_a->rewind(id);
        m_src_b->rewind(id);
        add(m_poly_a, *m_src_a);
        add(m_poly_b, *m_src_b);

        switch(m_operation)
        {
           case clip_union:
               gpc_polygon_clip(GPC_UNION, &m_poly_a, &m_poly_b, &m_result);
               break;
           case clip_intersect:
               gpc_polygon_clip(GPC_INT, &m_poly_a, &m_poly_b, &m_result);
               break;
           case clip_xor:
               gpc_polygon_clip(GPC_XOR, &m_poly_a, &m_poly_b, &m_result);
               break;
           case clip_diff:
               gpc_polygon_clip(GPC_DIFF, &m_poly_a, &m_poly_b, &m_result);
               break;
        }

        m_status = status_move_to;
        m_contour = -1;
        m_vertex = -1;
    }

	virtual unsigned int vertex(scalar* x, scalar* y) 
    { 
        if (m_status == status_move_to) {
            if(next_contour()) {
                if(next_vertex(x, y)) {
                    m_status = status_line_to;
                    return path_cmd_move_to;
                }
                m_status = status_stop;
                return path_cmd_end_poly | path_flags_close;
            }
        } else {
            if (next_vertex(x, y)) {
                return path_cmd_line_to;
            } else {
                m_status = status_move_to;
            }
            return path_cmd_end_poly | path_flags_close;
        }
        return path_cmd_stop;
    }
private:
    void free_result(void) 
    {
        if (m_result.contour) {
            gpc_free_polygon(&m_result);
        }
        memset(&m_result, 0, sizeof(m_result));
    }
    
    void free_all(void)
    {
        free_polygon(m_poly_a);
        free_polygon(m_poly_b);
        free_result();
    }

    void free_polygon(gpc_polygon& p)
    {
        for (int i = 0; i < p.num_contours; i++) {
            pod_allocator<gpc_vertex>::deallocate(p.contour[i].vertex, 
                                                  p.contour[i].num_vertices);
        }
        pod_allocator<gpc_vertex_list>::deallocate(p.contour, p.num_contours);
        memset(&p, 0, sizeof(gpc_polygon));
    }

    bool next_contour(void)
    {
        if (++m_contour < m_result.num_contours) {
            m_vertex = -1;
            return true;
        }
        return false;
    }

    bool next_vertex(scalar* x, scalar* y)
    {
        const gpc_vertex_list& vlist = m_result.contour[m_contour];
        if (++m_vertex < vlist.num_vertices) {
            const gpc_vertex& v = vlist.vertex[m_vertex];
            *x = v.x;
            *y = v.y;
            return true;
        }
        return false;
    }

    void add(gpc_polygon& p, vertex_source& src)
    {
        unsigned int cmd;
        scalar x, y;
        scalar start_x = FLT_TO_SCALAR(0.0f);
        scalar start_y = FLT_TO_SCALAR(0.0f);
        bool line_to = false;
        unsigned int orientation = 0;

        m_contour_accumulator.remove_all();

        while(!is_stop(cmd = src.vertex(&x, &y))) {
            if (is_vertex(cmd)) {
                if (is_move_to(cmd)) {
                    if (line_to) {
                        end_contour(orientation);
                        orientation = 0;
                    }
                    start_contour();
                    start_x = x;
                    start_y = y;
                }
                add_vertex(x, y);
                line_to = true;
            } else {
                if (is_end_poly(cmd)) {
                    orientation = get_orientation(cmd);
                    if (line_to && is_closed(cmd)) {
                        add_vertex(start_x, start_y);
                    }
                }
            }
        }

        if (line_to) {
            end_contour(orientation);
        }
        make_polygon(p);
    }

    void add_vertex(scalar x, scalar y)
    {
        gpc_vertex v;
        v.x = x;
        v.y = y;
        m_vertex_accumulator.add(v);
    }

    void start_contour(void)
    {
        contour_header h;
        memset(&h, 0, sizeof(h));
        m_contour_accumulator.add(h);
        m_vertex_accumulator.remove_all();
    }

    void end_contour(unsigned int orientation)
    {
        if (m_contour_accumulator.size()) {
            if (m_vertex_accumulator.size() > 2) {
                contour_header& h = m_contour_accumulator[m_contour_accumulator.size() - 1];

                h.num_vertices = m_vertex_accumulator.size();
                h.hole_flag = 0;
                h.vertices = pod_allocator<gpc_vertex>::allocate(h.num_vertices);
                gpc_vertex* d = h.vertices;
                for (int i = 0; i < h.num_vertices; i++) {
                    const gpc_vertex& s = m_vertex_accumulator[i];
                    d->x = s.x;
                    d->y = s.y;
                    ++d;
                }
            } else {
                m_vertex_accumulator.remove_last();
            }
        }
    }

    void make_polygon(gpc_polygon& p)
    {
        free_polygon(p);
        if (m_contour_accumulator.size()) {
            p.num_contours = m_contour_accumulator.size();
            p.hole = 0;
            p.contour = pod_allocator<gpc_vertex_list>::allocate(p.num_contours);

            gpc_vertex_list* pv = p.contour;
            for (int i = 0; i < p.num_contours; i++) {
                const contour_header& h = m_contour_accumulator[i];
                pv->num_vertices = h.num_vertices;
                pv->vertex = h.vertices;
                ++pv;
            }
        }
    }
private:
    conv_clipper(const conv_clipper&);
    conv_clipper& operator=(const conv_clipper&);

    vertex_source* m_src_a;
    vertex_source* m_src_b;
    status m_status;
    int m_vertex;
    int m_contour;
    clip_op m_operation;
    pod_bvector<gpc_vertex> m_vertex_accumulator;
    pod_bvector<contour_header> m_contour_accumulator;
    gpc_polygon m_poly_a;
    gpc_polygon m_poly_b;
    gpc_polygon m_result;
};

}
#endif/*_CONVERT_H_*/

