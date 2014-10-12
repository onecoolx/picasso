/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2008 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#include "common.h"
#include "device.h"
#include "interfaces.h"
#include "graphic_path.h"

#include "picasso.h"
#include "picasso_global.h"
#include "picasso_matrix.h"
#include "picasso_objects.h"

namespace picasso {

trans_affine::trans_affine()
{
    //Identity matrix
    m_impl = get_system_device()->create_trans_affine(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
}

trans_affine::trans_affine(scalar sx, scalar shy, scalar shx, scalar sy, scalar tx, scalar ty)
{
    //Custom matrix
    m_impl = get_system_device()->create_trans_affine(sx, shy, shx, sy, tx, ty);
}

trans_affine::trans_affine(const trans_affine& o)
{
    m_impl = get_system_device()->create_trans_affine(o.sx(), o.shy(), o.shx(), o.sy(), o.tx(), o.ty());
}

trans_affine& trans_affine::operator=(const trans_affine& o)
{
    if (this == &o)
        return *this;

    get_system_device()->destroy_trans_affine(m_impl);
    m_impl = get_system_device()->create_trans_affine(o.sx(), o.shy(), o.shx(), o.sy(), o.tx(), o.ty());

    return *this;
}

trans_affine::~trans_affine()
{
    get_system_device()->destroy_trans_affine(m_impl);
}

void trans_affine::sx(scalar v) 
{
    m_impl->sx(v);
}

void trans_affine::sy(scalar v)
{
    m_impl->sy(v);
}

scalar trans_affine::sx(void) const
{
    return m_impl->sx();
}

scalar trans_affine::sy(void) const
{
    return m_impl->sy();
}

void trans_affine::shx(scalar v)
{
    m_impl->shx(v);
}

void trans_affine::shy(scalar v) 
{
    m_impl->shy(v);
}

scalar trans_affine::shx(void) const
{
    return m_impl->shx();
}

scalar trans_affine::shy(void) const
{
    return m_impl->shy();
}

void trans_affine::tx(scalar v)
{
    m_impl->tx(v);
}

void trans_affine::ty(scalar v)
{
    m_impl->ty(v);
}

scalar trans_affine::tx(void) const
{
    return m_impl->tx();
}

scalar trans_affine::ty(void) const
{
    return m_impl->ty();
}

void trans_affine::transform(scalar* x, scalar* y) const
{
    m_impl->transform(x, y);
}

void trans_affine::transform_2x2(scalar* x, scalar* y) const
{
    m_impl->transform_2x2(x, y);
}

void trans_affine::inverse_transform(scalar* x, scalar* y) const
{
    m_impl->inverse_transform(x, y);
}

const trans_affine& trans_affine::translate(scalar x, scalar y)
{
    m_impl->translate(x, y);
    return *this;
}

const trans_affine& trans_affine::scale(scalar x, scalar y) 
{
    m_impl->scale(x, y);
    return *this;
}

const trans_affine& trans_affine::rotate(scalar a) 
{
    m_impl->rotate(a);
    return *this;
}

const trans_affine& trans_affine::shear(scalar x, scalar y)
{
    m_impl->shear(x, y);
    return *this;
}

const trans_affine& trans_affine::invert(void)
{
    m_impl->invert();
    return *this;
}

const trans_affine& trans_affine::flip_x(void)
{
    m_impl->flip_x();
    return *this;
}

const trans_affine& trans_affine::flip_y(void)
{
    m_impl->flip_y();
    return *this;
}

const trans_affine& trans_affine::reset(void)
{
    m_impl->reset();
    return *this;
}

const trans_affine& trans_affine::multiply(const trans_affine& o)
{
    m_impl->multiply(o.m_impl);
    return *this;
}

bool trans_affine::is_identity(void) const
{
    return m_impl->is_identity();
}

scalar trans_affine::determinant(void) const
{
    return m_impl->determinant();
}

scalar trans_affine::rotation(void) const
{
    return m_impl->rotation();
}

void trans_affine::translation(scalar* dx, scalar* dy) const
{
    m_impl->translation(dx, dy);
}

void trans_affine::scaling(scalar* x, scalar* y) const
{
    m_impl->scaling(x, y);
}
    
void trans_affine::shearing(scalar* x, scalar* y) const
{
    m_impl->shearing(x, y);
}

void trans_affine::store_to(scalar* m) const
{
    m_impl->store_to(m);
}

void trans_affine::load_from(const scalar* m)
{
    m_impl->load_from(m);
}

//inner function
static void _matrix_transform_rect(const trans_affine & matrix, ps_rect* rect)
{
    if ((matrix.shy() == DBL_TO_SCALAR(0.0)) && (matrix.shx() == DBL_TO_SCALAR(0.0))) {
        scalar x = matrix.sx() * DBL_TO_SCALAR(rect->x) + matrix.tx();
        scalar y = matrix.sy() * DBL_TO_SCALAR(rect->y) + matrix.ty();
        scalar w = matrix.sx() * DBL_TO_SCALAR(rect->w);
        scalar h = matrix.sy() * DBL_TO_SCALAR(rect->h);
        if ( w < 0 ) {
            w = -w;
            x -= w-1;
        }
        if ( h < 0 ) {
            h = -h;
            y -= h-1;
        }
        rect->x = SCALAR_TO_DBL(x); rect->y = SCALAR_TO_DBL(y); 
        rect->w = SCALAR_TO_DBL(w); rect->h = SCALAR_TO_DBL(h);
    } else {
        scalar x, y, w, h, x0, y0;
        scalar xmin, ymin, xmax, ymax;
        x0 = x = DBL_TO_SCALAR(rect->x); y0 = y = DBL_TO_SCALAR(rect->y); 
        w = DBL_TO_SCALAR(rect->w); h = DBL_TO_SCALAR(rect->h);
        matrix.transform(&x0, &y0);
        xmin = xmax = x0;
        ymin = ymax = y0;

        x = DBL_TO_SCALAR(rect->x+rect->w+1); y = DBL_TO_SCALAR(rect->y);
        matrix.transform(&x, &y);
        xmin = MIN(xmin, x);
        ymin = MIN(ymin, y);
        xmax = MAX(xmax, x);
        ymax = MAX(ymax, y);

        x = DBL_TO_SCALAR(rect->x+rect->w+1); y = DBL_TO_SCALAR(rect->y+rect->h+1);
        matrix.transform(&x, &y);
        xmin = MIN(xmin, x);
        ymin = MIN(ymin, y);
        xmax = MAX(xmax, x);
        ymax = MAX(ymax, y);

        x = DBL_TO_SCALAR(rect->x); y = DBL_TO_SCALAR(rect->y+rect->h+1);
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

        rect->x = SCALAR_TO_DBL(xmin); rect->y = SCALAR_TO_DBL(ymin); 
        rect->w = SCALAR_TO_DBL(xmax-xmin+1); rect->h = SCALAR_TO_DBL(ymax-ymin+1);
    }
}

}

#ifdef __cplusplus
extern "C" {
#endif

ps_matrix* PICAPI ps_matrix_create_init(double xx, double yx, double xy, double yy, double x0, double y0)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return 0;
    }

