/* flowers - application interface
 * 
 * Copyright (C) 2009 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

/*
 * This idea is come from AmanithVG.
 */

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#include "picasso.h"
#include "interface.h"

#define PI 3.14159265358979323846

static int width;
static int height;
static unsigned tid;
static double scale;
static double zoomFactor = 1;
static unsigned millsecons = 0;

static ps_path* shadowPath;
static ps_path* quadrantPath;
static ps_path* innerBevelPath;
static ps_path* outerBevelPath;
static ps_path* secondTagPath;
static ps_path* hourTagPath;
static ps_path* msTagPath;
static ps_path* glassPath;
static ps_path* secondCursorPath;
static ps_path* hourCursorPath;
static ps_path* minuteCursorPath;
static ps_path* msCursorPath;
static ps_path* cursorScrewPath;

static ps_gradient* shadowGradient;
static ps_gradient* quadrantGradient;
static ps_gradient* bevelsGradient;

static ps_matrix* adjust;

static void create_gradient()
{
	//shadow gradient
	{
		ps_point ssp = {260, 250}; 
		ps_point sep = {260, 250}; 
		ps_color css[4] = {{0.3, 0.3, 0.3, 1.0}, {0.3, 0.3, 0.3, 1.0},
			{0.3, 0.3, 0.3, 0.0}, {0.3, 0.3, 0.3, 0.0}}; 
		shadowGradient = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &ssp, 0, &sep, 270);
		ps_gradient_add_color_stop(shadowGradient, 0.00, &css[0]);
		ps_gradient_add_color_stop(shadowGradient, 0.80, &css[1]);
		ps_gradient_add_color_stop(shadowGradient, 0.95, &css[2]);
		ps_gradient_add_color_stop(shadowGradient, 1.00, &css[3]);
	}
	// quadrant background gradient
	{
		ps_point qsp = {236, 276}; 
		ps_point qep = {235, 275}; 
		ps_color cqs[3] = {{0.89, 0.90, 0.91, 1.00}, {0.76, 0.77, 0.78, 1.00}, {0.36, 0.36, 0.40, 1.00}}; 
		quadrantGradient = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &qsp, 0, &qep, 256);
		ps_gradient_add_color_stop(quadrantGradient, 0.00, &cqs[0]);
		ps_gradient_add_color_stop(quadrantGradient, 0.55, &cqs[1]);
		ps_gradient_add_color_stop(quadrantGradient, 1.00, &cqs[2]);
	}
	// outer/inner bevel ring gradient
	{
		ps_point osp = {80, 120}; 
		ps_point oep = {200, 256}; 
		ps_color cos[4] = {{1.00, 1.00, 1.00, 1.00}, {0.90, 0.90, 0.92, 1.00}, 
			{0.46, 0.47, 0.50, 1.00}, {0.16, 0.17, 0.20, 1.00}}; 
		bevelsGradient = ps_gradient_create_radial(GRADIENT_SPREAD_PAD, &osp, 0, &oep, 300);
		ps_gradient_add_color_stop(bevelsGradient, 0.00, &cos[0]);
		ps_gradient_add_color_stop(bevelsGradient, 0.40, &cos[1]);
		ps_gradient_add_color_stop(bevelsGradient, 0.80, &cos[2]);
		ps_gradient_add_color_stop(bevelsGradient, 1.00, &cos[3]);
	}
}

