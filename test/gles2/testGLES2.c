
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "picasso.h"
#include "picasso_gpu.h"
#include "drawFunc.h"
#include "timeuse.h"
#include "glut.h"

typedef struct {
    GLint width;
    GLint height;

    EGLNativeWindowType window;
    EGLNativeDisplayType display; 
    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;
} GLES2_CONTEXT;

static ps_context *pcontext;
static ps_canvas *pcanvas;

static int initEGL(GLES2_CONTEXT* ctx)
{
    NativeWindowType window;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    EGLConfig configs[2];
    EGLint majorVer, minorVer;
    EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

    EGLint numConfigs;
    EGLint cfg_attribs[] = { EGL_BUFFER_SIZE,  EGL_DONT_CARE,
                             EGL_DEPTH_SIZE,   32,
                             EGL_RED_SIZE,     8,
                             EGL_GREEN_SIZE,   8,
                             EGL_BLUE_SIZE,    8,
                             EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                             EGL_NONE };

    EGLNativeDisplayType native_dpy = (EGLNativeDisplayType)CreateNativeDisplay();
    display = eglGetDisplay(native_dpy); 
    if (display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Init display fail!\n");
        return EGL_FALSE;
    }

    if (eglInitialize(display, &majorVer, &minorVer) != EGL_TRUE) {
        fprintf(stderr, "Init EGL fail!\n");
        return EGL_FALSE;
    }

    if (eglGetConfigs(display, configs, 2, &numConfigs) != EGL_TRUE) {
        fprintf(stderr, "Get config fail!\n");
        return EGL_FALSE;
    }

    if (eglChooseConfig(display, cfg_attribs, configs, 2, &numConfigs) != EGL_TRUE || !numConfigs) {
        fprintf(stderr, "Choose config fail!\n");
        return EGL_FALSE;
    }

    window = CreateNativeWindow(display, native_dpy, configs[0], ctx->width, ctx->height);
    if (!window) {
        fprintf(stderr, "Create Native Window fail!\n");
        return EGL_FALSE;
    }

    surface = eglCreateWindowSurface(display, configs[0], window, NULL);
    if (surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Create surface fail! 0x%X\n", eglGetError());
        return EGL_FALSE;
    }

    if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
        fprintf(stderr, "Bind API fail!\n");
        return EGL_FALSE;
    }

    context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Create context fail!\n");
        return EGL_FALSE;
    }

    if (eglMakeCurrent(display, surface, surface, context) != EGL_TRUE) {
        fprintf(stderr, "Make current fail!\n");
        return EGL_FALSE;
    }

    eglSwapInterval(display, 0);

    ctx->eglDisplay = display;
    ctx->eglSurface = surface;
    ctx->eglContext = context;
    ctx->window = window;
    ctx->display = native_dpy;

    pcanvas = ps_canvas_create_for_gpu_surface(surface, context);
    pcontext = ps_context_create(pcanvas, 0);

    return GL_TRUE;
}

static void drawFrame(GLES2_CONTEXT* ctx)
{
    suseconds_t t1, t2;
    t1 = get_time();
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    draw_test(0, pcontext);
    t2 = get_time();
    fprintf(stderr, "draw frame use %.4f ms --- %.4f fps\n", (double)(t2-t1), 1000.0/((t2-t1) ? (t2-t1) : 1.0));
}

static void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
}

static void onMessage(int message, void* data)
{
    if (message == msg_idle) {

    } else if (message == msg_size) {
        size* s = (size*)data;
        reshape(s->width, s->height);
    }
}

static int __argc = 0;
static const char** __argv = NULL;

int argc(void)
{
    return __argc;
}

const char** argv(void)
{
    return __argv;
}

const char* getSysFontName(void) {
    return "sans-serif";
}

int main(int argc, char*argv[])
{
    GLES2_CONTEXT ctx;
    memset(&ctx, 0, sizeof(GLES2_CONTEXT));

    __argc = argc;
    __argv = (const char**)argv;

    ctx.width = 640;
    ctx.height = 480;

    ps_initialize();

    if (!initEGL(&ctx)) {
        fprintf(stderr, "Init EGL fail!\n");
        return GL_FALSE;
    }

    while (1) {
        DispatchNativeEvent(ctx.display, ctx.window, onMessage);

        drawFrame(&ctx);
        eglSwapBuffers(ctx.eglDisplay, ctx.eglSurface);
    }

    ps_context_unref(pcontext);
    ps_canvas_unref(pcanvas);

    DestroyNativeWindow(ctx.display, ctx.window);
    DestroyNativeDisplay(ctx.display);

    ps_shutdown();
    return GL_TRUE;
}
