#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "../include/picasso.h"
#include "drawFunc.h"

static ps_matrix* pm;
static ps_path * pa;
static ps_image * pi;
static ps_pattern * pt;
static ps_canvas* cv;

void draw_test (int id, ps_context* gc)
{
}

void init_context (ps_context* gc, ps_canvas* cs)
{
	float version = (float)ps_version() / 10000;
	fprintf(stderr, "picasso version %.2f\n", version);

    pa = ps_path_create();
    pm = ps_matrix_create();

	cv = cs;

}

void dini_context (ps_context* gc)
{
	stop_worker_threads();
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


	//start thread work!
	start_worker_threads(cv);	
}

void timer_action(ps_context* gc)
{
}


void thread_func1(void* data)
{
	ps_canvas* canvas = (ps_canvas*)data;
	ps_context * context = ps_context_create(canvas);

    ps_rect cr = {2.7 , 3.4, 272.4, 261.3};

	while(1) {
		ps_rectangle(context, &cr);
		ps_set_source_image(context, pi);
		ps_fill(context);
		ps_text_out_length(context, 10, 10, "first thread draw!", 18);
		loop_wait(10);
	}
}

void thread_func2(void* data)
{
	ps_canvas* canvas = (ps_canvas*)data;
	ps_context * context = ps_context_create(canvas);


    ps_rect cr = {102.7 , 123.4, 272.4, 261.3};

	while(1) {
		ps_ellipse(context, &cr);
		ps_set_source_pattern(context, pt);
		ps_set_fill_rule(context, FILL_RULE_EVEN_ODD);
		ps_fill(context);
		ps_text_out_length(context, 10, 200, "second thread draw!", 19);
		loop_wait(10);
	}
}