static void create_path()
{
	// shadow path
	{
		ps_point spp[3] = {{0.0, 242.0}, {540.0, 242.0}, {0.0, 242.0}};
		shadowPath = ps_path_create();
		ps_path_move_to(shadowPath, &spp[0]);
		ps_path_arc_to(shadowPath, 270.0, 270.0, 0.0, True, True, &spp[1]);
		ps_path_arc_to(shadowPath, 270.0, 270.0, 0.0, True, True, &spp[2]);
		ps_path_sub_close(shadowPath);
	}
	// quadrant path
	{
		ps_point qpp[3] = {{16.0, 256.0}, {496.0, 256.0}, {16.0, 256.0}};
		quadrantPath = ps_path_create();
		ps_path_move_to(quadrantPath, &qpp[0]);
		ps_path_arc_to(quadrantPath, 240.0, 240.0, 0.0, True, True, &qpp[1]);
		ps_path_arc_to(quadrantPath, 240.0, 240.0, 0.0, True, True, &qpp[2]);
		ps_path_sub_close(quadrantPath);
	}
	// outer/inner bevel ring gradient
	{
		ps_point ipp[3] = {{53.0, 256.0}, {459.0, 256.0}, {53.0, 256.0}};
		ps_point ipp2[3] = {{43.0, 256.0}, {469.0, 256.0}, {43.0, 256.0}};
		innerBevelPath = ps_path_create();
		ps_path_move_to(innerBevelPath, &ipp[0]);
		ps_path_arc_to(innerBevelPath, 203.0, 203.0, 0.0, True, True, &ipp[1]);
		ps_path_arc_to(innerBevelPath, 203.0, 203.0, 0.0, True, True, &ipp[2]);
		ps_path_sub_close(innerBevelPath);
		ps_path_move_to(innerBevelPath, &ipp2[0]);
		ps_path_arc_to(innerBevelPath, 213.0, 213.0, 0.0, True, True, &ipp2[1]);
		ps_path_arc_to(innerBevelPath, 213.0, 213.0, 0.0, True, True, &ipp2[2]);
		ps_path_sub_close(innerBevelPath);
	}
	{
		ps_point opp[3] = {{44.0, 256.0}, {468.0, 256.0}, {44.0, 256.0}};
		ps_point opp2[3] = {{8.5, 256.0}, {503.5, 256.0}, {8.5, 256.0}};
		outerBevelPath = ps_path_create();
		ps_path_move_to(outerBevelPath, &opp[0]);
		ps_path_arc_to(outerBevelPath, 212.0, 212.0, 0.0, True, True, &opp[1]);
		ps_path_arc_to(outerBevelPath, 212.0, 212.0, 0.0, True, True, &opp[2]);
		ps_path_sub_close(outerBevelPath);
		ps_path_move_to(outerBevelPath, &opp2[0]);
		ps_path_arc_to(outerBevelPath, 247.5, 247.5, 0.0, True, True, &opp2[1]);
		ps_path_arc_to(outerBevelPath, 247.5, 247.5, 0.0, True, True, &opp2[2]);
		ps_path_sub_close(outerBevelPath);
	}
	// seconds tag
	{
		ps_point stp[2] = {{256, 442}, {256, 446}};
		secondTagPath = ps_path_create();
		ps_path_move_to(secondTagPath, &stp[0]);
		ps_path_line_to(secondTagPath, &stp[1]);
	}
	// hours tag
	{
		ps_point htp[2] = {{256, 430}, {256, 446}};
		hourTagPath = ps_path_create();
		ps_path_move_to(hourTagPath, &htp[0]);
		ps_path_line_to(hourTagPath, &htp[1]);
	}
	// 1000 / 12 milliseconds tag
	{
		ps_point mtp[2] = {{356, 274}, {356, 276}};
		msTagPath = ps_path_create();
		ps_path_move_to(msTagPath, &mtp[0]);
		ps_path_line_to(msTagPath, &mtp[1]);
	}
	// glass
	{
		ps_point gtp[12] = {{162, 447}, {365, 540}, {564, 309}, {419, 118}
			,{400, 350}, {162, 447}, {256,  43}, {83 , 45 }
			,{4  , 231}, {62 , 340}, {80 , 110}, {256, 43 }};
			glassPath = ps_path_create();
			ps_path_move_to(glassPath, &gtp[0]);
			ps_path_bezier_to(glassPath, &gtp[1], &gtp[2], &gtp[3]);
			ps_path_quad_to(glassPath, &gtp[4], &gtp[5]);
			ps_path_move_to(glassPath, &gtp[6]);
			ps_path_bezier_to(glassPath, &gtp[7], &gtp[8], &gtp[9]);
			ps_path_quad_to(glassPath, &gtp[10], &gtp[11]);
	}

	// milliseconds	cursor
	{
		ps_point mpt[11] = {{355.5, 250.0}, {355.5, 253.5}, {355.5, 258.5}, {356.0, 270.0}
			,{356.5, 258.5}, {356.5, 253.5}, {356.5, 250.0}, {355.5, 250.0}
			,{356.0, 254.5}, {356.0, 257.5}, {356.0, 254.5}};
			msCursorPath = ps_path_create();
			ps_path_move_to(msCursorPath, &mpt[0]);
			ps_path_line_to(msCursorPath, &mpt[1]);
			ps_path_arc_to(msCursorPath, 2.0, 2.5, 0.0, True, True, &mpt[2]);
			ps_path_line_to(msCursorPath, &mpt[3]);
			ps_path_line_to(msCursorPath, &mpt[4]);
			ps_path_arc_to(msCursorPath, 2.0, 2.5, 0.0, True, True, &mpt[5]);
			ps_path_line_to(msCursorPath, &mpt[6]);
			ps_path_line_to(msCursorPath, &mpt[7]);
			ps_path_move_to(msCursorPath, &mpt[8]);
			ps_path_arc_to(msCursorPath, 1.5, 1.5, 0.0, True, True, &mpt[9]);
			ps_path_arc_to(msCursorPath, 1.5, 1.5, 0.0, True, True, &mpt[10]);
	}
	// seconds
	{	
		ps_point spt[17] = {{254.0, 210.0}, {254.0, 247.0}, {254.5, 265.0}, {254.5, 394.5}
			,{255.0, 407.5}, {256.0, 444.0}, {257.0, 407.5}, {257.5, 394.5}
			,{257.5, 265.0}, {258.0, 247.0}, {258.0, 210.0}, {256.0, 249.5}
			,{256.0, 262.5}, {256.0, 249.5}, {256.0, 396.25}, {256.0, 405.75}
			,{256.0, 396.25}};
			secondCursorPath = ps_path_create();
			ps_path_move_to(secondCursorPath, &spt[0]);
			ps_path_line_to(secondCursorPath, &spt[1]);
			ps_path_arc_to(secondCursorPath, 7.75, 9.0, 0.0, True, True, &spt[2]);
			ps_path_line_to(secondCursorPath, &spt[3]);
			ps_path_arc_to(secondCursorPath, 5.0, 6.0, 0.0, True, True, &spt[4]);
			ps_path_line_to(secondCursorPath, &spt[5]);
			ps_path_line_to(secondCursorPath, &spt[6]);
			ps_path_arc_to(secondCursorPath, 5.0, 6.0, 0.0, True, True, &spt[7]);
			ps_path_line_to(secondCursorPath, &spt[8]);
			ps_path_arc_to(secondCursorPath, 7.75, 9.0, 0.0, True, True, &spt[9]);
			ps_path_line_to(secondCursorPath, &spt[10]);
			ps_path_sub_close(secondCursorPath);
			ps_path_move_to(secondCursorPath, &spt[11]);
			ps_path_arc_to(secondCursorPath, 7.0, 6.5, 0.0, True, True, &spt[12]);
			ps_path_arc_to(secondCursorPath, 7.0, 6.5, 0.0, True, True, &spt[13]);
			ps_path_move_to(secondCursorPath, &spt[14]);
			ps_path_arc_to(secondCursorPath, 4.75, 4.75, 0.0, True, True, &spt[15]);
			ps_path_arc_to(secondCursorPath, 4.75, 4.75, 0.0, True, True, &spt[16]);
	}

	// hour cursor
	{
		ps_point hpt[4] = {{251.0, 228.0}, {252.0, 370.0}, {260.0, 370.0}, {261.0, 228.0}};
		hourCursorPath = ps_path_create();
		ps_path_move_to(hourCursorPath, &hpt[0]);
		ps_path_line_to(hourCursorPath, &hpt[1]);
		ps_path_line_to(hourCursorPath, &hpt[2]);
		ps_path_line_to(hourCursorPath, &hpt[3]);
		ps_path_sub_close(hourCursorPath);
	}

	// minute cursor
	{
		ps_point cpt[4] = {{253.0, 218.0}, {254.0, 410.0}, {258.0, 410.0}, {259.0, 218.0}};
		minuteCursorPath = ps_path_create();
		ps_path_move_to(minuteCursorPath, &cpt[0]);
		ps_path_line_to(minuteCursorPath, &cpt[1]);
		ps_path_line_to(minuteCursorPath, &cpt[2]);
		ps_path_line_to(minuteCursorPath, &cpt[3]);
		ps_path_sub_close(minuteCursorPath);
	}
	// cursors screw
	{
		ps_point xpt[3] = {{249.0, 256.0}, {263.0, 256.0}, {249.0, 256.0}};
		cursorScrewPath = ps_path_create();
		ps_path_move_to(cursorScrewPath, &xpt[0]);
		ps_path_arc_to(cursorScrewPath, 7.0, 7.0, 0.0, True, True, &xpt[1]);
		ps_path_arc_to(cursorScrewPath, 7.0, 7.0, 0.0, True, True, &xpt[2]);
		ps_path_sub_close(cursorScrewPath);
	}
}

