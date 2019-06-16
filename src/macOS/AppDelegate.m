//
//  AppDelegate.m
//  forward_metal
//
//  Created by jhq on 2019/5/14.
//  Copyright © 2019 jhq. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@property IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
}


- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}


@end
