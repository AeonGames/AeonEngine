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
#ifdef __APPLE__
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/GuiOverlay.hpp"
#include "aeongames/InputSystem.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#import <Cocoa/Cocoa.h>
#include "Window.h"

int Main ( int argc, char *argv[] );

int ENTRYPOINT main ( int argc, char *argv[] )
{
    @autoreleasepool
    {
        return Main ( argc, argv );
    }
}

namespace AeonGames
{
    Window::Window ( const std::string& aRendererName, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight, bool aFullScreen )
    {
        @autoreleasepool
        {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

            NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
            NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
            if ( aFullScreen )
            {
                NSScreen* screen = [NSScreen mainScreen];
                NSRect screenRect = [screen frame];
                aX = 0;
                aY = 0;
                aWidth = static_cast<uint32_t> ( screenRect.size.width );
                aHeight = static_cast<uint32_t> ( screenRect.size.height );
                styleMask = NSWindowStyleMaskBorderless;
            }

            NSRect contentRect = NSMakeRect ( aX, aY, aWidth, aHeight );
            mNSWindow = [[NSWindow alloc] initWithContentRect:contentRect
                         styleMask:styleMask
                         backing:NSBackingStoreBuffered
                         defer:NO];
            [mNSWindow setTitle:@"AeonGames"];
            [mNSWindow setAcceptsMouseMovedEvents:YES];

            mNSView = [mNSWindow contentView];
            [mNSView setWantsLayer:YES];

            mRenderer = ConstructRenderer ( aRendererName, ( __bridge void* ) mNSView );
            EnumerateGuiOverlayConstructors ( [this] ( const StringId & aIdentifier ) -> bool
            {
                mGuiOverlay = ConstructGuiOverlay ( aIdentifier, ( __bridge void* ) mNSView );
                return mGuiOverlay == nullptr;
            } );
            EnumerateInputSystemConstructors ( [this] ( const StringId & aIdentifier ) -> bool
            {
                mInputSystem = ConstructInputSystem ( aIdentifier );
                return mInputSystem == nullptr;
            } );
        }
    }

    Window::~Window()
    {
        if ( mRenderer )
        {
            mRenderer->DetachWindow ( this );
        }
        @autoreleasepool
        {
            if ( mNSWindow )
            {
                [mNSWindow close];
                mNSWindow = nil;
            }
        }
    }

