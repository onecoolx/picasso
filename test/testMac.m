//  testMac
//
//  Created by zhangjipeng on 2019/10/25.
//  Copyright © 2019 zhang  jipeng. All rights reserved.

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>


@end

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
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
    }
    return NSApplicationMain(argc, argv);
}

