

#ifndef _DRAW_FUNC_H
#define _DRAW_FUNC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void init_context (ps_context* gc, ps_canvas* cs);
void dini_context (ps_context* gc);
void draw_test (int id, ps_context* gc);
void set_image_data(unsigned char* data, ps_color_format fmt, int w, int h, int p);
void set_pattern_data(unsigned char* data, ps_color_format fmt, int w, int h, int p);
void timer_action(ps_context* gc);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
