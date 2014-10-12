/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "gfx_trans_affine.h"

namespace gfx {

gfx_trans_affine::gfx_trans_affine(scalar sx, scalar shy, scalar shx, scalar sy, scalar tx, scalar ty)
    : m_mtx(sx, shy, shx, sy, tx, ty)
{
}

gfx_trans_affine::~gfx_trans_affine()
{
}

void gfx_trans_affine::sx(scalar v)
{
    m_mtx.sx = v;
}

void gfx_trans_affine::sy(scalar v)
{
    m_mtx.sy = v;
}

scalar gfx_trans_affine::sx(void) const
{
    return m_mtx.sx;
}

scalar gfx_trans_affine::sy(void) const
{
    return m_mtx.sy;
}

void gfx_trans_affine::shx(scalar v)
{
    m_mtx.shx = v;
}

void gfx_trans_affine::shy(scalar v)
{
    m_mtx.shy = v;
}

scalar gfx_trans_affine::shx(void) const
{
    return m_mtx.shx;
}

scalar gfx_trans_affine::shy(void) const
{
    return m_mtx.shy;
}

void gfx_trans_affine::tx(scalar v)
{
    m_mtx.tx = v;
}

void gfx_trans_affine::ty(scalar v)
{
    m_mtx.ty = v;
}

scalar gfx_trans_affine::tx(void) const
{
    return m_mtx.tx;
}

scalar gfx_trans_affine::ty(void) const
{
    return m_mtx.ty;
}

void gfx_trans_affine::translate(scalar x, scalar y)
{
    m_mtx.translate(x, y);
}

void gfx_trans_affine::scale(scalar x, scalar y) 
{
    m_mtx.scale(x, y);
}

void gfx_trans_affine::rotate(scalar a) 
{
    m_mtx.rotate(a);
}

void gfx_trans_affine::shear(scalar x, scalar y)
{
    m_mtx.shear(x, y);
}

void gfx_trans_affine::invert(void)
{
    m_mtx.invert();
}

void gfx_trans_affine::flip_x(void)
{
    m_mtx.flip_x();
}

void gfx_trans_affine::flip_y(void)
{
    m_mtx.flip_y();
}

void gfx_trans_affine::reset(void)
{
    m_mtx.reset();
}

void gfx_trans_affine::multiply(const abstract_trans_affine* o)
{
    m_mtx.multiply(static_cast<const gfx_trans_affine*>(o)->m_mtx);
}

bool gfx_trans_affine::is_identity(void) const
{
    return m_mtx.is_identity();
}

scalar gfx_trans_affine::determinant(void) const
{
    return m_mtx.determinant();
}

scalar gfx_trans_affine::rotation(void) const
{
    return m_mtx.rotation();
}

void gfx_trans_affine::translation(scalar* dx, scalar* dy) const
{
    m_mtx.translation(dx, dy);
}

void gfx_trans_affine::scaling(scalar* x, scalar* y) const
{
    m_mtx.scaling(x, y);
}

void gfx_trans_affine::shearing(scalar* x, scalar* y) const
{
    m_mtx.shearing(x, y);
}

bool gfx_trans_affine::is_equal(const abstract_trans_affine* o)
{
    return m_mtx.is_equal(static_cast<const gfx_trans_affine*>(o)->m_mtx);
}

void gfx_trans_affine::transform(scalar* x, scalar* y) const
{
    m_mtx.transform(x, y);
}

void gfx_trans_affine::transform_2x2(scalar* x, scalar* y) const
{
    m_mtx.transform_2x2(x, y);
}

void gfx_trans_affine::inverse_transform(scalar* x, scalar* y) const
{
    m_mtx.inverse_transform(x, y);
}

void gfx_trans_affine::store_to(scalar* m) const
{
    m_mtx.store_to(m);
}

void gfx_trans_affine::load_from(const scalar* m)
{
    m_mtx.load_from(m);
}

}
