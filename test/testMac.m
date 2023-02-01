//  Created by zhangjipeng on 2019/10/25.
//  Copyright © 2023 zhang  jipeng. All rights reserved.

#import <Cocoa/Cocoa.h>
#include "picasso.h"
#include "drawFunc.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@end

@interface DrawView : NSView

@end

@interface AppDelegate ()
@property (strong) NSWindow* mainWindow;
@property (strong) IBOutlet DrawView* drawView;
@property (strong) NSTimer* timer;
@property (strong) NSImage* img1;
@property (strong) NSImage* img2;
@end

static int32_t width = 640;
static int32_t height = 480;

static size_t pitch = 0;
static ps_context *context;
static ps_canvas *canvas;
static uint8_t *buffer;

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    NSRect rc = NSMakeRect(0, 0, width, height);
    NSWindowStyleMask style = NSWindowStyleMaskClosable | NSWindowStyleMaskTitled;
    _mainWindow = [[NSWindow alloc] initWithContentRect:rc styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [_mainWindow setTitle:@"Picasso test"];
    [_mainWindow makeKeyAndOrderFront:_mainWindow];
    [_mainWindow makeKeyWindow];
    [_mainWindow center];
    
    NSString* path = [[NSBundle mainBundle] resourcePath];
    NSString* path1 = [path stringByAppendingString:@"/selt2.png"];
    NSString* path2 = [path stringByAppendingString:@"/pat.png"];
    
    _img1 = [[NSImage alloc] initWithContentsOfFile:path1];
    _img2 = [[NSImage alloc] initWithContentsOfFile:path2];
    
    pitch = width * 4;
    buffer = malloc(height * pitch);

    canvas = ps_canvas_create_with_data(buffer, COLOR_FORMAT_RGBA, width, height, (int)pitch);
    context = ps_context_create(canvas, 0);
    init_context(context, canvas, buffer);
    
    NSBitmapImageRep *bmp1 = (NSBitmapImageRep *)[[_img1 representations] objectAtIndex:0];
    set_image_data([bmp1 bitmapData], COLOR_FORMAT_RGBA, [_img1 size].width, [_img1 size].height, (int)[bmp1 bytesPerRow]);
    NSBitmapImageRep *bmp2 = (NSBitmapImageRep *)[[_img2 representations] objectAtIndex:0];
    set_pattern_data([bmp2 bitmapData], COLOR_FORMAT_RGBA, [_img2 size].width, [_img2 size].height, (int)[bmp2 bytesPerRow]);
    
    _drawView = [[DrawView alloc] initWithFrame:rc];
    [_drawView setWantsLayer:YES];
    _drawView.layer.backgroundColor = [[NSColor blackColor] CGColor];
    [self.mainWindow.contentView addSubview:_drawView];
    _timer = [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(timeFired) userInfo:nil repeats:YES];
        
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    dini_context(context);
    ps_context_unref(context);
    ps_canvas_unref(canvas);
    ps_shutdown();
    
    free(buffer);
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)aNottification {
    return YES;
}

- (void) timeFired {
    timer_action(context);
    [_drawView setNeedsDisplay:YES];
}

@end

@implementation DrawView

- (void) drawRect:(NSRect)dirtyRect {
    memset(buffer, 0xFF, height * pitch); // clear buffer with color white
    draw_test(0, context);
    NSDrawBitmap(dirtyRect, width, height, 8, 4, 32, pitch, NO, YES, NSDeviceRGBColorSpace, (const unsigned char *const  _Nullable*)&buffer);
}

@end

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

int main(int argc, const char * argv[])
{
    __argc = (int)argc;
    __argv = (const char**)argv;

    ps_initialize();

    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        AppDelegate * delegate = [[AppDelegate alloc] init];
        NSApplication * application = [NSApplication sharedApplication];
        [application setDelegate:delegate];
    }
    return NSApplicationMain(argc, argv);
}

