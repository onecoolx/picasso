//  Created by zhangjipeng on 2019/10/25.
//  Copyright © 2019 zhang  jipeng. All rights reserved.

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@end

@interface AppDelegate ()
@property NSWindow* mainWindow;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    NSRect rc = NSMakeRect(0, 0, 640, 480);
    NSWindowStyleMask style = NSWindowStyleMaskClosable | NSWindowStyleMaskTitled;
    _mainWindow = [[NSWindow alloc] initWithContentRect:rc styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [_mainWindow setTitle:@"Picasso test"];
    [_mainWindow makeKeyAndOrderFront:_mainWindow];
    [_mainWindow makeKeyWindow];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
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

    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        AppDelegate * delegate = [[AppDelegate alloc] init];
        NSApplication * application = [NSApplication sharedApplication];
        [application setDelegate:delegate];
    }
    return NSApplicationMain(argc, argv);
}

