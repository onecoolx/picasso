/* Picasso - a vector graphics library
 *
 * Copyright (C) 2011 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "math_type.h"
#include "geometry.h"
#include "matrix.h"
#include "graphic_base.h"
#include "graphic_path.h"

namespace picasso {

#define DEFAULT_VERTEICES  (8)

// graphic path special
graphic_path::graphic_path()
    : m_vertices(DEFAULT_VERTEICES)
    , m_cmds(DEFAULT_VERTEICES)
    , m_iterator(0)
{
}

graphic_path::~graphic_path()
{
}

graphic_path::graphic_path(const graphic_path& o)
{
    m_vertices = o.m_vertices;
    m_cmds = o.m_cmds;
    m_iterator = o.m_iterator;
}

graphic_path& graphic_path::operator=(const graphic_path& o)
{
    if (this == &o) {
        return *this;
    }

    m_vertices = o.m_vertices;
    m_cmds = o.m_cmds;
    m_iterator = o.m_iterator;

    return *this;
}

void graphic_path::remove_all(void)
{
    remove_all_impl();
    m_iterator = 0;
}

void graphic_path::free_all(void)
{
    remove_all();
}

uint32_t graphic_path::start_new_path(void)
{
    if (!is_stop(last_command_impl())) {
        add_vertex_impl(FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f), path_cmd_stop);
    }
    return total_vertices_impl();
}

void graphic_path::move_to(scalar x, scalar y)
{
    add_vertex_impl(x, y, path_cmd_move_to);
}

void graphic_path::move_rel(scalar dx, scalar dy)
{
    rel_to_abs(&dx, &dy);
    add_vertex_impl(dx, dy, path_cmd_move_to);
}

void graphic_path::line_to(scalar x, scalar y)
{
    add_vertex_impl(x, y, path_cmd_line_to);
}

void graphic_path::line_rel(scalar dx, scalar dy)
{
    rel_to_abs(&dx, &dy);
    add_vertex_impl(dx, dy, path_cmd_line_to);
}

void graphic_path::hline_to(scalar x)
{
    add_vertex_impl(x, last_y(), path_cmd_line_to);
}

void graphic_path::hline_rel(scalar dx)
{
    scalar dy = 0;
    rel_to_abs(&dx, &dy);
    add_vertex_impl(dx, dy, path_cmd_line_to);
}

void graphic_path::vline_to(scalar y)
{
    add_vertex_impl(last_x(), y, path_cmd_line_to);
}

void graphic_path::vline_rel(scalar dy)
{
    scalar dx = 0;
    rel_to_abs(&dx, &dy);
    add_vertex_impl(dx, dy, path_cmd_line_to);
}

void graphic_path::arc_to(scalar rx, scalar ry, scalar angle,
                          bool large_arc_flag, bool sweep_flag, scalar x, scalar y)
{
    if (total_vertices_impl() && is_vertex(last_command_impl())) {
        const scalar epsilon = FLT_TO_SCALAR(1e-30f);
        scalar x0 = 0.0f;
        scalar y0 = 0.0f;
        last_vertex_impl(&x0, &y0);

        rx = Fabs(rx);
        ry = Fabs(ry);

        // Ensure radii are valid
        //-------------------------
        if (rx < epsilon || ry < epsilon) {
            line_to(x, y);
            return;
        }

        if (calc_distance(x0, y0, x, y) < epsilon) {
            // If the endpoints (x, y) and (x0, y0) are identical, then this
            // is equivalent to omitting the elliptical arc segment entirely.
            return;
        }

        picasso::bezier_arc_svg a(x0, y0, rx, ry, angle, large_arc_flag, sweep_flag, x, y);
        if (a.radii_ok()) {
            join_path(a);
        } else {
            line_to(x, y);
        }
    } else {
        move_to(x, y);
    }
}

void graphic_path::arc_rel(scalar rx, scalar ry, scalar angle,
                           bool large_arc_flag, bool sweep_flag, scalar dx, scalar dy)
{
    rel_to_abs(&dx, &dy);
    arc_to(rx, ry, angle, large_arc_flag, sweep_flag, dx, dy);
}

void graphic_path::curve3(scalar x_ctrl, scalar y_ctrl, scalar x_to, scalar y_to)
{
    add_vertex_impl(x_ctrl, y_ctrl, path_cmd_curve3);
    add_vertex_impl(x_to, y_to, path_cmd_curve3);
}

void graphic_path::curve3_rel(scalar dx_ctrl, scalar dy_ctrl, scalar dx_to, scalar dy_to)
{
    rel_to_abs(&dx_ctrl, &dy_ctrl);
    rel_to_abs(&dx_to, &dy_to);
    add_vertex_impl(dx_ctrl, dy_ctrl, path_cmd_curve3);
    add_vertex_impl(dx_to, dy_to, path_cmd_curve3);
}

void graphic_path::curve3(scalar x_to, scalar y_to)
{
    scalar x0;
    scalar y0;
    if (is_vertex(last_vertex_impl(&x0, &y0))) {
        scalar x_ctrl;
        scalar y_ctrl;
        uint32_t cmd = prev_vertex_impl(&x_ctrl, &y_ctrl);
        if (is_curve(cmd)) {
            x_ctrl = x0 + x0 - x_ctrl;
            y_ctrl = y0 + y0 - y_ctrl;
        } else {
            x_ctrl = x0;
            y_ctrl = y0;
        }
        curve3(x_ctrl, y_ctrl, x_to, y_to);
    }
}

void graphic_path::curve3_rel(scalar dx_to, scalar dy_to)
{
    rel_to_abs(&dx_to, &dy_to);
    curve3(dx_to, dy_to);
}

void graphic_path::curve4(scalar x_ctrl1, scalar y_ctrl1, scalar x_ctrl2, scalar y_ctrl2, scalar x_to, scalar y_to)
{
    add_vertex_impl(x_ctrl1, y_ctrl1, path_cmd_curve4);
    add_vertex_impl(x_ctrl2, y_ctrl2, path_cmd_curve4);
    add_vertex_impl(x_to, y_to, path_cmd_curve4);
}

void graphic_path::curve4_rel(scalar dx_ctrl1, scalar dy_ctrl1, scalar dx_ctrl2,
                              scalar dy_ctrl2, scalar dx_to, scalar dy_to)
{
    rel_to_abs(&dx_ctrl1, &dy_ctrl1);
    rel_to_abs(&dx_ctrl2, &dy_ctrl2);
    rel_to_abs(&dx_to, &dy_to);
    add_vertex_impl(dx_ctrl1, dy_ctrl1, path_cmd_curve4);
    add_vertex_impl(dx_ctrl2, dy_ctrl2, path_cmd_curve4);
    add_vertex_impl(dx_to, dy_to, path_cmd_curve4);
}

void graphic_path::curve4(scalar x_ctrl2, scalar y_ctrl2, scalar x_to, scalar y_to)
{
    scalar x0;
    scalar y0;
    if (is_vertex(last_vertex(&x0, &y0))) {
        scalar x_ctrl1;
        scalar y_ctrl1;
        uint32_t cmd = prev_vertex(&x_ctrl1, &y_ctrl1);
        if (is_curve(cmd)) {
            x_ctrl1 = x0 + x0 - x_ctrl1;
            y_ctrl1 = y0 + y0 - y_ctrl1;
        } else {
            x_ctrl1 = x0;
            y_ctrl1 = y0;
        }
        curve4(x_ctrl1, y_ctrl1, x_ctrl2, y_ctrl2, x_to, y_to);
    }
}

void graphic_path::curve4_rel(scalar dx_ctrl2, scalar dy_ctrl2, scalar dx_to, scalar dy_to)
{
    rel_to_abs(&dx_ctrl2, &dy_ctrl2);
    rel_to_abs(&dx_to, &dy_to);
    curve4(dx_ctrl2, dy_ctrl2, dx_to, dy_to);
}

void graphic_path::end_poly(uint32_t flags)
{
    if (is_vertex(last_command_impl())) {
        add_vertex_impl(FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f), path_cmd_end_poly | flags);
    }
}

void graphic_path::close_polygon(uint32_t flags)
{
    end_poly(path_flags_close | flags);
}

scalar graphic_path::last_x(void) const
{
    return last_x_impl();
}

scalar graphic_path::last_y(void) const
{
    return last_y_impl();
}

uint32_t graphic_path::last_vertex(scalar* x, scalar* y) const
{
    return last_vertex_impl(x, y);
}

uint32_t graphic_path::prev_vertex(scalar* x, scalar* y) const
{
    return prev_vertex_impl(x, y);
}

uint32_t graphic_path::total_vertices(void) const
{
    return total_vertices_impl();
}

uint32_t graphic_path::total_byte_size(void) const
{
    return total_byte_size_impl();
}

void graphic_path::rel_to_abs(scalar* x, scalar* y) const
{
    if (total_vertices_impl()) {
        scalar x2;
        scalar y2;
        if (is_vertex(last_vertex_impl(&x2, &y2))) {
            *x += x2;
            *y += y2;
        }
    }
}

uint32_t graphic_path::vertex(uint32_t idx, scalar* x, scalar* y) const
{
    return vertex_impl(idx, x, y);
}

uint32_t graphic_path::command(uint32_t idx) const
{
    return command_impl(idx);
}

void graphic_path::modify_vertex(uint32_t idx, scalar x, scalar y)
{
    modify_vertex_impl(idx, x, y);
}

void graphic_path::modify_vertex(uint32_t idx, scalar x, scalar y, uint32_t cmd)
{
    modify_vertex_impl(idx, x, y, cmd);
}

void graphic_path::modify_command(uint32_t idx, uint32_t cmd)
{
    modify_command_impl(idx, cmd);
}

void graphic_path::rewind(uint32_t id)
{
    m_iterator = id;
}

uint32_t graphic_path::vertex(scalar* x, scalar* y)
{
    if (m_iterator >= total_vertices_impl()) {
        return path_cmd_stop;
    }
    return vertex_impl(m_iterator++, x, y);
}

void graphic_path::add_vertex(scalar x, scalar y, uint32_t cmd)
{
    add_vertex_impl(x, y, cmd);
}

uint32_t graphic_path::arrange_polygon_orientation(uint32_t start, uint32_t flag_orientation)
{
    if (flag_orientation == path_flags_none) {
        return start;
    }

    // Skip all non-vertices at the beginning
    while (start < total_vertices_impl() &&
           !is_vertex(command_impl(start))) { ++start; }

    // Skip all insignificant move_to
    while (start + 1 < total_vertices_impl() &&
           is_move_to(command_impl(start)) &&
           is_move_to(command_impl(start + 1))) { ++start; }

    // Find the last vertex
    uint32_t end = start + 1;
    while (end < total_vertices_impl() &&
           !is_next_poly(command_impl(end))) { ++end; }

    if (end - start > 2) {
        if (perceive_polygon_orientation(start, end) != flag_orientation) {
            // Invert polygon, set orientation flag, and skip all end_poly
            invert_polygon(start, end);
            uint32_t cmd;
            while (end < total_vertices_impl() &&
                   is_end_poly(cmd = command_impl(end))) {
                modify_command_impl(end++, set_orientation(cmd, flag_orientation));
            }
        }
    }
    return end;
}

uint32_t graphic_path::arrange_orientations(uint32_t start, uint32_t flag_orientation)
{
    if (flag_orientation != path_flags_none) {
        while (start < total_vertices_impl()) {
            start = arrange_polygon_orientation(start, flag_orientation);
            if (is_stop(command_impl(start))) {
                ++start;
                break;
            }
        }
    }
    return start;
}

void graphic_path::arrange_orientations_all_paths(uint32_t flag_orientation)
{
    if (flag_orientation != path_flags_none) {
        uint32_t start = 0;
        while (start < total_vertices_impl()) {
            start = arrange_orientations(start, flag_orientation);
        }
    }
}

uint32_t graphic_path::perceive_polygon_orientation(uint32_t start, uint32_t end)
{
    // Calculate signed area (scalar area to be exact)
    //---------------------
    uint32_t np = end - start;
    scalar area = FLT_TO_SCALAR(0.0f);
    uint32_t i;
    for (i = 0; i < np; i++) {
        scalar x1, y1, x2, y2;
        vertex_impl(start + i, &x1, &y1);
        vertex_impl(start + (i + 1) % np, &x2, &y2);
        area += x1 * y2 - y1 * x2;
    }
    return (area < FLT_TO_SCALAR(0.0f)) ? path_flags_cw : path_flags_ccw;
}

void graphic_path::invert_polygon(uint32_t start, uint32_t end)
{
    uint32_t i;
    uint32_t tmp_cmd = command_impl(start);

    --end; // Make "end" inclusive

    // Shift all commands to one position
    for (i = start; i < end; i++) {
        modify_command_impl(i, command_impl(i + 1));
    }

    // Assign starting command to the ending command
    modify_command_impl(end, tmp_cmd);

    // Reverse the polygon
    while (end > start) {
        swap_vertices_impl(start++, end--);
    }
}

void graphic_path::invert_polygon(uint32_t start)
{
    // Skip all non-vertices at the beginning
    while (start < total_vertices_impl() &&
           !is_vertex(command_impl(start))) { ++start; }

    // Skip all insignificant move_to
    while (start + 1 < total_vertices_impl() &&
           is_move_to(command_impl(start)) &&
           is_move_to(command_impl(start + 1))) { ++start; }

    // Find the last vertex
    uint32_t end = start + 1;
    while (end < total_vertices_impl() &&
           !is_next_poly(command_impl(end))) { ++end; }

    invert_polygon(start, end);
}

void graphic_path::flip_x(scalar x1, scalar x2)
{
    scalar x, y;
    for (uint32_t i = 0; i < total_vertices_impl(); i++) {
        uint32_t cmd = vertex_impl(i, &x, &y);
        if (is_vertex(cmd)) {
            modify_vertex_impl(i, x2 - x + x1, y);
        }
    }
}

void graphic_path::flip_y(scalar y1, scalar y2)
{
    scalar x, y;
    for (uint32_t i = 0; i < total_vertices_impl(); i++) {
        uint32_t cmd = vertex_impl(i, &x, &y);
        if (is_vertex(cmd)) {
            modify_vertex_impl(i, x, y2 - y + y1);
        }
    }
}

void graphic_path::translate(scalar dx, scalar dy, uint32_t id)
{
    uint32_t num_ver = total_vertices_impl();
    for (uint32_t path_id = id; path_id < num_ver; path_id++) {
        scalar x, y;
        uint32_t cmd = vertex_impl(path_id, &x, &y);
        if (is_stop(cmd)) {
            break;
        }

        if (is_vertex(cmd)) {
            x += dx;
            y += dy;
            modify_vertex_impl(path_id, x, y);
        }
    }
}

void graphic_path::translate_all_paths(scalar dx, scalar dy)
{
    uint32_t idx;
    uint32_t num_ver = total_vertices_impl();
    for (idx = 0; idx < num_ver; idx++) {
        scalar x, y;
        if (is_vertex(vertex_impl(idx, &x, &y))) {
            x += dx;
            y += dy;
            modify_vertex_impl(idx, x, y);
        }
    }
}

void graphic_path::transform(const trans_affine& trans, uint32_t id)
{
    uint32_t num_ver = total_vertices_impl();
    for (uint32_t path_id = id; path_id < num_ver; path_id++) {
        scalar x, y;
        uint32_t cmd = vertex_impl(path_id, &x, &y);
        if (is_stop(cmd)) {
            break;
        }

        if (is_vertex(cmd)) {
            trans.transform(&x, &y);
            modify_vertex_impl(path_id, x, y);
        }
    }
}

void graphic_path::transform_all_paths(const trans_affine& trans)
{
    uint32_t idx;
    uint32_t num_ver = total_vertices_impl();
    for (idx = 0; idx < num_ver; idx++) {
        scalar x, y;
        if (is_vertex(vertex_impl(idx, &x, &y))) {
            trans.transform(&x, &y);
            modify_vertex_impl(idx, x, y);
        }
    }
}

void graphic_path::join_path(vertex_source& vs, uint32_t id)
{
    scalar x = 0, y = 0;
    uint32_t cmd;
    vs.rewind(id);
    cmd = vs.vertex(&x, &y);
    if (!is_stop(cmd)) {
        if (is_vertex(cmd)) {
            scalar x0, y0;
            uint32_t cmd0 = last_vertex(&x0, &y0);
            if (is_vertex(cmd0)) {
                if (calc_distance(x, y, x0, y0) > FLT_TO_SCALAR(vertex_dist_epsilon)) {
                    if (is_move_to(cmd)) {
                        cmd = path_cmd_line_to;
                    }
                    add_vertex_impl(x, y, cmd);
                }
            } else {
                if (is_stop(cmd0)) {
                    cmd = path_cmd_move_to;
                } else {
                    if (is_move_to(cmd)) {
                        cmd = path_cmd_line_to;
                    }
                }
                add_vertex_impl(x, y, cmd);
            }
        }

        while (!is_stop(cmd = vs.vertex(&x, &y))) {
            add_vertex_impl(x, y, is_move_to(cmd) ? path_cmd_line_to : cmd);
        }
    }
}

void graphic_path::concat_path(vertex_source& vs, uint32_t id)
{
    scalar x = 0, y = 0;
    uint32_t cmd;
    vs.rewind(id);
    while (!is_stop(cmd = vs.vertex(&x, &y))) {
        add_vertex_impl(x, y, cmd);
    }
}

void graphic_path::serialize_to(byte* buffer)
{
    uint32_t num = total_vertices_impl();
    mem_copy(buffer, m_vertices.data(), num * sizeof(vertex_s));
    buffer += num * sizeof(vertex_s);
    mem_copy(buffer, m_cmds.data(), num * sizeof(uint32_t));
}

void graphic_path::serialize_from(uint32_t num, byte* buffer, uint32_t buf_len)
{
    //FIXME: paramer check and buf_len!
    remove_all();
    m_vertices.resize(num);
    m_vertices.set_data(num, (vertex_s*)buffer);
    buffer += num * sizeof(vertex_s);
    m_cmds.resize(num);
    m_cmds.set_data(num, (uint32_t*)buffer);
}

void graphic_path::remove_all_impl(void)
{
    m_vertices.clear();
    m_cmds.clear();
}

void graphic_path::add_vertex_impl(scalar x, scalar y, uint32_t cmd)
{
    if (m_vertices.is_full()) {
        m_vertices.resize(m_vertices.capacity() << 1);
        m_cmds.resize(m_cmds.capacity() << 1);
    }

    m_vertices.push_back(vertex_s(x, y));
    m_cmds.push_back(cmd);
}

void graphic_path::modify_vertex_impl(uint32_t idx, scalar x, scalar y)
{
    vertex_s& v = m_vertices[idx];
    v.x = x;
    v.y = y;
}

void graphic_path::modify_vertex_impl(uint32_t idx, scalar x, scalar y, uint32_t cmd)
{
    vertex_s& v = m_vertices[idx];
    v.x = x;
    v.y = y;
    m_cmds[idx] = cmd;
}

void graphic_path::modify_command_impl(uint32_t idx, uint32_t cmd)
{
    m_cmds[idx] = cmd;
}

void graphic_path::swap_vertices_impl(uint32_t v1, uint32_t v2)
{
    vertex_s t = m_vertices[v1];
    uint32_t c = m_cmds[v1];

    m_vertices[v1] = m_vertices[v2];
    m_cmds[v1] = m_cmds[v2];

    m_vertices[v2] = t;
    m_cmds[v2] = c;
}

uint32_t graphic_path::last_command_impl(void) const
{
    return m_cmds.size() ? m_cmds[m_cmds.size() - 1] : path_cmd_stop;
}

uint32_t graphic_path::last_vertex_impl(scalar* x, scalar* y) const
{
    if (m_vertices.size() == 0) {
        *x = *y = FLT_TO_SCALAR(0.0f);
        return path_cmd_stop;
    }
    return vertex(m_vertices.size() - 1, x, y);
}

uint32_t graphic_path::prev_vertex_impl(scalar* x, scalar* y) const
{
    if (m_vertices.size() < 2) {
        *x = *y = FLT_TO_SCALAR(0.0f);
        return path_cmd_stop;
    }
    return vertex(m_vertices.size() - 2, x, y);
}

scalar graphic_path::last_x_impl(void) const
{
    return m_vertices.size() ? m_vertices[m_vertices.size() - 1].x : FLT_TO_SCALAR(0.0f);
}

scalar graphic_path::last_y_impl(void) const
{
    return m_vertices.size() ? m_vertices[m_vertices.size() - 1].y : FLT_TO_SCALAR(0.0f);
}

uint32_t graphic_path::total_vertices_impl(void) const
{
    return m_vertices.size();
}

uint32_t graphic_path::total_byte_size_impl(void) const
{
    return m_vertices.size() * sizeof(vertex_s) + m_cmds.size() * sizeof(uint32_t);
}

uint32_t graphic_path::vertex_impl(uint32_t idx, scalar* x, scalar* y) const
{
    const vertex_s& v = m_vertices[idx];
    *x = v.x;
    *y = v.y;
    return m_cmds[idx];
}

uint32_t graphic_path::command_impl(uint32_t idx) const
{
    return m_cmds[idx];
}

}
