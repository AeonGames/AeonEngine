/*
Copyright (C) 2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <stdexcept>
#include "aeongames/LogLevel.hpp"

extern "C" void* GetMetalLayerFromNSView ( void* nsview_ptr )
{
    @autoreleasepool
    {
        if ( !nsview_ptr )
        {
            return nullptr;
        }

        NSView* view = ( __bridge NSView* ) nsview_ptr;

        // Ensure we're on the main thread for AppKit operations
        if ( ![NSThread isMainThread] )
        {
            std::cout << AeonGames::LogLevel::Error << "GetMetalLayerFromNSView must be called from main thread" << std::endl;
            throw std::runtime_error ( "GetMetalLayerFromNSView must be called from main thread" );
            return nullptr;
        }

        // Check if the view already has a metal layer
        if ( !view.layer || ![view.layer isKindOfClass:[CAMetalLayer class]] )
        {
            std::cout << AeonGames::LogLevel::Error << "NSView does not have a CAMetalLayer" << std::endl;
            throw std::runtime_error ( "NSView does not have a CAMetalLayer" );
            return nullptr;
        }

        return ( __bridge void* ) view.layer;
    }
}
