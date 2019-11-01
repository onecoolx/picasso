//  platform_apple
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
    }
    return NSApplicationMain(argc, argv);
}
