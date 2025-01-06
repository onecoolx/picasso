#include "stdio.h"

#include "../include/picasso.h"
#include "drawFunc.h"

static ps_matrix* pm;
static ps_path * pa;
static ps_image * pi;
static ps_canvas * pc;
static ps_pattern * pt;
static ps_context* mc;
static ps_mask * m;
static float a = 1.0f;
int acase = 1;
void draw_test (int id, ps_context* gc)
{
    //ps_color col = {0, 0, 1, 1};
    ps_color sol = {1, 0, 0, 0.8f};
    ps_rect cr = {2.7f , 3.4f, 272.4f, 261.3f};

    ps_set_alpha(gc, 1.0f - a);
    ps_set_line_width(gc, 8);
    ps_rectangle(gc, &cr);

    ps_set_stroke_color(gc, &sol);
    ps_set_source_image(gc, pi);
//    ps_set_source_canvas(gc, pc);

    ps_fill(gc);

    ps_set_alpha(gc, a);

    ps_ellipse(gc, &cr);
    ps_set_source_pattern(gc, pt);
    //ps_set_source_color(gc, &col);
    ps_set_fill_rule(gc, FILL_RULE_EVEN_ODD);

    ps_paint(gc);
}

void init_context (ps_context* gc, ps_canvas* cs, unsigned char* buf)
{
    float version = (float)ps_version() / 10000;
    fprintf(stderr, "picasso version %.2f\n", version);

    pa = ps_path_create();
    pm = ps_matrix_create();
    m = ps_mask_create(480, 240);

    ps_canvas* ms = ps_canvas_create_from_mask(m, NULL);
    ps_context* mx = ps_context_create(ms, mc);

    ps_rect r = {0.3f, 2.7f, 320.45f, 240.19f};
    ps_color c = {1, 1, 1, 1};
    ps_ellipse(mx, &r);
    ps_set_source_color(mx, &c);
    ps_fill(mx);

    ps_context_unref(mx);
    ps_canvas_unref(ms);
    ps_canvas_set_mask(cs, m);
    mc = gc;
}

void dini_context (ps_context* gc)
{
    ps_image_unref(pi);
    ps_matrix_unref(pm);
    ps_pattern_unref(pt);
    ps_path_unref(pa);
    ps_canvas_unref(pc);
    ps_mask_unref(m);
}

void set_image_data(unsigned char* data, ps_color_format fmt, int w, int h, int p)
{
    ps_context* ptc = 0;
    ps_color c = {0, 0, 1, 1};
    ps_rect r = {0, 0, 100, 100};

    pi = ps_image_create_with_data(data, fmt, w, h, p);
    pc = ps_canvas_create(fmt, w, h);
    
    ptc = ps_context_create(pc, mc);
    ps_clear(ptc);
    ps_set_source_color(ptc, &c);
    ps_rectangle(ptc, &r);
    ps_fill(ptc);

    ps_context_unref(ptc);
}

void set_pattern_data(unsigned char* data, ps_color_format fmt, int w, int h, int p)
{
    ps_image* pam = ps_image_create_with_data(data, fmt, w, h, p);
    pt = ps_pattern_create_image(pam, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, pm);
    ps_image_unref(pam);
}

void timer_action(ps_context* gc)
{
    if (acase)
        a -= 0.1f;
    else 
        a += 0.1f;

    if (a < 0) acase = 0;
    else if (a > 1) acase = 1;
}