void on_init(ps_context* gc, int w, int h)
{
	width = w;
	height = h;

	scale = height / 512.0;

	create_gradient();
	create_path();
	adjust = ps_matrix_create();
	ps_matrix_translate(adjust, -256, -256);
	ps_matrix_flip_y(adjust);
	ps_matrix_translate(adjust, 256, 256);

	tid = set_timer(100);
}

static void drawCursors(ps_context*gc, unsigned hours, unsigned minutes, unsigned seconds, unsigned milliseconds)
{
	double hh, mm, ss, ks;
	ps_color col;

	if (hours < 13)
		hh = (((double)hours / 12.0 + (double)minutes / 720.0) * PI * 2) - PI;
	else
		hh = (((double)(hours - 12.0) / 12.0 + (double)minutes / 720.0) * PI * 2) - PI;

	mm = (((double)minutes / 60.0 + (double)seconds / 3600.0) * 2 * PI) - PI;
	ss = (((double)seconds / 60.0) * 2 * PI) - PI;
	ks = (((double)milliseconds / 1000.0) * 2 * PI) - PI;

	// milliseconds	
	col.r = 0.0; col.g = 0.2; col.b = 0.6; col.a = 1.0;
	ps_identity(gc);
	ps_set_source_color(gc, &col);
	ps_translate(gc, -356, -256);
	ps_rotate(gc, ks);
	ps_translate(gc, 356, 256);
	ps_scale(gc, scale, scale);
	ps_set_path(gc, msCursorPath);
	ps_fill(gc);

	// seconds shadow
	col.r = 0.3; col.g = 0.3; col.b = 0.3; col.a = 0.25;
	ps_identity(gc);
	ps_set_source_color(gc, &col);
	ps_translate(gc, -255, -257);
	ps_rotate(gc, ss);
	ps_translate(gc, 255, 257);
	ps_scale(gc, scale, scale);
	ps_set_path(gc, secondCursorPath);
	ps_fill(gc);

	// minutes shadow
	ps_identity(gc);
	ps_translate(gc, -255.5, -256.5);
	ps_scale(gc, 1.25, 1.01);
	ps_rotate(gc, mm);
	ps_translate(gc, 255.5, 256.5);
	ps_scale(gc, scale, scale);
	ps_set_path(gc, minuteCursorPath);
	ps_fill(gc);

	// hours shadow
	ps_identity(gc);
	ps_translate(gc, -255.5, -256.5);
	ps_scale(gc, 1.25, 1.01);
	ps_rotate(gc, hh);
	ps_translate(gc, 255.5, 256.5);
	ps_scale(gc, scale, scale);
	ps_set_path(gc, hourCursorPath);
	ps_fill(gc);

	// hours
	col.r = 0.2; col.g = 0.2; col.b = 0.22; col.a = 1.0;
	ps_identity(gc);
	ps_set_source_color(gc, &col);
	ps_translate(gc, -256, -256);
	ps_rotate(gc, hh);
	ps_translate(gc, 256, 256);
	ps_scale(gc, scale, scale);
	ps_set_path(gc, hourCursorPath);
	ps_fill(gc);

	// draw screw
	ps_identity(gc);
	ps_scale(gc, scale, scale);
	ps_set_path(gc, cursorScrewPath);
	ps_fill(gc);

	// minutes
	ps_identity(gc);
	ps_translate(gc, -256, -256);
	ps_rotate(gc, mm);
	ps_translate(gc, 256, 256);
	ps_scale(gc, scale, scale);
	ps_set_path(gc, minuteCursorPath);
	ps_fill(gc);

	// seconds
	col.r = 0.65; col.g = 0.1; col.b = 0.075; col.a = 1.0;
	ps_identity(gc);
	ps_set_source_color(gc, &col);
	ps_translate(gc, -256, -256);
	ps_rotate(gc, ss);
	ps_translate(gc, 256, 256);
	ps_scale(gc, scale, scale);
	ps_set_path(gc, secondCursorPath);
	ps_fill(gc);

}

