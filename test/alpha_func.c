#include "stdio.h"

#include "../include/picasso.h"
#include "drawFunc.h"

static ps_rect gr = {200, 100, 320, 240};
static ps_rect gt = {100, 100, 200, 200};
static ps_point gp = {0 , 0};
static ps_matrix* pm;
static ps_path * pa;
static ps_image * pi;
static ps_pattern * pt;
static double a = 1.0;
int acase = 1;
void draw_test (int id, ps_context* gc)
{
	ps_color col = {0, 0, 1, 1};
	ps_color sol = {1, 0, 0, 0.8};
    ps_rect cr = {2.7 , 3.4, 272.4, 261.3};

	ps_set_alpha(gc, 1.0 - a);
	ps_set_line_width(gc, 8);
	ps_rectangle(gc, &cr);

	ps_set_stroke_color(gc, &sol);
	ps_set_source_image(gc, pi);

	ps_fill(gc);

	ps_set_alpha(gc, a);

	ps_ellipse(gc, &cr);
	ps_set_source_pattern(gc, pt);
	//ps_set_source_color(gc, &col);
	ps_set_fill_rule(gc, FILL_RULE_EVEN_ODD);


	ps_paint(gc);
}

void init_context (ps_context* gc, ps_canvas* cs)
{
	float version = (float)ps_version() / 10000;
	fprintf(stderr, "picasso version %.2f\n", version);

    pa = ps_path_create();
    pm = ps_matrix_create();

    ps_translate(gc, 203, 105);
    ps_scale(gc, 0.5, 0.5);
    ps_rotate(gc, 0.35);
	ps_set_filter(gc, FILTER_NEAREST);
}

void dini_context (ps_context* gc)
{
	ps_image_unref(pi);
    ps_matrix_unref(pm);
	ps_pattern_unref(pt);
    ps_path_unref(pa);
}

void set_image_data(unsigned char* data, ps_color_format fmt, int w, int h, int p)
{
	pi = ps_image_create_with_data(data, fmt, w, h, p);
}

void set_pattern_data(unsigned char* data, ps_color_format fmt, int w, int h, int p)
{
	ps_image* pam = ps_image_create_with_data(data, fmt, w, h, p);
	pt = ps_pattern_create_image(pam, WRAP_TYPE_REPEAT, WRAP_TYPE_REPEAT, pm);
}

void timer_action(ps_context* gc)
{
	if (acase)
		a -= 0.1;
	else 
		a += 0.1;

	if (a < 0) acase = 0;
	else if (a > 1) acase = 1;
}

