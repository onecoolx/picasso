
#ifndef _GLUT_H_
#define _GLUT_H_

#include <EGL/egl.h>

enum {
    msg_idle,
    msg_draw,
    msg_size,
    msg_input,
};

typedef struct {
    int width;
    int height;
} size;

#include "glut_xwin.h"

#endif
