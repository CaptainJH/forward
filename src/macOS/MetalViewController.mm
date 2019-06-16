//
//  MetalViewController.m
//  forwardMetal
//
//  Created by jhq on 2019/5/15.
//

#import "MetalViewController.h"
#import <Metal/Metal.h>
#include "ApplicationMacImpl.h"

@implementation MetalViewController
{
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do view setup here.
    NSLog(@"MetalViewController loaded!");
    
    [self.view.window makeFirstResponder:self];
    
    forward::ApplicationMacImpl::gContext.Application->InitWithView(self.view);
    ((MTKView*)self.view).delegate = self;
    
    [self _reshape];
}

- (void) viewWillAppear
{
    [super viewWillAppear];
    NSSize size;
    size.width = forward::ApplicationMacImpl::gContext.Application->GetWindowWidth();
    size.height = forward::ApplicationMacImpl::gContext.Application->GetWindowHeight();
    self.preferredContentSize = size;
}

- (void) viewWillDisappear
{
    NSLog(@"about to exit");
}

- (void)_reshape
{
    
}

- (void)_update
{
    
}

// Called whenever view changes orientation or layout is changed
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    [self _reshape];
}


// Called whenever the view needs to render
- (void)drawInMTKView:(nonnull MTKView *)view
{
//    @autoreleasepool {
//        [self _render];
//    }
    forward::ApplicationMacImpl::gContext.Application->Render();
}

-(void)keyDown:(NSEvent *)theEvent
{
    unichar characterHit = [[theEvent charactersIgnoringModifiers] characterAtIndex:0];
    forward::ApplicationMacImpl::gContext.Application->OnInput(characterHit);
    if (characterHit == 'q' || characterHit == 27)
    {
        [NSApp terminate:self];
    }
    else
    {
        [super keyDown:theEvent];
    }
}

@end
