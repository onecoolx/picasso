//  Created by zhangjipeng on 2019/10/25.
//  Copyright © 2019 zhang  jipeng. All rights reserved.

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>


@end

@interface AppDelegate ()
@property (nonatomic,strong) NSWindow* mainWindow;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    NSRect rc = NSMakeRect(0, 0, 640, 480);
    NSWindowStyleMask style = NSWindowStyleMaskClosable | NSWindowStyleMaskTitled | NSWindowStyleMaskResizable;
    _mainWindow = [[NSWindow alloc] initWithContentRect:rc styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [_mainWindow setTitle:@"Picasso demos"];
    [_mainWindow makeKeyAndOrderFront:_mainWindow];
    [_mainWindow makeKeyWindow];
    [_mainWindow center];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)aNottification {
    return YES;
}

@end

#include "picasso.h"
#include "interface.h"

void refresh(const ps_rect* r)
{
}

unsigned set_timer(unsigned mc)
{
    return 0;
}

void clear_timer(unsigned id)
{
}

int main(int argc, const char * argv[])
{
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        AppDelegate * delegate = [[AppDelegate alloc] init];
        NSApplication * application = [NSApplication sharedApplication];
        [application setDelegate:delegate];
    }
    return NSApplicationMain(argc, argv);
}
