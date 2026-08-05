/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#import <AppKit/AppKit.h>

#include <stdexcept>
#include "RenderTestWindow.h"

namespace AeonGames
{
    namespace
    {
        NSMutableDictionary<NSValue*, NSWindow*>* TestWindows()
        {
            static NSMutableDictionary<NSValue*, NSWindow*>* windows = nil;
            static dispatch_once_t once;
            dispatch_once ( &once, ^
            {
                windows = [[NSMutableDictionary alloc] init];
            } );
            return windows;
        }
    }

    void* CreateHiddenRenderWindowMac()
    {
        @autoreleasepool
        {
            if ( ![NSThread isMainThread] )
            {
                throw std::runtime_error ( "macOS render tests must create AppKit windows on the main thread" );
            }
            [NSApplication sharedApplication];
NSWindow* window = [[NSWindow alloc] initWithContentRect:NSMakeRect ( 0, 0, 64, 64 )
                    styleMask:NSWindowStyleMaskBorderless
                    backing:NSBackingStoreBuffered
                    defer:NO];
            NSView* view = window.contentView;
[view setWantsLayer:YES];
            void* handle = ( __bridge void* ) view;
            TestWindows() [[NSValue valueWithPointer:handle]] = window;
            return handle;
        }
    }

    void DestroyHiddenRenderWindowMac ( void* aWindow )
    {
        @autoreleasepool
        {
            if ( aWindow == nullptr )
            {
                return;
            }
            NSValue* key = [NSValue valueWithPointer:aWindow];
            NSWindow* window = TestWindows() [key];
            NSView* view = ( __bridge NSView* ) aWindow;
            [view setLayer:nil];
            [window setContentView:nil];
            [window close];
            [TestWindows() removeObjectForKey:key];
        }
    }
}