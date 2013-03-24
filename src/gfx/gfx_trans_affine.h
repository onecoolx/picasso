/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _GFX_TRANS_AFFINE_H_
#define _GFX_TRANS_AFFINE_H_

#include "common.h"
#include "interfaces.h"

#include "agg_trans_affine.h"

namespace gfx {

class gfx_trans_affine : public abstract_trans_affine
{
public:
	gfx_trans_affine(scalar sx, scalar shy, scalar shx, scalar sy, scalar tx, scalar ty);
	virtual ~gfx_trans_affine();

	virtual void sx(scalar v);
	virtual void sy(scalar v);
	virtual scalar sx(void) const;
	virtual scalar sy(void) const;
	virtual void shx(scalar v);
	virtual void shy(scalar v);
	virtual scalar shx(void) const;
	virtual scalar shy(void) const;
	virtual void tx(scalar v);
	virtual void ty(scalar v);
	virtual scalar tx(void) const;
	virtual scalar ty(void) const;

	virtual void translate(scalar x, scalar y);
	virtual void scale(scalar x, scalar y); 
	virtual void rotate(scalar a); 
	virtual void shear(scalar x, scalar y);

	virtual void invert(void);
	virtual void flip_x(void);
	virtual void flip_y(void);
	virtual void reset(void);
	virtual void multiply(const abstract_trans_affine* o);

	virtual bool is_identity(void) const;
	virtual scalar determinant(void) const;
	virtual scalar rotation(void) const;
	virtual void translation(scalar* dx, scalar* dy) const;
	virtual void scaling(scalar* x, scalar* y) const;
	virtual void shearing(scalar* x, scalar* y) const;

	virtual bool is_equal(const abstract_trans_affine* o);
	virtual void transform(scalar* x, scalar* y) const;
	virtual void transform_2x2(scalar* x, scalar* y) const;
    virtual void inverse_transform(scalar* x, scalar* y) const;

	virtual void store_to(scalar* m) const;
	virtual void load_from(const scalar* m);

	agg::trans_affine& impl(void) { return m_mtx; }
private:
	agg::trans_affine m_mtx;
};

inline agg::trans_affine stable_matrix(const agg::trans_affine& m)
{
	if (agg::is_boxer(m.rotation())) {
		scalar tx = Floor(m.tx);
		scalar ty = Floor(m.ty);
		return agg::trans_affine(m.sx, m.shy, m.shx, m.sy, SCALAR_TO_FLT(tx), SCALAR_TO_FLT(ty));
	} else  {
		return agg::trans_affine(m);
	}
}

}
#endif/*_GFX_TRANS_AFFINE_H_*/
