/*
Copyright (C) 2025 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#import <QuartzCore/CAMetalLayer.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>

extern "C" void* GetMetalLayerFromNSView(void* nsview_ptr)
{
    @autoreleasepool {
        if (!nsview_ptr)
        {
            return nullptr;
        }
        
        NSView* view = (__bridge NSView*)nsview_ptr;
        
        // Ensure we're on the main thread for AppKit operations
        if (![NSThread isMainThread])
        {
            NSLog(@"Warning: GetMetalLayerFromNSView called from non-main thread");
            return nullptr;
        }
        
        // Check if the view already has a metal layer
        if (view.layer && [view.layer isKindOfClass:[CAMetalLayer class]])
        {
            return (__bridge void*)view.layer;
        }
        
        // Create a CAMetalLayer
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        
        // Important: Set wantsLayer BEFORE setting the layer to avoid issues
        view.wantsLayer = YES;
        view.layer = metalLayer;
        
        // Remove all gesture recognizers to prevent Swift runtime conflicts
        // This prevents issues with Qt's QMacStyle gesture recognizer management
        NSArray* recognizers = [view.gestureRecognizers copy];
        for (NSGestureRecognizer* recognizer in recognizers)
        {
            [view removeGestureRecognizer:recognizer];
        }
        
        // Set the content scale to match the display
        if (view.window)
        {
            metalLayer.contentsScale = view.window.backingScaleFactor;
        }
        else
        {
            metalLayer.contentsScale = [[NSScreen mainScreen] backingScaleFactor];
        }
        
        // Return bridged pointer without transferring ownership
        return (__bridge void*)metalLayer;
    }
}
