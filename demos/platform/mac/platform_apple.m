//  Created by zhangjipeng on 2019/10/25.
//  Copyright © 2023 zhang  jipeng. All rights reserved.

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

#include "picasso.h"
#include "interface.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@end

@interface DrawView : NSView

- (void) drawRect:(NSRect)dirtyRect;

- (void) mouseDown:(NSEvent *)theEvent;
- (void) mouseUp:(NSEvent *)theEvent;
- (void) mouseDragged:(NSEvent *)theEvent;
- (void) keyDown:(NSEvent *)theEvent;
- (void) keyUp:(NSEvent *)theEvent;

//- (void) touchesBeganWithEvent:(NSEvent *) theEvent;
//- (void) touchesMovedWithEvent:(NSEvent *) theEvent;
//- (void) touchesEndedWithEvent:(NSEvent *) theEvent;
//- (void) touchesCancelledWithEvent:(NSEvent *) theEvent;

typedef enum {
    COCOA_TOUCH_DOWN,
    COCOA_TOUCH_UP,
    COCOA_TOUCH_MOVE,
    COCOA_TOUCH_CANCELLED
} cocoaTouchType;

- (void) handleTouches:(cocoaTouchType)type withEvent:(NSEvent *)event;

@end

@interface MainWindowDelegate : NSResponder <NSWindowDelegate>

-(void) windowDidResize:(NSNotification *) aNotification;


@property (weak) AppDelegate* app;
@end


@interface AppDelegate ()
@property (strong) NSWindow* mainWindow;
@property (strong) MainWindowDelegate* windowDelegate;
@property (strong) IBOutlet DrawView* drawView;
@property (strong) NSTimer* timer;
@end

static int32_t width = 640;
static int32_t height = 480;

static size_t pitch = 0;
static ps_color_format fmt;
static ps_context *context;
static ps_canvas *canvas;
static uint8_t *buffer;

static int get_virtual_key(int pk);

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    _windowDelegate = [[MainWindowDelegate alloc] init];
    [_windowDelegate setApp:self];
    
    NSRect rc = NSMakeRect(0, 0, width, height);
    NSWindowStyleMask style = NSWindowStyleMaskClosable | NSWindowStyleMaskTitled | NSWindowStyleMaskResizable;
    _mainWindow = [[NSWindow alloc] initWithContentRect:rc styleMask:style backing:NSBackingStoreBuffered defer:NO];
    
    [_mainWindow setTitle:@"Picasso Demos"];
    [_mainWindow setDelegate:_windowDelegate];
    [_mainWindow makeKeyAndOrderFront:_mainWindow];
    [_mainWindow makeKeyWindow];
    [_mainWindow setAcceptsMouseMovedEvents:YES];
    [_mainWindow setNextResponder:_drawView];
    [_mainWindow center];
    
    fmt = COLOR_FORMAT_RGBA;

    pitch = width * 4;
    buffer = malloc(height * pitch);
 
    canvas = ps_canvas_create_with_data(buffer, fmt, width, height, (int)pitch);
    context = ps_context_create(canvas, 0);
    on_init(context, width, height);
    
    _drawView = [[DrawView alloc] initWithFrame:rc];
    [_drawView setWantsLayer:YES];
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1060 && MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
    [_drawView setAcceptsTouchEvents:YES];
#elif MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
    _drawView.allowedTouchTypes = NSTouchTypeMaskDirect;
#endif
    [_drawView layer].backgroundColor = [[NSColor blackColor] CGColor];
    [self.mainWindow.contentView addSubview:_drawView];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    on_term(context);
    ps_context_unref(context);
    ps_canvas_unref(canvas);
    
    ps_shutdown();
    free(buffer);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)aNottification {
    return YES;
}

- (void) startTimer:(unsigned int)mc{
    _timer = [NSTimer scheduledTimerWithTimeInterval:mc/1000.0 target:self selector:@selector(timeFired) userInfo:nil repeats:YES];
}

- (void) stopTimer {
    [_timer invalidate];
}

- (void) timeFired {
    on_timer();
}

@end

@implementation DrawView

- (void) drawRect:(NSRect)dirtyRect {
    on_draw(context);
    NSDrawBitmap(dirtyRect, width, height, 8, 4, 32, pitch, NO, YES, NSDeviceRGBColorSpace, (const unsigned char *const  _Nullable*)&buffer);
}

- (BOOL) acceptsFirstResponder { return YES; }
- (BOOL) canBecomeKeyView { return YES; }

