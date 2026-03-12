/*
 * Copyright (c) 2025, Zhang Ji Peng
 * All rights reserved.
 *
 * SVG Animation Player demo.
 * Usage: svg_player <svg_file_name>
 *
 * Loads an SVG file containing animation elements and plays it
 * in a window using the psx_svg_player API.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "picasso.h"
#include "picasso_ext.h"
#include "psx_svg.h"
#include "psx_svg_player.h"
#include "drawFunc.h"
#include "timeuse.h"

#if defined(WIN32)
#include <direct.h>
#define getcwd _getcwd
#define DIRPREFIX "\\"
#else
#include <unistd.h>
#define DIRPREFIX "/"
#endif

static psx_svg* svg_doc = NULL;
static psx_svg_player* player = NULL;
static char full_path[1024];
static suseconds_t last_time = 0;

/* Frame interval target: ~16 fps */
#define FRAME_INTERVAL_MS 16

void draw_test(int id, ps_context* gc)
{
    ps_canvas* c;
    ps_size ss;
    ps_color bg = {1.0f, 1.0f, 1.0f, 1.0f};

    if (!player) {
        return;
    }

    c = ps_context_get_canvas(gc);
    ps_canvas_get_size(c, &ss);

    ps_save(gc);

    /* clear background */
    ps_set_source_color(gc, &bg);
    {
        ps_rect cr = {0, 0, ss.w, ss.h};
        ps_rectangle(gc, &cr);
    }
    ps_fill(gc);

    /* draw current animation frame */
    psx_svg_player_draw(player, gc);

    ps_restore(gc);
}

void init_context(ps_context* gc, ps_canvas* cs, unsigned char* buf)
{
    const char* path = NULL;
    float version = (float)ps_version() / 10000;
    fprintf(stderr, "picasso version %.2f\n", version);

    psx_svg_init();

    if (argc() >= 1) {
        path = argv()[1];
    }

    if (!path) {
        fprintf(stderr, "Usage: svg_player <svg_file_name>\n");
        return;
    }

    /* build full path */
    if (path[0] == '/' || path[0] == '\\' || (path[0] != '\0' && path[1] == ':')) {
        /* absolute path */
        strncpy(full_path, path, sizeof(full_path) - 1);
        full_path[sizeof(full_path) - 1] = '\0';
    } else {
        getcwd(full_path, sizeof(full_path));
        strncat(full_path, DIRPREFIX, sizeof(full_path) - strlen(full_path) - 1);
        strncat(full_path, path, sizeof(full_path) - strlen(full_path) - 1);
    }

    fprintf(stderr, "loading SVG: %s\n", full_path);

    {
        psx_result err;
        svg_doc = psx_svg_load(full_path, &err);
        if (!svg_doc) {
            fprintf(stderr, "failed to load SVG file: %s (error=%d)\n", full_path, (int)err);
            return;
        }
    }

    /* create player from the SVG DOM root */
    {
        psx_result err;
        player = psx_svg_player_create((const psx_svg_node*)svg_doc, &err);
        if (!player) {
            fprintf(stderr, "failed to create SVG player (error=%d)\n", (int)err);
            return;
        }
    }

    psx_svg_player_set_loop(player, true);
    psx_svg_player_play(player);

    last_time = get_time();

    (void)gc;
    (void)cs;
    (void)buf;
}

void dini_context(ps_context* gc)
{
    if (player) {
        psx_svg_player_destroy(player);
        player = NULL;
    }
    if (svg_doc) {
        psx_svg_destroy(svg_doc);
        svg_doc = NULL;
    }
    psx_svg_shutdown();
}

void set_image_data(unsigned char* data, ps_color_format fmt, int w, int h, int p)
{
    (void)data; (void)fmt; (void)w; (void)h; (void)p;
}

void set_pattern_data(unsigned char* data, ps_color_format fmt, int w, int h, int p)
{
    (void)data; (void)fmt; (void)w; (void)h; (void)p;
}

void timer_action(ps_context* gc)
{
    suseconds_t now;
    float delta;

    if (!player) {
        return;
    }

    now = get_time();
    delta = (float)(now - last_time) / 1000.0f;
    last_time = now;

    /* clamp delta to avoid jumps on stalls */
    if (delta > 0.5f) {
        delta = 0.5f;
    }
    if (delta < 0.0f) {
        delta = 0.0f;
    }

    psx_svg_player_tick(player, delta);
    (void)gc;
}
