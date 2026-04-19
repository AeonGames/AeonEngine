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
#import <CoreGraphics/CoreGraphics.h>
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
    /** Translate an NSEvent modifier flag mask into a KeyModifier bitmask. */
    static uint32_t TranslateNSModifiers ( NSEventModifierFlags aFlags )
    {
        uint32_t mods = KeyModifier_None;
        if ( aFlags & NSEventModifierFlagShift )
        {
            mods |= KeyModifier_Shift;
        }
        if ( aFlags & NSEventModifierFlagControl )
        {
            mods |= KeyModifier_Ctrl;
        }
        if ( aFlags & NSEventModifierFlagOption )
        {
            mods |= KeyModifier_Alt;
        }
        if ( aFlags & NSEventModifierFlagCommand )
        {
            mods |= KeyModifier_Super;
        }
        return mods;
    }

    /** Translate an NSEvent buttonNumber into a normalized MouseButton value.
     *  Cocoa already uses 0=Left, 1=Right, 2=Middle, 3+=Other, which lines up
     *  with MouseButton_Left/Right/Middle/X1/X2; this is just a clamp + name. */
    static int32_t TranslateNSButton ( NSInteger aButton )
    {
        switch ( aButton )
        {
        case 0:
            return MouseButton_Left;
        case 1:
            return MouseButton_Right;
        case 2:
            return MouseButton_Middle;
        case 3:
            return MouseButton_X1;
        case 4:
            return MouseButton_X2;
        default:
            return -1;
        }
    }
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
            SetInputSystem ( mInputSystem.get() );
        }
    }

    Window::~Window()
    {
        SetInputSystem ( nullptr );
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
                        {
                            uint32_t key = [event keyCode];
                            if ( mInputSystem )
                            {
                                mInputSystem->SetKeyModifiers ( TranslateNSModifiers ( [event modifierFlags] ) );
                            }
                            bool consumed = mGuiOverlay && mGuiOverlay->OnKeyEvent ( key, true );
                            if ( !consumed && mInputSystem )
                            {
                                mInputSystem->OnKeyEvent ( key, true );
                            }
                            // Forward typed characters; route through the GUI
                            // overlay first, fall back to InputSystem only if
                            // the overlay does not consume them.
                            NSString* characters = [event characters];
                            NSUInteger len = [characters length];
                            for ( NSUInteger i = 0; i < len; ++i )
                            {
                                unichar unit = [characters characterAtIndex:i];
                                uint32_t codepoint = unit;
                                // Combine UTF-16 surrogate pairs into a UTF-32 codepoint.
                                if ( unit >= 0xD800 && unit <= 0xDBFF && ( i + 1 ) < len )
                                {
                                    unichar low = [characters characterAtIndex:i + 1];
                                    if ( low >= 0xDC00 && low <= 0xDFFF )
                                    {
                                        codepoint = 0x10000u
                                                    + ( ( static_cast<uint32_t> ( unit - 0xD800 ) ) << 10 )
                                                    + ( low - 0xDC00 );
                                        ++i;
                                    }
                                }
                                // Skip control characters; arrow keys, F-keys etc.
                                // are reported here in the Unicode private area
                                // (0xF700-0xF8FF) and as control codes (< 0x20).
                                if ( codepoint < 0x20 || ( codepoint >= 0xF700 && codepoint <= 0xF8FF ) || codepoint == 0x7F )
                                {
                                    continue;
                                }
                                bool char_consumed = mGuiOverlay && mGuiOverlay->OnTextInput ( codepoint );
                                if ( !char_consumed && mInputSystem )
                                {
                                    mInputSystem->OnChar ( codepoint );
                                }
                            }
                        }
                        break;
                        case NSEventTypeKeyUp:
                        {
                            uint32_t key = [event keyCode];
                            if ( mInputSystem )
                            {
                                mInputSystem->SetKeyModifiers ( TranslateNSModifiers ( [event modifierFlags] ) );
                            }
                            bool consumed = mGuiOverlay && mGuiOverlay->OnKeyEvent ( key, false );
                            if ( !consumed && mInputSystem )
                            {
                                mInputSystem->OnKeyEvent ( key, false );
                            }
                        }
                        break;
                        case NSEventTypeFlagsChanged:
                        {
                            // Sent when modifier keys (Shift/Ctrl/Option/Cmd) change
                            // state without producing a normal key event.
                            if ( mInputSystem )
                            {
                                mInputSystem->SetKeyModifiers ( TranslateNSModifiers ( [event modifierFlags] ) );
                            }
                        }
                        break;
                        case NSEventTypeLeftMouseDown:
                        case NSEventTypeRightMouseDown:
                        case NSEventTypeOtherMouseDown:
                        {
                            NSPoint loc = [event locationInWindow];
                            NSRect frame = [[mNSWindow contentView] frame];
                            int32_t x = static_cast<int32_t> ( loc.x );
                            int32_t y = static_cast<int32_t> ( frame.size.height - loc.y );
                            int32_t button = TranslateNSButton ( [event buttonNumber] );
                            if ( button >= 0 )
                            {
                                bool consumed = mGuiOverlay && mGuiOverlay->OnMouseButton ( button, true, x, y );
                                if ( !consumed && mInputSystem )
                                {
                                    mInputSystem->OnMouseButton ( button, true, x, y );
                                }
                            }
                        }
                        break;
                        case NSEventTypeLeftMouseUp:
                        case NSEventTypeRightMouseUp:
                        case NSEventTypeOtherMouseUp:
                        {
                            NSPoint loc = [event locationInWindow];
                            NSRect frame = [[mNSWindow contentView] frame];
                            int32_t x = static_cast<int32_t> ( loc.x );
                            int32_t y = static_cast<int32_t> ( frame.size.height - loc.y );
                            int32_t button = TranslateNSButton ( [event buttonNumber] );
                            if ( button >= 0 )
                            {
                                bool consumed = mGuiOverlay && mGuiOverlay->OnMouseButton ( button, false, x, y );
                                if ( !consumed && mInputSystem )
                                {
                                    mInputSystem->OnMouseButton ( button, false, x, y );
                                }
                            }
                        }
                        break;
                        case NSEventTypeMouseMoved:
                        case NSEventTypeLeftMouseDragged:
                        case NSEventTypeRightMouseDragged:
                        case NSEventTypeOtherMouseDragged:
                        {
                            NSPoint loc = [event locationInWindow];
                            NSRect frame = [[mNSWindow contentView] frame];
                            int32_t x = static_cast<int32_t> ( loc.x );
                            int32_t y = static_cast<int32_t> ( frame.size.height - loc.y );
                            bool consumed = mGuiOverlay && mGuiOverlay->OnMouseMove ( x, y );
                            if ( !consumed && mInputSystem )
                            {
                                mInputSystem->OnMouseMove ( x, y );
                            }
                        }
                        break;
                        case NSEventTypeScrollWheel:
                        {
                            // Prefer precise (high-resolution trackpad) deltas
                            // when available; otherwise fall back to integer
                            // wheel deltas.
                            float dx;
                            float dy;
                            if ( [event hasPreciseScrollingDeltas] )
                            {
                                // Cocoa precise deltas are in pixels; scale to
                                // approximate "wheel notches" so back-ends get
                                // values comparable to other platforms.
                                dx = static_cast<float> ( [event scrollingDeltaX] ) / 10.0f;
                                dy = static_cast<float> ( [event scrollingDeltaY] ) / 10.0f;
                            }
                            else
                            {
                                dx = static_cast<float> ( [event scrollingDeltaX] );
                                dy = static_cast<float> ( [event scrollingDeltaY] );
                            }
                            bool consumed = mGuiOverlay && mGuiOverlay->OnMouseWheel ( dx, dy );
                            if ( !consumed && mInputSystem )
                            {
                                mInputSystem->OnMouseWheel ( dx, dy );
                            }
                        }
                        break;
                        case NSEventTypeAppKitDefined:
                        {
                            // Window focus changes arrive as AppKit-defined
                            // events with specific subtypes.
                            switch ( [event subtype] )
                            {
                            case NSEventSubtypeWindowExposed:
                                break;
                            case NSEventSubtypeApplicationActivated:
                                if ( mInputSystem )
                                {
                                    mInputSystem->OnFocusGained();
                                    mInputSystem->SetKeyModifiers ( TranslateNSModifiers ( [NSEvent modifierFlags] ) );
                                }
                                break;
                            case NSEventSubtypeApplicationDeactivated:
                                if ( mInputSystem )
                                {
                                    mInputSystem->OnFocusLost();
                                }
                                break;
                            default:
                                break;
                            }
                        }
                        break;
                        default:
                            break;
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
                        // Apply cursor capture / relative-mouse-mode requests.
                        static bool prev_cursor_captured = false;
                        static bool cursor_hidden = false;
                        bool cursor_captured = mInputSystem->IsCursorCaptured() || mInputSystem->IsRelativeMouseMode();
                        if ( cursor_captured != prev_cursor_captured )
                        {
                            if ( cursor_captured )
                            {
                                if ( !cursor_hidden )
                                {
                                    [NSCursor hide];
                                    cursor_hidden = true;
                                }
                                CGAssociateMouseAndMouseCursorPosition ( false );
                            }
                            else
                            {
                                CGAssociateMouseAndMouseCursorPosition ( true );
                                if ( cursor_hidden )
                                {
                                    [NSCursor unhide];
                                    cursor_hidden = false;
                                }
                            }
                            prev_cursor_captured = cursor_captured;
                        }
                        // In relative-mouse mode, recenter the cursor in the
                        // window each frame so deltas keep accumulating.
                        if ( mInputSystem->IsRelativeMouseMode() )
                        {
                            NSRect content = [[mNSWindow contentView] frame];
                            NSPoint center_view = NSMakePoint ( content.size.width / 2.0, content.size.height / 2.0 );
                            NSRect center_rect = [mNSWindow convertRectToScreen:NSMakeRect ( center_view.x, center_view.y, 0, 0 )];
                            // CGWarpMouseCursorPosition uses top-left screen
                            // coordinates; flip Y from Cocoa's bottom-left.
                            CGFloat screen_height = [[NSScreen mainScreen] frame].size.height;
                            CGPoint warp = CGPointMake ( center_rect.origin.x, screen_height - center_rect.origin.y );
                            CGWarpMouseCursorPosition ( warp );
                            int32_t cx = static_cast<int32_t> ( content.size.width / 2.0 );
                            int32_t cy = static_cast<int32_t> ( content.size.height / 2.0 );
                            mInputSystem->OnMouseMove ( cx, cy );
                        }
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