/*
- (void) touchesBeganWithEvent:(NSEvent *) theEvent {
    [self handleTouches:COCOA_TOUCH_DOWN withEvent:theEvent];
}

- (void) touchesMovedWithEvent:(NSEvent *) theEvent {
    [self handleTouches:COCOA_TOUCH_MOVE withEvent:theEvent];
}

- (void) touchesEndedWithEvent:(NSEvent *) theEvent {
    [self handleTouches:COCOA_TOUCH_UP withEvent:theEvent];
}

- (void) touchesCancelledWithEvent:(NSEvent *) theEvent {
    [self handleTouches:COCOA_TOUCH_CANCELLED withEvent:theEvent];
}
*/

- (void) handleTouches:(cocoaTouchType)type withEvent:(NSEvent *)event {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
    NSPoint point = [event locationInWindow];

    switch (type) {
        case COCOA_TOUCH_DOWN:
            on_mouse_event(LEFT_BUTTON_DOWN, EVT_LBUTTON, point.x, height - point.y);
            break;
        case COCOA_TOUCH_UP:
        case COCOA_TOUCH_CANCELLED:
            on_mouse_event(LEFT_BUTTON_UP, EVT_LBUTTON, point.x, height - point.y);
            break;
        case COCOA_TOUCH_MOVE:
            on_mouse_event(MOUSE_MOVE, EVT_LBUTTON, point.x, height - point.y);
            break;
    }
    
#endif /* MAC_OS_X_VERSION_MAX_ALLOWED >= 1060 */
}

- (void)mouseDown:(NSEvent *)theEvent {
    mouse_event_type type;
    unsigned int key;

    switch ([theEvent buttonNumber]) {
    case 0:
        type = LEFT_BUTTON_DOWN;
        key = EVT_LBUTTON;
        break;
    case 1:
        type = RIGHT_BUTTON_DOWN;
        key = EVT_RBUTTON;
        break;
    case 2:
        type = MIDDLE_BUTTON_DOWN;
        key = EVT_MBUTTON;
        break;
    }

    NSPoint point = [theEvent locationInWindow];
    on_mouse_event(type, key, point.x, height - point.y);
}

- (void)mouseUp:(NSEvent *)theEvent {
    mouse_event_type type;
    unsigned int key;

    switch ([theEvent buttonNumber]) {
    case 0:
        type = LEFT_BUTTON_UP;
        key = EVT_LBUTTON;
        break;
    case 1:
        type = RIGHT_BUTTON_UP;
        key = EVT_RBUTTON;
        break;
    case 2:
        type = MIDDLE_BUTTON_UP;
        key = EVT_MBUTTON;
        break;
    }

    NSPoint point = [theEvent locationInWindow];
    on_mouse_event(type, key, point.x, height - point.y);
}

- (void) mouseDragged:(NSEvent *)theEvent {
    unsigned int key;
    switch ([theEvent buttonNumber]) {
    case 0:
        key = EVT_LBUTTON;
        break;
    case 1:
        key = EVT_RBUTTON;
        break;
    case 2:
        key = EVT_MBUTTON;
        break;
    }
    
    NSPoint point = [theEvent locationInWindow];
    on_mouse_event(MOUSE_MOVE, key, point.x, height - point.y);
}

-(void) keyDown:(NSEvent *)theEvent {
    on_key_event(KEY_EVENT_DOWN, get_virtual_key([theEvent keyCode]));
}

-(void) keyUp:(NSEvent *)theEvent {
    on_key_event(KEY_EVENT_UP, get_virtual_key([theEvent keyCode]));
}

@end

@implementation MainWindowDelegate

-(void) windowDidResize:(NSNotification *) aNotification {
    ps_canvas* old_canvas = 0;
    NSRect rect = [[_app mainWindow] contentRectForFrameRect:[[_app mainWindow] frame]];
    width = rect.size.width;
    height = rect.size.height;
    [[_app drawView] setFrameSize:rect.size];

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    pitch = width * 4;
    buffer = realloc(buffer, height * pitch);
    canvas = ps_canvas_create_with_data(buffer, fmt, width, height, (int)pitch);
    old_canvas = ps_context_set_canvas(context, canvas);
    ps_canvas_unref(old_canvas);

    on_size(width, height);
}

@end


void refresh(const ps_rect* r)
{
    AppDelegate* app = [[NSApplication sharedApplication] delegate];
    if (r) {
        NSRect rc = NSMakeRect(r->x, r->y, r->w, r->h);
        [[app drawView] setNeedsDisplayInRect:rc];
    } else {
        [[app drawView] setNeedsDisplay:YES];
    }
}

unsigned set_timer(unsigned mc)
{
    AppDelegate* app = [[NSApplication sharedApplication] delegate];
    [app startTimer:mc];
    return 0;
}

