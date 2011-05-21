/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include <stdlib.h>
#include <new>

#include "pconfig.h"
#include "picasso.h"
#include "picasso_utils.h"
#include "picasso_p.h"

using namespace picasso;

#ifdef __cplusplus
extern "C" {
#endif

ps_matrix* PICAPI ps_matrix_create_init(double xx, double yx, double xy, double yy, double x0, double y0)
{
    ps_matrix *p = (ps_matrix*)malloc(sizeof(ps_matrix));
    if (p) {
		p->refcount = 1;
        new ((void*)&(p->matrix)) trans_affine((float)xx, (float)yx, (float)xy, (float)yy, (float)x0, (float)y0); 
		global_status = STATUS_SUCCEED;
        return p;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_matrix* PICAPI ps_matrix_create(void)
{
    ps_matrix *p = (ps_matrix*)malloc(sizeof(ps_matrix));
    if (p) {
		p->refcount = 1;
        new ((void*)&(p->matrix)) trans_affine; 
		global_status = STATUS_SUCCEED;
        return p;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_matrix* PICAPI ps_matrix_create_copy(const ps_matrix* matrix)
{
	if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}

    ps_matrix *p = (ps_matrix*)malloc(sizeof(ps_matrix));
    if (p) {
		p->refcount = 1;
        new ((void*)&(p->matrix)) trans_affine; 
		p->matrix = matrix->matrix;
		global_status = STATUS_SUCCEED;
        return p;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_matrix* PICAPI ps_matrix_ref(ps_matrix* matrix)
{
	if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
		return 0;
	}
    matrix->refcount++;
    global_status = STATUS_SUCCEED;
    return matrix;
}

void PICAPI ps_matrix_unref(ps_matrix* matrix)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	matrix->refcount--;
	if (matrix->refcount <= 0) {
		(&matrix->matrix)->trans_affine::~trans_affine();
		free(matrix);
	}
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_init(ps_matrix* matrix, double xx, double yx, double xy, 
														double yy, double x0, double y0)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	matrix->matrix = trans_affine((float)xx, (float)yx, (float)xy, (float)yy, (float)x0, (float)y0);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_translate(ps_matrix* matrix, double tx, double ty)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    	matrix->matrix *= trans_affine_translation(float(iround((float)tx)), float(iround((float)ty)));
    	global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_scale(ps_matrix* matrix, double sx, double sy)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	matrix->matrix *= trans_affine_scaling((float)sx, (float)sy);
    	global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_rotate(ps_matrix* matrix, double angle)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	matrix->matrix *= trans_affine_rotation((float)angle);
    	global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_shear(ps_matrix* matrix, double sx, double sy)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	matrix->matrix *= trans_affine_skewing((float)sx, (float)sy);
    	global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_invert(ps_matrix* matrix)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	matrix->matrix.invert();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_flip_x(ps_matrix* matrix)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	matrix->matrix.flip_x();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_flip_y(ps_matrix* matrix)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	matrix->matrix.flip_y();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_reset(ps_matrix* matrix)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
	matrix->matrix.reset();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_multiply(ps_matrix* result, const ps_matrix* a, const ps_matrix* b)
{
	if (!result || !a || !b) {
		global_status = STATUS_INVALID_ARGUMENT;
		return;
	}
	result->matrix = a->matrix;
	result->matrix = result->matrix * b->matrix;
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_transform_point(const ps_matrix* matrix, ps_point* point)
{
    if (!matrix || !point) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	float x = float(point->x);
	float y = float(point->y);
    matrix->matrix.transform(&x, &y);
	point->x = x;
	point->y = y;
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_matrix_is_equal(const ps_matrix* a, const ps_matrix* b)
{
	if (!a || !b) {
		global_status = STATUS_INVALID_ARGUMENT;
		return False;
	}

    global_status = STATUS_SUCCEED;
	if (a == b)
		return True;

	if (a->matrix == b->matrix)
		return True;

	return False;
}

ps_bool PICAPI ps_matrix_is_identity(const ps_matrix* matrix)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return False;
    }

    global_status = STATUS_SUCCEED;
    return matrix->matrix.is_identity() ? True : False;
}

double PICAPI ps_matrix_get_determinant(const ps_matrix* matrix)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    global_status = STATUS_SUCCEED;
	return matrix->matrix.determinant();
}

void PICAPI ps_matrix_set_translate_factor(ps_matrix* matrix, double tx, double ty)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	matrix->matrix.tx = float(tx);
	matrix->matrix.ty = float(ty);
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_matrix_get_translate_factor(ps_matrix* matrix, double *tx, double *ty)
{
	if (!matrix || !tx || !ty) {
		global_status = STATUS_INVALID_ARGUMENT;
		return False;
	}

	*tx = matrix->matrix.tx;
	*ty = matrix->matrix.ty;
    global_status = STATUS_SUCCEED;
	return True;
}

void PICAPI ps_matrix_set_scale_factor(ps_matrix* matrix, double sx, double sy)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	matrix->matrix.sx = float(sx);
	matrix->matrix.sy = float(sy);
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_matrix_get_scale_factor(ps_matrix* matrix, double *sx, double *sy)
{
	if (!matrix || !sx || !sy) {
		global_status = STATUS_INVALID_ARGUMENT;
		return False;
	}

	*sx = matrix->matrix.sx;
	*sy = matrix->matrix.sy;
    global_status = STATUS_SUCCEED;
	return True;
}

void PICAPI ps_matrix_set_shear_factor(ps_matrix* matrix, double shx, double shy)
{
    if (!matrix) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

	matrix->matrix.shx = float(shx);
	matrix->matrix.shy = float(shy);
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_matrix_get_shear_factor(ps_matrix* matrix, double *shx, double *shy)
{
	if (!matrix || !shx || !shy) {
		global_status = STATUS_INVALID_ARGUMENT;
		return False;
	}

	*shx = matrix->matrix.shx;
	*shy = matrix->matrix.shy;
    global_status = STATUS_SUCCEED;
	return True;
}

void PICAPI ps_matrix_transform_rect(const ps_matrix* matrix, ps_rect* rect)
{
    if (!matrix || !rect) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    _matrix_transform_rect(matrix->matrix, rect);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_transform_path(const ps_matrix* matrix, ps_path* path)
{
    if (!matrix || !path) {
		global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    path->path.transform_all_paths(matrix->matrix);
    global_status = STATUS_SUCCEED;
}

#ifdef __cplusplus
}
#endif

namespace picasso {

//inner function
void _matrix_transform_rect(const trans_affine & matrix, ps_rect* rect)
{
    if ((matrix.shy == 0.0f) && (matrix.shx == 0.0f)) {
        float x = float(matrix.sx * rect->x + matrix.tx);
        float y = float(matrix.sy * rect->y + matrix.ty);
        float w = float(matrix.sx * rect->w);
        float h = float(matrix.sy * rect->h);
	    if ( w < 0 ) {
            w = -w;
            x -= w-1;
	    }
	    if ( h < 0 ) {
            h = -h;
            y -= h-1;
	    }
        rect->x = x; rect->y = y; rect->w = w; rect->h = h;
    } else {
        float x, y, w, h, x0, y0;
        float xmin, ymin, xmax, ymax;
        x0 = x = float(rect->x); y0 = y = float(rect->y); w = float(rect->w), h = float(rect->h);
        matrix.transform(&x0, &y0);
        xmin = xmax = x0;
        ymin = ymax = y0;

        x = float(rect->x+rect->w+1); y = float(rect->y);
        matrix.transform(&x, &y);
        xmin = MIN(xmin, x);
        ymin = MIN(ymin, y);
        xmax = MAX(xmax, x);
        ymax = MAX(ymax, y);

        x = float(rect->x+rect->w+1); y = float(rect->y+rect->h+1);
        matrix.transform(&x, &y);
        xmin = MIN(xmin, x);
        ymin = MIN(ymin, y);
        xmax = MAX(xmax, x);
        ymax = MAX(ymax, y);

        x = float(rect->x); y = float(rect->y+rect->h+1);
        matrix.transform(&x, &y);
        xmin = MIN(xmin, x);
        ymin = MIN(ymin, y);
        xmax = MAX(xmax, x);
        ymax = MAX(ymax, y);

        //rebuild rect
        w = xmax - xmin;
        h = ymax - ymin;

        xmin -= (xmin - x0) / w;
        ymin -= (ymin - y0) / h;
        xmax -= (xmax - x0) / w;
        ymax -= (ymax - y0) / h;

        rect->x = xmin; rect->y = ymin; rect->w = xmax-xmin+1; rect->h = ymax-ymin+1;
    }
}

}