    ps_matrix *p = (ps_matrix*)mem_malloc(sizeof(ps_matrix));
    if (p) {
        p->refcount = 1;
        new ((void*)&(p->matrix)) picasso::trans_affine(DBL_TO_SCALAR(xx), DBL_TO_SCALAR(yx), 
                               DBL_TO_SCALAR(xy), DBL_TO_SCALAR(yy), DBL_TO_SCALAR(x0), DBL_TO_SCALAR(y0)); 
        global_status = STATUS_SUCCEED;
        return p;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_matrix* PICAPI ps_matrix_create(void)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return 0;
    }

    ps_matrix *p = (ps_matrix*)mem_malloc(sizeof(ps_matrix));
    if (p) {
        p->refcount = 1;
        new ((void*)&(p->matrix)) picasso::trans_affine; 
        global_status = STATUS_SUCCEED;
        return p;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_matrix* PICAPI ps_matrix_create_copy(const ps_matrix* matrix)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return 0;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    ps_matrix *p = (ps_matrix*)mem_malloc(sizeof(ps_matrix));
    if (p) {
        p->refcount = 1;
        new ((void*)&(p->matrix))picasso::trans_affine(matrix->matrix);
        global_status = STATUS_SUCCEED;
        return p;
    } else {
        global_status = STATUS_OUT_OF_MEMORY;
        return 0;
    }
}

ps_matrix* PICAPI ps_matrix_ref(ps_matrix* matrix)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return 0;
    }

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
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    matrix->refcount--;
    if (matrix->refcount <= 0) {
        (&matrix->matrix)->trans_affine::~trans_affine();
        mem_free(matrix);
    }
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_init(ps_matrix* matrix, double xx, double yx, double xy, 
                                                        double yy, double x0, double y0)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    matrix->matrix = picasso::trans_affine(DBL_TO_SCALAR(xx), DBL_TO_SCALAR(yx), 
                    DBL_TO_SCALAR(xy), DBL_TO_SCALAR(yy), DBL_TO_SCALAR(x0), DBL_TO_SCALAR(y0));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_translate(ps_matrix* matrix, double tx, double ty)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    matrix->matrix.translate(DBL_TO_SCALAR(tx), DBL_TO_SCALAR(ty));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_scale(ps_matrix* matrix, double sx, double sy)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    matrix->matrix.scale(DBL_TO_SCALAR(sx), DBL_TO_SCALAR(sy));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_rotate(ps_matrix* matrix, double angle)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    matrix->matrix.rotate(DBL_TO_SCALAR(angle));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_shear(ps_matrix* matrix, double sx, double sy)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    matrix->matrix.shear(DBL_TO_SCALAR(sx), DBL_TO_SCALAR(sy));
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_invert(ps_matrix* matrix)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    matrix->matrix.invert();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_flip_x(ps_matrix* matrix)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    matrix->matrix.flip_x();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_flip_y(ps_matrix* matrix)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    matrix->matrix.flip_y();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_reset(ps_matrix* matrix)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    matrix->matrix.reset();
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_multiply(ps_matrix* result, const ps_matrix* a, const ps_matrix* b)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!result || !a || !b) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    result->matrix = a->matrix * b->matrix;
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_transform_point(const ps_matrix* matrix, ps_point* point)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix || !point) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }
    scalar x = DBL_TO_SCALAR(point->x);
    scalar y = DBL_TO_SCALAR(point->y);

    matrix->matrix.transform(&x, &y);

    point->x = SCALAR_TO_DBL(x);
    point->y = SCALAR_TO_DBL(y);
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_matrix_is_equal(const ps_matrix* a, const ps_matrix* b)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return False;
    }

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
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return False;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return False;
    }

    global_status = STATUS_SUCCEED;
    return matrix->matrix.is_identity() ? True : False;
}