void clear_timer(unsigned id)
{
    AppDelegate* app = [[NSApplication sharedApplication] delegate];
    [app stopTimer];
}

int main(int argc, const char * argv[])
{
    ps_initialize();
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        AppDelegate * delegate = [[AppDelegate alloc] init];
        NSApplication * application = [NSApplication sharedApplication];
        [application setDelegate:delegate];
    }
    return NSApplicationMain(argc, argv);
}

typedef struct {
    int vk;
    int pk;
}KeyEntities;

static KeyEntities key_map[] = {
    {KEY_BACK, kVK_Delete},
    {KEY_TAB, kVK_Tab},
    {KEY_RETURN, kVK_Return},
    {KEY_SHIFT, kVK_Shift},
    {KEY_CONTROL, kVK_Control},
    {KEY_CAPITAL, kVK_CapsLock},
    {KEY_ESCAPE, kVK_Escape},
    {KEY_SPACE, kVK_Space},
    {KEY_PRIOR, kVK_PageUp},
    {KEY_NEXT, kVK_PageDown},
    {KEY_END, kVK_End},
    {KEY_HOME, kVK_Home},
    {KEY_LEFT, kVK_LeftArrow},
    {KEY_UP, kVK_UpArrow},
    {KEY_RIGHT, kVK_RightArrow},
    {KEY_DOWN, kVK_DownArrow},
    {KEY_HELP, kVK_Help},
    {KEY_0, kVK_ANSI_0},
    {KEY_1, kVK_ANSI_1},
    {KEY_2, kVK_ANSI_2},
    {KEY_3, kVK_ANSI_3},
    {KEY_4, kVK_ANSI_4},
    {KEY_5, kVK_ANSI_5},
    {KEY_6, kVK_ANSI_6},
    {KEY_7, kVK_ANSI_7},
    {KEY_8, kVK_ANSI_8},
    {KEY_9, kVK_ANSI_9},
    {KEY_A, kVK_ANSI_A},
    {KEY_B, kVK_ANSI_B},
    {KEY_C, kVK_ANSI_C},
    {KEY_D, kVK_ANSI_D},
    {KEY_E, kVK_ANSI_E},
    {KEY_F, kVK_ANSI_F},
    {KEY_G, kVK_ANSI_G},
    {KEY_H, kVK_ANSI_H},
    {KEY_I, kVK_ANSI_I},
    {KEY_J, kVK_ANSI_J},
    {KEY_K, kVK_ANSI_K},
    {KEY_L, kVK_ANSI_L},
    {KEY_M, kVK_ANSI_M},
    {KEY_N, kVK_ANSI_N},
    {KEY_O, kVK_ANSI_O},
    {KEY_P, kVK_ANSI_P},
    {KEY_Q, kVK_ANSI_Q},
    {KEY_R, kVK_ANSI_R},
    {KEY_S, kVK_ANSI_S},
    {KEY_T, kVK_ANSI_T},
    {KEY_U, kVK_ANSI_U},
    {KEY_V, kVK_ANSI_V},
    {KEY_W, kVK_ANSI_W},
    {KEY_X, kVK_ANSI_X},
    {KEY_Y, kVK_ANSI_Y},
    {KEY_Z, kVK_ANSI_Z},
    {KEY_LWIN, kVK_Option},
    {KEY_RWIN, kVK_RightOption},
    {KEY_OEM_PLUS, kVK_ANSI_Equal},
    {KEY_OEM_MINUS, kVK_ANSI_Minus},
    {KEY_DIVIDE, kVK_ANSI_Slash},
    {KEY_F1, kVK_F1},
    {KEY_F2, kVK_F2},
    {KEY_F3, kVK_F3},
    {KEY_F4, kVK_F4},
    {KEY_F5, kVK_F5},
    {KEY_F6, kVK_F6},
    {KEY_F7, kVK_F7},
    {KEY_F8, kVK_F8},
    {KEY_F9, kVK_F9},
    {KEY_F10, kVK_F10},
    {KEY_F11, kVK_F11},
    {KEY_F12, kVK_F12},
    {KEY_F13, kVK_F13},
    {KEY_F14, kVK_F14},
    {KEY_F15, kVK_F15},
    {KEY_LSHIFT, kVK_Shift},
    {KEY_RSHIFT, kVK_RightShift},
    {KEY_LCONTROL, kVK_Control},
    {KEY_RCONTROL, kVK_RightControl},
};

static int get_virtual_key(int pk)
{
    int i;
    for(i = 0; i < (sizeof(key_map)/sizeof(KeyEntities)); i++)
        if (key_map[i].pk == pk)
            return key_map[i].vk;
    return KEY_UNKNOWN;
}