void on_draw(ps_context* gc)
{
	int i;
	ps_color color = {1, 1, 1, 1};
	ps_set_source_color(gc, &color);
	ps_clear(gc);

	ps_set_line_cap(gc, LINE_CAP_BUTT);
	ps_set_composite_operator(gc, COMPOSITE_SRC_OVER);

	ps_identity(gc);
	ps_set_matrix(gc, adjust);
	ps_scale(gc, scale, scale);

	// draw background
	ps_set_source_gradient(gc, shadowGradient);
	ps_set_path(gc, shadowPath);
	ps_fill(gc);

	// draw quadrant
	ps_set_source_gradient(gc, quadrantGradient);
	ps_set_path(gc, quadrantPath);
	ps_fill(gc);

	ps_identity(gc);
	ps_scale(gc, scale, scale);
	// draw inner bevel
	ps_set_source_gradient(gc, bevelsGradient);
	ps_set_fill_rule(gc, FILL_RULE_EVEN_ODD);
	ps_set_path(gc, innerBevelPath);
	ps_fill(gc);

	ps_identity(gc);
	ps_translate(gc, -256, -256);
	ps_rotate(gc, PI);
	ps_translate(gc, 256, 256);
	ps_scale(gc, scale, scale);

	ps_set_source_gradient(gc, bevelsGradient);
	ps_set_fill_rule(gc, FILL_RULE_EVEN_ODD);
	ps_set_path(gc, outerBevelPath);
	ps_fill(gc);

	// draw seconds tags
	{
		ps_color sc = {0.11, 0.12, 0.13, 0.65};
		ps_set_line_width(gc, 2.0);
		ps_set_line_cap(gc, LINE_CAP_ROUND);
		ps_set_stroke_color(gc, &sc);
		for (i = 0; i < 60; i++) {
			if ((i % 5) != 0) {
				ps_identity(gc);
				ps_translate(gc, -256, -256);
				ps_rotate(gc, 2 * PI - ((double)i/60.0) * PI * 2);
				ps_translate(gc, 256, 256);
				ps_scale(gc, scale, scale);
				ps_set_path(gc, secondTagPath);
				ps_stroke(gc);
			}
		}

		// draw hours and milliseconds tags
		for (i = 0; i < 12; i++) {
			double rot = 2 * PI - (((double)i / 12.0f) * PI * 2);
			ps_set_line_width(gc, 7.0);
			ps_identity(gc);
			ps_translate(gc, -256, -256);
			ps_rotate(gc, rot);
			ps_translate(gc, 256, 256);
			ps_scale(gc, scale, scale);
			ps_set_path(gc, hourTagPath);
			ps_stroke(gc);

			ps_set_line_width(gc, 2.0);
			ps_identity(gc);
			ps_translate(gc, -356, -256);
			ps_rotate(gc, rot);
			ps_translate(gc, 356, 256);
			ps_scale(gc, scale, scale);
			ps_set_path(gc, msTagPath);
			ps_stroke(gc);
		}
	}
	//draw cursors
	{
		time_t ctime;
		struct tm *ltime;
		ctime = time(NULL);
		ltime = localtime(&ctime);
		drawCursors(gc, ltime->tm_hour, ltime->tm_min, ltime->tm_sec, millsecons);
	}
	//draw glass
	{
		ps_color gcol = {0.04, 0.045, 0.05, 0.8};
		ps_identity(gc);
		ps_set_matrix(gc, adjust);
		ps_scale(gc, scale, scale);
		ps_set_composite_operator(gc, COMPOSITE_PLUS);
		ps_set_source_color(gc, &gcol);
		ps_set_path(gc, glassPath);
		ps_fill(gc);
	}
}

void on_term(ps_context* gc)
{
	clear_timer(tid);
	ps_matrix_unref(adjust);

	ps_gradient_unref(bevelsGradient);
	ps_gradient_unref(quadrantGradient);
	ps_gradient_unref(shadowGradient);

	ps_path_unref(shadowPath);
	ps_path_unref(quadrantPath);
	ps_path_unref(innerBevelPath);
	ps_path_unref(outerBevelPath);
	ps_path_unref(secondTagPath);
	ps_path_unref(hourTagPath);
	ps_path_unref(msTagPath);
	ps_path_unref(glassPath);
	ps_path_unref(secondCursorPath);
	ps_path_unref(hourCursorPath);
	ps_path_unref(minuteCursorPath);
	ps_path_unref(msCursorPath);
	ps_path_unref(cursorScrewPath);
}

void on_key_event(key_event_type kvt, int vk)
{
}

void on_mouse_event(mouse_event_type evt, unsigned key, int x, int y)
{
}

void on_timer()
{
	millsecons += 100; 
	refresh(NULL);
}

void on_size(int w, int h)
{
	width = w;
	height = h;
	scale = height / 512.0;
}