    uint32_t Window::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight && mRenderer )
        {
            mRenderer->ResizeViewport ( ( __bridge void* ) mNSView, 0, 0, aWidth, aHeight );
            mAspectRatio = static_cast<float> ( aWidth ) / static_cast<float> ( aHeight );
            if ( mGuiOverlay )
            {
                mGuiOverlay->Resize ( aWidth, aHeight );
            }
        }
        return 0;
    }

    void Window::Run ( Scene& aScene )
    {
        bool running{true};
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };

        @autoreleasepool
        {
            [mNSWindow makeKeyAndOrderFront:nil];
            [NSApp activateIgnoringOtherApps:YES];

            NSRect frame = [[mNSWindow contentView] frame];
            Resize ( static_cast<uint32_t> ( frame.size.width ), static_cast<uint32_t> ( frame.size.height ) );

            while ( running )
            {
                @autoreleasepool
                {
                    NSEvent* event = nil;
                    while ( ( event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:nil
                                       inMode:NSDefaultRunLoopMode
                                       dequeue:YES] ) != nil )
                    {
                        switch ( [event type] )
                        {
                        case NSEventTypeKeyDown:
                            if ( mInputSystem )
                            {
                                mInputSystem->OnKeyEvent ( [event keyCode], true );
                            }
                            break;
                        case NSEventTypeKeyUp:
                            if ( mInputSystem )
                            {
                                mInputSystem->OnKeyEvent ( [event keyCode], false );
                            }
                            break;
                        case NSEventTypeLeftMouseDown:
                        case NSEventTypeRightMouseDown:
                        case NSEventTypeOtherMouseDown:
                            if ( mInputSystem )
                            {
                                NSPoint loc = [event locationInWindow];
                                NSRect frame = [[mNSWindow contentView] frame];
                                mInputSystem->OnMouseButton ( static_cast<int32_t> ( [event buttonNumber] ), true,
                                                              static_cast<int32_t> ( loc.x ), static_cast<int32_t> ( frame.size.height - loc.y ) );
                            }
                            break;
                        case NSEventTypeLeftMouseUp:
                        case NSEventTypeRightMouseUp:
                        case NSEventTypeOtherMouseUp:
                            if ( mInputSystem )
                            {
                                NSPoint loc = [event locationInWindow];
                                NSRect frame = [[mNSWindow contentView] frame];
                                mInputSystem->OnMouseButton ( static_cast<int32_t> ( [event buttonNumber] ), false,
                                                              static_cast<int32_t> ( loc.x ), static_cast<int32_t> ( frame.size.height - loc.y ) );
                            }
                            break;
                        case NSEventTypeMouseMoved:
                        case NSEventTypeLeftMouseDragged:
                        case NSEventTypeRightMouseDragged:
                        case NSEventTypeOtherMouseDragged:
                            if ( mInputSystem )
                            {
                                NSPoint loc = [event locationInWindow];
                                NSRect frame = [[mNSWindow contentView] frame];
                                mInputSystem->OnMouseMove ( static_cast<int32_t> ( loc.x ), static_cast<int32_t> ( frame.size.height - loc.y ) );
                            }
                            break;
                        default:
                            break;
                        }

                        if ( [event type] == NSEventTypeApplicationDefined &&
                             [event subtype] == NSEventSubtypeApplicationActivated )
                        {
                            // Handle window close via application events if needed
                        }
                        [NSApp sendEvent:event];
                        [NSApp updateWindows];

                        if ( ![mNSWindow isVisible] )
                        {
                            running = false;
                        }
                    }

                    if ( ![mNSWindow isVisible] )
                    {
                        running = false;
                        break;
                    }

                    // Check for resize
                    NSRect currentFrame = [[mNSWindow contentView] frame];
                    NSRect backingFrame = [[mNSWindow contentView] convertRectToBacking:currentFrame];
                    uint32_t currentWidth = static_cast<uint32_t> ( backingFrame.size.width );
                    uint32_t currentHeight = static_cast<uint32_t> ( backingFrame.size.height );
                    static uint32_t lastWidth = 0;
                    static uint32_t lastHeight = 0;
                    if ( currentWidth != lastWidth || currentHeight != lastHeight )
                    {
                        Resize ( currentWidth, currentHeight );
                        lastWidth = currentWidth;
                        lastHeight = currentHeight;
                    }

                    std::chrono::high_resolution_clock::time_point current_time{std::chrono::high_resolution_clock::now() };
                    std::chrono::duration<double> delta{std::chrono::duration_cast<std::chrono::duration<double >> ( current_time - last_time ) };
                    if ( mInputSystem )
                    {
                        mInputSystem->Update();
                    }
                    aScene.Update ( delta.count() );
                    last_time = current_time;

                    if ( mRenderer )
                    {
                        if ( const Node * camera = aScene.GetCamera() )
                        {
                            mRenderer->SetViewMatrix ( ( __bridge void* ) mNSView, camera->GetGlobalTransform().GetInverted().GetMatrix() );
                            Matrix4x4 projection{};
                            projection.Perspective ( aScene.GetFieldOfView(), mAspectRatio, aScene.GetNear(), aScene.GetFar() );
                            mRenderer->SetProjectionMatrix ( ( __bridge void* ) mNSView, projection );
                        }
                        if ( mGuiOverlay )
                        {
                            mGuiOverlay->BeginFrame ( ( __bridge void* ) mNSView, delta.count() );
                            mGuiOverlay->EndFrame ( ( __bridge void* ) mNSView );
                        }
                        mRenderer->BeginRender ( ( __bridge void* ) mNSView );
                        aScene.LoopTraverseDFSPreOrder ( [this] ( const Node & aNode )
                        {
                            AABB transformed_aabb = aNode.GetGlobalTransform() * aNode.GetAABB();
                            if ( mRenderer->GetFrustum ( ( __bridge void * ) mNSView ).Intersects ( transformed_aabb ) )
                            {
                                aNode.Render ( *mRenderer, ( __bridge void* ) mNSView );
                            }
                        } );
                        if ( mGuiOverlay )
                        {
                            mRenderer->RenderOverlay ( ( __bridge void* ) mNSView, *mGuiOverlay );
                        }
                        mRenderer->EndRender ( ( __bridge void* ) mNSView );
                    }
                }
            }
        }
    }
}
#endif
