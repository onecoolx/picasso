/* Picasso - a vector graphics library
 *
 * Copyright (C) 2011 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GRAPHIC_PATH_H_
#define _GRAPHIC_PATH_H_

#include "common.h"
#include "vertex.h"
#include "graphic_base.h"
#include "data_vector.h"

namespace picasso {

class trans_affine;

class graphic_path : public vertex_container
{
public:
    graphic_path();
    graphic_path(const graphic_path& o);

    virtual ~graphic_path();

    graphic_path& operator=(const graphic_path& o);

    void free_all(void);

    //Make path functions
    uint32_t start_new_path(void);

    void move_to(scalar x, scalar y);
    void move_rel(scalar dx, scalar dy);

    void line_to(scalar x, scalar y);
    void line_rel(scalar dx, scalar dy);

    void hline_to(scalar x);
    void hline_rel(scalar dx);

    void vline_to(scalar y);
    void vline_rel(scalar dy);

    void arc_to(scalar rx, scalar ry, scalar angle,
                bool large_arc_flag, bool sweep_flag, scalar x, scalar y);

    void arc_rel(scalar rx, scalar ry, scalar angle,
                 bool large_arc_flag, bool sweep_flag, scalar dx, scalar dy);

    void curve3(scalar x_ctrl, scalar y_ctrl, scalar x_to, scalar y_to);
    void curve3_rel(scalar dx_ctrl, scalar dy_ctrl, scalar dx_to, scalar dy_to);

    void curve3(scalar x_to, scalar y_to);
    void curve3_rel(scalar dx_to, scalar dy_to);

    void curve4(scalar x_ctrl1, scalar y_ctrl1, scalar x_ctrl2, scalar y_ctrl2, scalar x_to, scalar y_to);
    void curve4_rel(scalar dx_ctrl1, scalar dy_ctrl1, scalar dx_ctrl2, scalar dy_ctrl2, scalar dx_to, scalar dy_to);

    void curve4(scalar x_ctrl2, scalar y_ctrl2, scalar x_to, scalar y_to);
    void curve4_rel(scalar x_ctrl2, scalar y_ctrl2, scalar x_to, scalar y_to);

    void end_poly(uint32_t flags = path_flags_close);
    void close_polygon(uint32_t flags = path_flags_none);

    // Accessors
    scalar last_x(void) const;
    scalar last_y(void) const;

    uint32_t last_vertex(scalar* x, scalar* y) const;
    uint32_t prev_vertex(scalar* x, scalar* y) const;

    uint32_t total_vertices(void) const;
    uint32_t total_byte_size(void) const;
    void rel_to_abs(scalar* x, scalar* y) const;

    uint32_t vertex(uint32_t idx, scalar* x, scalar* y) const;
    uint32_t command(uint32_t idx) const;

    void modify_vertex(uint32_t idx, scalar x, scalar y);
    void modify_vertex(uint32_t idx, scalar x, scalar y, uint32_t cmd);
    void modify_command(uint32_t idx, uint32_t cmd);

    uint32_t arrange_polygon_orientation(uint32_t start, uint32_t flag_orientation);
    uint32_t arrange_orientations(uint32_t start, uint32_t flag_orientation);
    void arrange_orientations_all_paths(uint32_t flag_orientation);
    void invert_polygon(uint32_t start);

    void flip_x(scalar x1, scalar x2);
    void flip_y(scalar y1, scalar y2);

    void translate(scalar dx, scalar dy, uint32_t id = 0);
    void translate_all_paths(scalar dx, scalar dy);

    void transform(const trans_affine& trans, uint32_t id = 0);
    void transform_all_paths(const trans_affine& trans);

    // path calc
    void join_path(vertex_source& vs, uint32_t id = 0);
    void concat_path(vertex_source& vs, uint32_t id = 0);

    // vertex source interface
    virtual void rewind(uint32_t id = 0);
    virtual uint32_t vertex(scalar* x, scalar* y);
    virtual void add_vertex(scalar x, scalar y, uint32_t cmd);
    virtual void remove_all(void);

    // serialize
    void serialize_to(byte* buffer);
    void serialize_from(uint32_t num, byte* buffer, uint32_t buf_len);
private:
    uint32_t perceive_polygon_orientation(uint32_t start, uint32_t end);
    void invert_polygon(uint32_t start, uint32_t end);

    void remove_all_impl(void);
    void add_vertex_impl(scalar x, scalar y, uint32_t cmd);
    void modify_vertex_impl(uint32_t idx, scalar x, scalar y);
    void modify_vertex_impl(uint32_t idx, scalar x, scalar y, uint32_t cmd);
    void modify_command_impl(uint32_t idx, uint32_t cmd);
    void swap_vertices_impl(uint32_t v1, uint32_t v2);
    uint32_t last_command_impl(void) const;
    uint32_t last_vertex_impl(scalar* x, scalar* y) const;
    uint32_t prev_vertex_impl(scalar* x, scalar* y) const;
    scalar last_x_impl(void) const;
    scalar last_y_impl(void) const;
    uint32_t total_vertices_impl(void) const;
    uint32_t total_byte_size_impl(void) const;
    uint32_t vertex_impl(uint32_t idx, scalar* x, scalar* y) const;
    uint32_t command_impl(uint32_t idx) const;

    pod_vector<vertex_s> m_vertices;
    pod_vector<uint32_t> m_cmds;
    uint32_t m_iterator;
};

inline bool operator != (const graphic_path& a, const graphic_path& b)
{
    if (a.total_vertices() != b.total_vertices()) {
        return true; // total vertices is not same , the path is not same
    }

    uint32_t num = a.total_vertices(); //because a total_vertices is same to b total_vertices.
    scalar x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    const_cast<graphic_path&>(a).rewind();
    const_cast<graphic_path&>(b).rewind();
    for (uint32_t i = 0; i < num; i++) {
        uint32_t cmd1 = a.vertex(i, &x1, &y1);
        uint32_t cmd2 = b.vertex(i, &x2, &y2);
        if ((cmd1 != cmd2) || (x1 != x2) || (y1 != y2)) {
            return true; // this vertex is not same.
        }
    }
    return false;
}

inline bool _is_closed_path(const graphic_path& path)
{
    scalar x, y;
    uint32_t flag;
    if (!path.total_vertices()) {
        return true;
    }

    flag = path.last_vertex(&x, &y);
    if (flag & path_flags_close) {
        return true;
    }

    return false;
}

}
#endif /*_GRAPHIC_PATH_H_*/
