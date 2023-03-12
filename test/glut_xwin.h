
#ifndef _GLUT_XWIN_H_
#define _GLUT_XWIN_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>

static NativeDisplayType CreateNativeDisplay(void)
{
    return XOpenDisplay(NULL);
}

static void DestroyNativeDisplay(NativeDisplayType dpy)
{
    XCloseDisplay(dpy);
}

static NativeWindowType CreateNativeWindow(EGLDisplay dpy, NativeDisplayType display, EGLConfig config, GLint width, GLint height)
{
    XVisualInfo *visInfo, visTemplate;
    int num_visuals;
    Window root, xwin;
    XSetWindowAttributes attr;
    unsigned long mask;
    EGLint vid;

    if (!eglGetConfigAttrib(dpy, config, EGL_NATIVE_VISUAL_ID, &vid))
        return 0;

    visTemplate.visualid = vid;
    visInfo = XGetVisualInfo(display, VisualIDMask, &visTemplate, &num_visuals);

    root = RootWindow(display, DefaultScreen(display));

    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(display, root, visInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    xwin = XCreateWindow(display, root, 0, 0, width, height,
            0, visInfo->depth, InputOutput, visInfo->visual, mask, &attr);

    XFree(visInfo);

    /* set hints and properties */
    {
        XSizeHints sizehints;
        sizehints.x = 0;
        sizehints.y = 0;
        sizehints.width  = width;
        sizehints.height = height;
        sizehints.flags = USSize | USPosition;
        XSetNormalHints(display, xwin, &sizehints);
        XSetStandardProperties(display, xwin,
                "picasso", "picasso", None, (char **) NULL, 0, &sizehints);
    }

    XMapWindow(display, xwin);
    return xwin;
}

static void DestroyNativeWindow(NativeDisplayType display, NativeWindowType window)
{
    XDestroyWindow(display, window);
}

static void DispatchNativeEvent(NativeDisplayType display, NativeWindowType window, void (*callback)(int, void*))
{
    XEvent event;
    if (!XPending(display)) {
        if (callback) {
            callback(msg_idle, 0);
            return;
        }
    }

   /* block for next event */
   XNextEvent(display, &event);

   switch (event.type) {
   case Expose:
       if (callback)
           callback(msg_draw, 0);
       break;
   case ConfigureNotify:
       {
           size s;
           s.width = event.xconfigure.width;
           s.height = event.xconfigure.height;
           if (callback)
               callback(msg_size, &s);
       }
       break;
   default:
       ;
   }
}
#endif
