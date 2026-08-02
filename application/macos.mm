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
#include "aeongames/KeyCode.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Carbon/Carbon.h>
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

    /// Carbon virtual key codes occupy the low 7 bits.
    static constexpr size_t kMacKeyCodeCount = 0x80;

    /** Carbon kVK_* virtual key code to KeyCode. Despite the name these are
     *  hardware key codes, i.e. layout independent, matching KeyCode. */
    static constexpr std::array<KeyCode, kMacKeyCodeCount> kMacToKeyCode = []
    {
        std::array<KeyCode, kMacKeyCodeCount> table{};
        table[kVK_ANSI_A] = KeyCode::A;
        table[kVK_ANSI_B] = KeyCode::B;
        table[kVK_ANSI_C] = KeyCode::C;
        table[kVK_ANSI_D] = KeyCode::D;
        table[kVK_ANSI_E] = KeyCode::E;
        table[kVK_ANSI_F] = KeyCode::F;
        table[kVK_ANSI_G] = KeyCode::G;
        table[kVK_ANSI_H] = KeyCode::H;
        table[kVK_ANSI_I] = KeyCode::I;
        table[kVK_ANSI_J] = KeyCode::J;
        table[kVK_ANSI_K] = KeyCode::K;
        table[kVK_ANSI_L] = KeyCode::L;
        table[kVK_ANSI_M] = KeyCode::M;
        table[kVK_ANSI_N] = KeyCode::N;
        table[kVK_ANSI_O] = KeyCode::O;
        table[kVK_ANSI_P] = KeyCode::P;
        table[kVK_ANSI_Q] = KeyCode::Q;
        table[kVK_ANSI_R] = KeyCode::R;
        table[kVK_ANSI_S] = KeyCode::S;
        table[kVK_ANSI_T] = KeyCode::T;
        table[kVK_ANSI_U] = KeyCode::U;
        table[kVK_ANSI_V] = KeyCode::V;
        table[kVK_ANSI_W] = KeyCode::W;
        table[kVK_ANSI_X] = KeyCode::X;
        table[kVK_ANSI_Y] = KeyCode::Y;
        table[kVK_ANSI_Z] = KeyCode::Z;
        table[kVK_ANSI_1] = KeyCode::Num1;
        table[kVK_ANSI_2] = KeyCode::Num2;
        table[kVK_ANSI_3] = KeyCode::Num3;
        table[kVK_ANSI_4] = KeyCode::Num4;
        table[kVK_ANSI_5] = KeyCode::Num5;
        table[kVK_ANSI_6] = KeyCode::Num6;
        table[kVK_ANSI_7] = KeyCode::Num7;
        table[kVK_ANSI_8] = KeyCode::Num8;
        table[kVK_ANSI_9] = KeyCode::Num9;
        table[kVK_ANSI_0] = KeyCode::Num0;
        table[kVK_ANSI_Grave] = KeyCode::Grave;
        table[kVK_ANSI_Minus] = KeyCode::Minus;
        table[kVK_ANSI_Equal] = KeyCode::Equal;
        table[kVK_ANSI_LeftBracket] = KeyCode::LeftBracket;
        table[kVK_ANSI_RightBracket] = KeyCode::RightBracket;
        table[kVK_ANSI_Backslash] = KeyCode::Backslash;
        table[kVK_ANSI_Semicolon] = KeyCode::Semicolon;
        table[kVK_ANSI_Quote] = KeyCode::Apostrophe;
        table[kVK_ANSI_Comma] = KeyCode::Comma;
        table[kVK_ANSI_Period] = KeyCode::Period;
        table[kVK_ANSI_Slash] = KeyCode::Slash;
        table[kVK_ISO_Section] = KeyCode::NonUsBackslash;
        table[kVK_Return] = KeyCode::Enter;
        table[kVK_Tab] = KeyCode::Tab;
        table[kVK_Space] = KeyCode::Space;
        table[kVK_Delete] = KeyCode::Backspace;
        table[kVK_ForwardDelete] = KeyCode::Delete;
        table[kVK_Escape] = KeyCode::Escape;
        table[kVK_CapsLock] = KeyCode::CapsLock;
        table[kVK_Command] = KeyCode::LeftSuper;
        table[kVK_RightCommand] = KeyCode::RightSuper;
        table[kVK_Shift] = KeyCode::LeftShift;
        table[kVK_RightShift] = KeyCode::RightShift;
        table[kVK_Option] = KeyCode::LeftAlt;
        table[kVK_RightOption] = KeyCode::RightAlt;
        table[kVK_Control] = KeyCode::LeftCtrl;
        table[kVK_RightControl] = KeyCode::RightCtrl;
        table[kVK_Home] = KeyCode::Home;
        table[kVK_End] = KeyCode::End;
        table[kVK_PageUp] = KeyCode::PageUp;
        table[kVK_PageDown] = KeyCode::PageDown;
        table[kVK_LeftArrow] = KeyCode::Left;
        table[kVK_RightArrow] = KeyCode::Right;
        table[kVK_DownArrow] = KeyCode::Down;
        table[kVK_UpArrow] = KeyCode::Up;
        table[kVK_Help] = KeyCode::Insert;
        table[kVK_Mute] = KeyCode::Mute;
        table[kVK_VolumeUp] = KeyCode::VolumeUp;
        table[kVK_VolumeDown] = KeyCode::VolumeDown;
        table[kVK_F1] = KeyCode::F1;
        table[kVK_F2] = KeyCode::F2;
        table[kVK_F3] = KeyCode::F3;
        table[kVK_F4] = KeyCode::F4;
        table[kVK_F5] = KeyCode::F5;
        table[kVK_F6] = KeyCode::F6;
        table[kVK_F7] = KeyCode::F7;
        table[kVK_F8] = KeyCode::F8;
        table[kVK_F9] = KeyCode::F9;
        table[kVK_F10] = KeyCode::F10;
        table[kVK_F11] = KeyCode::F11;
        table[kVK_F12] = KeyCode::F12;
        table[kVK_F13] = KeyCode::PrintScreen;
        table[kVK_F14] = KeyCode::ScrollLock;
        table[kVK_F15] = KeyCode::Pause;
        table[kVK_F16] = KeyCode::F16;
        table[kVK_F17] = KeyCode::F17;
        table[kVK_F18] = KeyCode::F18;
        table[kVK_F19] = KeyCode::F19;
        table[kVK_F20] = KeyCode::F20;
        table[kVK_ANSI_Keypad0] = KeyCode::Keypad0;
        table[kVK_ANSI_Keypad1] = KeyCode::Keypad1;
        table[kVK_ANSI_Keypad2] = KeyCode::Keypad2;
        table[kVK_ANSI_Keypad3] = KeyCode::Keypad3;
        table[kVK_ANSI_Keypad4] = KeyCode::Keypad4;
        table[kVK_ANSI_Keypad5] = KeyCode::Keypad5;
        table[kVK_ANSI_Keypad6] = KeyCode::Keypad6;
        table[kVK_ANSI_Keypad7] = KeyCode::Keypad7;
        table[kVK_ANSI_Keypad8] = KeyCode::Keypad8;
        table[kVK_ANSI_Keypad9] = KeyCode::Keypad9;
        table[kVK_ANSI_KeypadDecimal] = KeyCode::KeypadDecimal;
        table[kVK_ANSI_KeypadDivide] = KeyCode::KeypadDivide;
        table[kVK_ANSI_KeypadEnter] = KeyCode::KeypadEnter;
        table[kVK_ANSI_KeypadEquals] = KeyCode::KeypadEqual;
        table[kVK_ANSI_KeypadMinus] = KeyCode::KeypadSubtract;
        table[kVK_ANSI_KeypadMultiply] = KeyCode::KeypadMultiply;
        table[kVK_ANSI_KeypadPlus] = KeyCode::KeypadAdd;
        table[kVK_ANSI_KeypadClear] = KeyCode::NumLock;
        table[kVK_JIS_Yen] = KeyCode::International3;
        table[kVK_JIS_Underscore] = KeyCode::International1;
        table[kVK_JIS_KeypadComma] = KeyCode::KeypadComma;
        table[kVK_JIS_Eisu] = KeyCode::Lang2;
        table[kVK_JIS_Kana] = KeyCode::Lang1;
        return table;
    }
    ();

    /** Translate a Carbon virtual key code into a KeyCode.
     *  Returns KeyCode::Unknown for keys outside the table. */
    static KeyCode TranslateNSKeyCode ( unsigned short aKeyCode )
    {
        return ( aKeyCode < kMacKeyCodeCount ) ? kMacToKeyCode[aKeyCode] : KeyCode::Unknown;
    }

    /** Returns the NSEvent modifier flag a key contributes, or 0.
     *  Cocoa's public flags do not distinguish left from right, so with both
     *  of a pair held, releasing one still reports the other's flag as set. */
    static NSEventModifierFlags ModifierFlagFor ( KeyCode aKeyCode )
    {
        switch ( aKeyCode )
        {
        case KeyCode::LeftShift:
        case KeyCode::RightShift:
            return NSEventModifierFlagShift;
        case KeyCode::LeftCtrl:
        case KeyCode::RightCtrl:
            return NSEventModifierFlagControl;
        case KeyCode::LeftAlt:
        case KeyCode::RightAlt:
            return NSEventModifierFlagOption;
        case KeyCode::LeftSuper:
        case KeyCode::RightSuper:
            return NSEventModifierFlagCommand;
        case KeyCode::CapsLock:
            return NSEventModifierFlagCapsLock;
        default:
            return 0;
        }
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
    Window::Window ( const std::string& aRendererName, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight,
                     bool aFullScreen, const RendererSettings& aRendererSettings )
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

            mRenderer = ConstructRenderer ( aRendererName, ( __bridge void* ) mNSView, aRendererSettings );
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

    void Window::SetTitle ( const std::string& aTitle )
    {
        [mNSWindow setTitle:[NSString stringWithUTF8String:aTitle.c_str()]];
    }

    void Window::Run ( Scene& aScene )
    {
        bool running{true};
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };
        aScene.SetInputSystem ( mInputSystem.get() );

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
                            KeyCode key = TranslateNSKeyCode ( [event keyCode] );
                            if ( mInputSystem )
                            {
                                mInputSystem->SetKeyModifiers ( TranslateNSModifiers ( [event modifierFlags] ) );
                            }
                            bool consumed = key != KeyCode::Unknown && mGuiOverlay && mGuiOverlay->OnKeyEvent ( key, true );
                            if ( !consumed )
                            {
                                // ESC exits the application unless the GUI overlay consumed it.
                                if ( key == KeyCode::Escape )
                                {
                                    running = false;
                                    break;
                                }
                                // F1 toggles the renderer's debug-geometry overlay.
                                if ( key == KeyCode::F1 )
                                {
                                    ToggleDebugRendering();
                                    break;
                                }
                                // F2/F3/F4 toggle a whole light type on/off
                                // (debugging aid: e.g. disable point and spot
                                // lights to isolate the directional light's
                                // shadow).
                                if ( key == KeyCode::F2 )
                                {
                                    ToggleLightType ( LightType::Directional );
                                    break;
                                }
                                if ( key == KeyCode::F3 )
                                {
                                    ToggleLightType ( LightType::Point );
                                    break;
                                }
                                if ( key == KeyCode::F4 )
                                {
                                    ToggleLightType ( LightType::Spot );
                                    break;
                                }
                                if ( key != KeyCode::Unknown && mInputSystem )
                                {
                                    mInputSystem->OnKeyEvent ( key, true );
                                }
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
                            KeyCode key = TranslateNSKeyCode ( [event keyCode] );
                            if ( mInputSystem )
                            {
                                mInputSystem->SetKeyModifiers ( TranslateNSModifiers ( [event modifierFlags] ) );
                            }
                            if ( key == KeyCode::Unknown )
                            {
                                break;
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
                            // Cocoa reports modifier keys only as a flag change,
                            // never as KeyDown/KeyUp, so their pressed state has
                            // to be derived from the flag mask or queries like
                            // IsKeyDown ( KeyCode::LeftShift ) never fire.
                            NSEventModifierFlags flags = [event modifierFlags];
                            if ( mInputSystem )
                            {
                                mInputSystem->SetKeyModifiers ( TranslateNSModifiers ( flags ) );
                            }
                            KeyCode key = TranslateNSKeyCode ( [event keyCode] );
                            NSEventModifierFlags key_flag = ModifierFlagFor ( key );
                            if ( key_flag == 0 )
                            {
                                break;
                            }
                            bool pressed = ( flags & key_flag ) != 0;
                            bool consumed = mGuiOverlay && mGuiOverlay->OnKeyEvent ( key, pressed );
                            if ( !consumed && mInputSystem )
                            {
                                mInputSystem->OnKeyEvent ( key, pressed );
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
                        // Wait for the previous frame to finish before
                        // overwriting the per-frame uniform buffers. The
                        // view/projection matrices and light buffers are
                        // single-buffered and host-visible, so they must not be
                        // written while the previous frame's GPU work may still
                        // be reading them. BeginFrame() waits on the frame
                        // fence; doing it first closes a write-after-read race
                        // that shows up as geometry flicker while the camera is
                        // moving. BeginFrame() is idempotent, so the later
                        // BeginRender() reuses this frame's command recording.
                        mRenderer->BeginFrame ( ( __bridge void* ) mNSView );
                        if ( aScene.GetCamera() )
                        {
                            mRenderer->SetViewMatrix ( ( __bridge void* ) mNSView, aScene.GetViewMatrix() );
                            Matrix4x4 projection{};
                            projection.Perspective ( aScene.GetFieldOfView(), mAspectRatio, aScene.GetNear(), aScene.GetFar() );
                            mRenderer->SetProjectionMatrix ( ( __bridge void* ) mNSView, projection );
                        }
                        mRenderer->SetLights ( ( __bridge void* ) mNSView, aScene.GetFrameLights() );
                        mRenderer->SetGlobals ( ( __bridge void* ) mNSView, aScene.GetGlobals() );
                        mRenderer->SetEnvironmentMap ( ( __bridge void* ) mNSView, aScene.GetEnvironmentMap() );
                        if ( mGuiOverlay )
                        {
                            mGuiOverlay->BeginFrame ( ( __bridge void* ) mNSView, delta.count() );
                            mGuiOverlay->EndFrame ( ( __bridge void* ) mNSView );
                        }
                        // Compute skinning pre-pass: dispatch skinning before the
                        // render pass begins so the skinned vertex buffers are
                        // ready for both the depth and shading traversals.
                        aScene.LoopTraverseDFSPreOrder ( [this] ( const Node & aNode )
                        {
                            aNode.Skin ( *mRenderer, ( __bridge void* ) mNSView );
                        } );
                        // Hand the whole frame to the renderer: it brackets
                        // BeginRender/EndRender, builds the render queue from the
                        // window frustum, runs the depth pre-pass and light
                        // culling when the scene has a lighting pipeline, submits
                        // the shading pass and composites the overlay.
                        mRenderer->RenderScene ( ( __bridge void* ) mNSView, aScene, mGuiOverlay.get() );
                    }
                }
            }
        }
    }
}
#endif
