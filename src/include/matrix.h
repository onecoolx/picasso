/* Picasso - a vector graphics library
 *
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "common.h"
#include "graphic_base.h"
#include "math_type.h"

namespace picasso {

class trans_affine
{
public:
    trans_affine();
    trans_affine(scalar sx, scalar shy, scalar shx, scalar sy, scalar tx, scalar ty);
    trans_affine(const trans_affine& o);
    trans_affine& operator = (const trans_affine& o);

public:
    void sx(scalar v) { m_sx = v; }
    void sy(scalar v) { m_sy = v; }
    scalar sx(void) const { return m_sx; }
    scalar sy(void) const { return m_sy; }
    void shx(scalar v) { m_shx = v; }
    void shy(scalar v) { m_shy = v; }
    scalar shx(void) const { return m_shx; }
    scalar shy(void) const { return m_shy; }
    void tx(scalar v) { m_tx = v; }
    void ty(scalar v) { m_ty = v; }
    scalar tx(void) const { return m_tx; }
    scalar ty(void) const { return m_ty; }
public:
    const trans_affine& translate(scalar x, scalar y);
    const trans_affine& scale(scalar x, scalar y);
    const trans_affine& rotate(scalar a);
    const trans_affine& shear(scalar x, scalar y);

    const trans_affine& invert(void);
    const trans_affine& flip_x(void);
    const trans_affine& flip_y(void);
    const trans_affine& reset(void);
    const trans_affine& multiply(const trans_affine& o);

    bool is_identity(void) const;
    scalar determinant(void) const;
    scalar rotation(void) const;
    void transform(scalar* x, scalar* y) const;
    void transform_2x2(scalar* x, scalar* y) const;
    void inverse_transform(scalar* x, scalar* y) const;
    bool is_equal(const trans_affine& o) const;

    const trans_affine& operator *= (const trans_affine& o) { return multiply(o); }
protected:
    // Calculate the reciprocal of the determinant
    scalar determinant_reciprocal(void) const
    {
        return FLT_TO_SCALAR(1.0f) / (m_sx * m_sy - m_shy * m_shx);
    }
private:
    scalar m_sx;
    scalar m_shy;
    scalar m_shx;
    scalar m_sy;
    scalar m_tx;
    scalar m_ty;
};

inline bool operator == (const trans_affine& a, const trans_affine& b)
{
    return a.is_equal(b);
}

inline bool operator != (const trans_affine& a, const trans_affine& b)
{
    return !a.is_equal(b);
}

inline trans_affine operator * (const trans_affine& a, const trans_affine& b)
{
    return trans_affine(a).multiply(b);
}

// Translation matrix
class trans_affine_translation : public trans_affine
{
public:
    trans_affine_translation(scalar x, scalar y)
        : trans_affine(FLT_TO_SCALAR(1.0f), FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(1.0f), x, y)
    {
    }
};

// Scaling matrix. x, y - scale coefficients by X and Y respectively
class trans_affine_scaling : public trans_affine
{
public:
    trans_affine_scaling(scalar x, scalar y)
        : trans_affine(x, FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f), y, FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f))
    {
    }

    trans_affine_scaling(scalar s)
        : trans_affine(s, FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f), s, FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f))
    {
    }
};

// Rotation matrix. sin() and cos() are calculated twice for the same angle.
class trans_affine_rotation : public trans_affine
{
public:
    trans_affine_rotation(scalar a)
        : trans_affine(Cos(a), Sin(a), -(Sin(a)), Cos(a), FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f))
    {
    }
};

// Skewing (shear) matrix
class trans_affine_skewing : public trans_affine
{
public:
    trans_affine_skewing(scalar x, scalar y)
        : trans_affine(FLT_TO_SCALAR(1.0f), Tan(y), Tan(x), FLT_TO_SCALAR(1.0f), FLT_TO_SCALAR(0.0f), FLT_TO_SCALAR(0.0f))
    {
    }
};

// hack!! Stable matrix
inline trans_affine stable_matrix(const trans_affine& o)
{
    if (is_boxer(o.rotation())) {
        scalar tx = Floor(o.tx());
        scalar ty = Floor(o.ty());
        return trans_affine(o.sx(), o.shy(), o.shx(), o.sy(), SCALAR_TO_FLT(tx), SCALAR_TO_FLT(ty));
    } else {
        return trans_affine(o);
    }
}
}
#endif/*_MATRIX_H_*/
