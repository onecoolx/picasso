/* Picasso - a vector graphics library
 *
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "matrix.h"

namespace picasso {

const scalar affine_epsilon = FLT_TO_SCALAR(1e-14f);

trans_affine::trans_affine()
    : m_sx(FLT_TO_SCALAR(1.0f)), m_shy(FLT_TO_SCALAR(0.0f))
    , m_shx(FLT_TO_SCALAR(0.0f)), m_sy(FLT_TO_SCALAR(1.0f))
    , m_tx(FLT_TO_SCALAR(0.0f)), m_ty(FLT_TO_SCALAR(0.0f))
{
}

trans_affine::trans_affine(scalar sx, scalar shy, scalar shx, scalar sy, scalar tx, scalar ty)
    : m_sx(sx), m_shy(shy), m_shx(shx), m_sy(sy), m_tx(tx), m_ty(ty)
{
}

trans_affine::trans_affine(const trans_affine& o)
{
    m_sx = o.m_sx;
    m_shy = o.m_shy;
    m_shx = o.m_shx;
    m_sy = o.m_sy;
    m_tx = o.m_tx;
    m_ty = o.m_ty;
}

trans_affine& trans_affine::operator=(const trans_affine& o)
{
    if (this == &o)
        return *this;

    m_sx = o.m_sx;
    m_shy = o.m_shy;
    m_shx = o.m_shx;
    m_sy = o.m_sy;
    m_tx = o.m_tx;
    m_ty = o.m_ty;

    return *this;
}

void trans_affine::transform(scalar* x, scalar* y) const
{
    _REGISTER_ scalar tmp = *x;
    *x = tmp * m_sx  + *y * m_shx + m_tx;
    *y = tmp * m_shy + *y * m_sy  + m_ty;
}

void trans_affine::transform_2x2(scalar* x, scalar* y) const
{
    _REGISTER_ scalar tmp = *x;
    *x = tmp * m_sx  + *y * m_shx;
    *y = tmp * m_shy + *y * m_sy;
}

void trans_affine::inverse_transform(scalar* x, scalar* y) const
{
    _REGISTER_ scalar d = determinant_reciprocal();
    _REGISTER_ scalar a = (*x - m_tx) * d;
    _REGISTER_ scalar b = (*y - m_ty) * d;
    *x = a * m_sy - b * m_shx;
    *y = b * m_sx - a * m_shy;
}

const trans_affine& trans_affine::translate(scalar x, scalar y)
{
    m_tx += x;
    m_ty += y;
    return *this;
}

const trans_affine& trans_affine::scale(scalar x, scalar y)
{
    scalar m0 = x;
    scalar m1 = y;
    m_sx  *= m0;
    m_shx *= m0;
    m_tx  *= m0;
    m_shy *= m1;
    m_sy  *= m1;
    m_ty  *= m1;
    return *this;
}

const trans_affine& trans_affine::rotate(scalar a)
{
    scalar ca = Cos(a);
    scalar sa = Sin(a);
    scalar t0 = m_sx  * ca - m_shy * sa;
    scalar t2 = m_shx * ca - m_sy * sa;
    scalar t4 = m_tx  * ca - m_ty * sa;
    m_shy = m_sx  * sa + m_shy * ca;
    m_sy  = m_shx * sa + m_sy * ca;
    m_ty  = m_tx  * sa + m_ty * ca;
    m_sx  = t0;
    m_shx = t2;
    m_tx  = t4;
    return *this;
}

const trans_affine& trans_affine::shear(scalar x, scalar y)
{
    scalar t0 = y * m_shx;
    scalar t1 = y * m_sy;
    scalar t2 = x * m_sx;
    scalar t3 = x * m_shy;
    m_sx += t0;
    m_shy += t1;
    m_shx += t2;
    m_sy += t3;
    return *this;
}

const trans_affine& trans_affine::invert(void)
{
    scalar d = determinant_reciprocal();

    scalar t0 = m_sy * d;
    m_sy  =  m_sx  * d;
    m_shy = -m_shy * d;
    m_shx = -m_shx * d;

    scalar t4 = -m_tx * t0 - m_ty * m_shx;
    m_ty = -m_tx * m_shy - m_ty * m_sy;

    m_sx = t0;
    m_tx = t4;
    return *this;
}

const trans_affine& trans_affine::flip_x(void)
{
    m_sx  = -m_sx;
    m_shy = -m_shy;
    m_tx  = -m_tx;
    return *this;
}

const trans_affine& trans_affine::flip_y(void)
{
    m_shx = -m_shx;
    m_sy  = -m_sy;
    m_ty  = -m_ty;
    return *this;
}

const trans_affine& trans_affine::reset(void)
{
    m_sx = m_sy = FLT_TO_SCALAR(1.0f);
    m_shy = m_shx = m_tx = m_ty = FLT_TO_SCALAR(0.0f);
    return *this;
}

const trans_affine& trans_affine::multiply(const trans_affine& o)
{
    scalar t0 = m_sx  * o.m_sx + m_shy * o.m_shx;
    scalar t2 = m_shx * o.m_sx + m_sy  * o.m_shx;
    scalar t4 = m_tx  * o.m_sx + m_ty  * o.m_shx + o.m_tx;
    m_shy = m_sx  * o.m_shy + m_shy * o.m_sy;
    m_sy  = m_shx * o.m_shy + m_sy  * o.m_sy;
    m_ty  = m_tx  * o.m_shy + m_ty  * o.m_sy + o.m_ty;
    m_sx  = t0;
    m_shx = t2;
    m_tx  = t4;
    return *this;
}

bool trans_affine::is_identity(void) const
{
    return is_equal_eps(m_sx,  FLT_TO_SCALAR(1.0f), affine_epsilon) &&
           is_equal_eps(m_shy, FLT_TO_SCALAR(0.0f), affine_epsilon) &&
           is_equal_eps(m_shx, FLT_TO_SCALAR(0.0f), affine_epsilon) &&
           is_equal_eps(m_sy,  FLT_TO_SCALAR(1.0f), affine_epsilon) &&
           is_equal_eps(m_tx,  FLT_TO_SCALAR(0.0f), affine_epsilon) &&
           is_equal_eps(m_ty,  FLT_TO_SCALAR(0.0f), affine_epsilon);
}

scalar trans_affine::determinant(void) const
{
    return m_sx * m_sy - m_shy * m_shx;
}

scalar trans_affine::rotation(void) const
{
    scalar x1 = FLT_TO_SCALAR(0.0f);
    scalar y1 = FLT_TO_SCALAR(0.0f);
    scalar x2 = FLT_TO_SCALAR(1.0f);
    scalar y2 = FLT_TO_SCALAR(0.0f);
    transform(&x1, &y1);
    transform(&x2, &y2);
    return Atan2(y2-y1, x2-x1);
}

bool trans_affine::is_equal(const trans_affine& o) const
{
    return is_equal_eps(m_sx,  o.sx(),  affine_epsilon) &&
           is_equal_eps(m_shy, o.shy(), affine_epsilon) &&
           is_equal_eps(m_shx, o.shx(), affine_epsilon) &&
           is_equal_eps(m_sy,  o.sy(),  affine_epsilon) &&
           is_equal_eps(m_tx,  o.tx(),  affine_epsilon) &&
           is_equal_eps(m_ty,  o.ty(),  affine_epsilon);
}

}