double PICAPI ps_matrix_get_determinant(const ps_matrix* matrix)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return 0;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return 0;
    }

    global_status = STATUS_SUCCEED;
    return SCALAR_TO_DBL(matrix->matrix.determinant());
}

void PICAPI ps_matrix_set_translate_factor(ps_matrix* matrix, double tx, double ty)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    matrix->matrix.tx(DBL_TO_SCALAR(tx));
    matrix->matrix.ty(DBL_TO_SCALAR(ty));
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_matrix_get_translate_factor(ps_matrix* matrix, double *tx, double *ty)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return False;
    }

    if (!matrix || !tx || !ty) {
        global_status = STATUS_INVALID_ARGUMENT;
        return False;
    }

    *tx = SCALAR_TO_DBL(matrix->matrix.tx());
    *ty = SCALAR_TO_DBL(matrix->matrix.ty());
    global_status = STATUS_SUCCEED;
    return True;
}

void PICAPI ps_matrix_set_scale_factor(ps_matrix* matrix, double sx, double sy)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    matrix->matrix.sx(DBL_TO_SCALAR(sx));
    matrix->matrix.sy(DBL_TO_SCALAR(sy));
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_matrix_get_scale_factor(ps_matrix* matrix, double *sx, double *sy)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return False;
    }

    if (!matrix || !sx || !sy) {
        global_status = STATUS_INVALID_ARGUMENT;
        return False;
    }

    *sx = SCALAR_TO_DBL(matrix->matrix.sx());
    *sy = SCALAR_TO_DBL(matrix->matrix.sy());
    global_status = STATUS_SUCCEED;
    return True;
}

void PICAPI ps_matrix_set_shear_factor(ps_matrix* matrix, double shx, double shy)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    matrix->matrix.shx(DBL_TO_SCALAR(shx));
    matrix->matrix.shy(DBL_TO_SCALAR(shy));
    global_status = STATUS_SUCCEED;
}

ps_bool PICAPI ps_matrix_get_shear_factor(ps_matrix* matrix, double *shx, double *shy)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return False;
    }

    if (!matrix || !shx || !shy) {
        global_status = STATUS_INVALID_ARGUMENT;
        return False;
    }

    *shx = SCALAR_TO_DBL(matrix->matrix.shx());
    *shy = SCALAR_TO_DBL(matrix->matrix.shy());
    global_status = STATUS_SUCCEED;
    return True;
}

void PICAPI ps_matrix_transform_rect(const ps_matrix* matrix, ps_rect* rect)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

    if (!matrix || !rect) {
        global_status = STATUS_INVALID_ARGUMENT;
        return;
    }

    _matrix_transform_rect(matrix->matrix, rect);
    global_status = STATUS_SUCCEED;
}

void PICAPI ps_matrix_transform_path(const ps_matrix* matrix, ps_path* path)
{
    if (!picasso::is_valid_system_device()) {
        global_status = STATUS_DEVICE_ERROR;
        return;
    }

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

