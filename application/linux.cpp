/*
Copyright (C) 2016,2018-2021,2024,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/GuiOverlay.hpp"
#include "aeongames/InputSystem.hpp"
#include "aeongames/KeyCode.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <clocale>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include "Window.h"

int Main ( int argc, char *argv[] );

int ENTRYPOINT main ( int argc, char *argv[] )
{
    // The X input method decodes keystrokes according to the locale, so this
    // has to happen before any other X call for non-ASCII text input to work.
    // Only LC_CTYPE: LC_NUMERIC would change the decimal separator strtod
    // expects and break text-format asset parsing under e.g. de_DE.
    std::setlocale ( LC_CTYPE, "" );
    if ( XSupportsLocale() )
    {
        XSetLocaleModifiers ( "" );
    }
    XSetErrorHandler ( [] ( Display * mDisplay, XErrorEvent * error_event ) -> int
    {
        char error_string[1024];
        XGetErrorText ( mDisplay, error_event->error_code, error_string, 1024 );
        std::cout << AeonGames::LogLevel::Error << error_string << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Error Code " << static_cast<int> ( error_event->error_code ) << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Request Code " << static_cast<int> ( error_event->request_code ) << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Minor Code " << static_cast<int> ( error_event->minor_code ) << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Display " << error_event->display << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Resource Id " << error_event->resourceid << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Serial " << error_event->serial << std::endl;
        std::cout << AeonGames::LogLevel::Error << "Type " << error_event->type << std::endl;
        return 0;
    } );
    return Main ( argc, argv );
}

namespace AeonGames
{
    static int choose_visual_attribs[] =
    {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        None
    };

#if 0
    static const int choose_fb_config_attribs[] =
    {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };

    static GLXFBConfig GetGLXConfig ( Display* display )
    {
        int frame_buffer_config_count{};
        GLXFBConfig *frame_buffer_configs =
            glXChooseFBConfig ( display,
                                DefaultScreen ( display ),
                                choose_fb_config_attribs, &frame_buffer_config_count );
        if ( !frame_buffer_configs )
        {
            throw std::runtime_error ( "Failed to retrieve a framebuffer config" );
        }
        std::sort ( frame_buffer_configs, frame_buffer_configs + frame_buffer_config_count,
                    [display] ( const GLXFBConfig & a, const GLXFBConfig & b )->bool
        {
            int a_sample_buffers{};
            int a_samples{};
            int b_sample_buffers{};
            int b_samples{};
            glXGetFBConfigAttrib ( display, a, GLX_SAMPLE_BUFFERS, &a_sample_buffers );
            glXGetFBConfigAttrib ( display, a, GLX_SAMPLES, &a_samples  );
            glXGetFBConfigAttrib ( display, b, GLX_SAMPLE_BUFFERS, &b_sample_buffers );
            glXGetFBConfigAttrib ( display, b, GLX_SAMPLES, &b_samples  );
            return a_sample_buffers >= b_sample_buffers && a_samples > b_samples;
        } );
        GLXFBConfig result = frame_buffer_configs[ 0 ];
        XFree ( frame_buffer_configs );
        return result;
    }
#endif

    /** Translate X11 mouse-button code to a normalized MouseButton value.
     *  Returns -1 for wheel buttons (4-7) and unknown codes; the caller is
     *  expected to handle wheel events separately. */
    static int32_t TranslateX11Button ( unsigned int aButton )
    {
        switch ( aButton )
        {
        case 1:
            return MouseButton_Left;
        case 2:
            return MouseButton_Middle;
        case 3:
            return MouseButton_Right;
        case 8:
            return MouseButton_X1;
        case 9:
            return MouseButton_X2;
        default:
            return -1;
        }
    }

    /** Translate an X11 KeyEvent state mask into a KeyModifier bitmask. */
    static uint32_t TranslateX11Modifiers ( unsigned int aState )
    {
        uint32_t mods = KeyModifier_None;
        if ( aState & ShiftMask )
        {
            mods |= KeyModifier_Shift;
        }
        if ( aState & ControlMask )
        {
            mods |= KeyModifier_Ctrl;
        }
        if ( aState & Mod1Mask )
        {
            mods |= KeyModifier_Alt;
        }
        if ( aState & Mod4Mask )
        {
            mods |= KeyModifier_Super;
        }
        return mods;
    }

    /// Highest Linux evdev key code the translation table covers (KEY_F24 is 194).
    static constexpr size_t kEvdevKeyCount = 195;

    /** Linux evdev key code to KeyCode. Xorg's evdev/libinput driver reports
     *  X11 key codes as the evdev code plus 8, so this table also serves X11
     *  after subtracting that offset, and is reusable as-is for a future
     *  Wayland front-end, which receives evdev codes directly. */
    static constexpr std::array<KeyCode, kEvdevKeyCount> kEvdevToKeyCode = []
    {
        std::array<KeyCode, kEvdevKeyCount> table{};
        table[1] = KeyCode::Escape;
        table[2] = KeyCode::Num1;
        table[3] = KeyCode::Num2;
        table[4] = KeyCode::Num3;
        table[5] = KeyCode::Num4;
        table[6] = KeyCode::Num5;
        table[7] = KeyCode::Num6;
        table[8] = KeyCode::Num7;
        table[9] = KeyCode::Num8;
        table[10] = KeyCode::Num9;
        table[11] = KeyCode::Num0;
        table[12] = KeyCode::Minus;
        table[13] = KeyCode::Equal;
        table[14] = KeyCode::Backspace;
        table[15] = KeyCode::Tab;
        table[16] = KeyCode::Q;
        table[17] = KeyCode::W;
        table[18] = KeyCode::E;
        table[19] = KeyCode::R;
        table[20] = KeyCode::T;
        table[21] = KeyCode::Y;
        table[22] = KeyCode::U;
        table[23] = KeyCode::I;
        table[24] = KeyCode::O;
        table[25] = KeyCode::P;
        table[26] = KeyCode::LeftBracket;
        table[27] = KeyCode::RightBracket;
        table[28] = KeyCode::Enter;
        table[29] = KeyCode::LeftCtrl;
        table[30] = KeyCode::A;
        table[31] = KeyCode::S;
        table[32] = KeyCode::D;
        table[33] = KeyCode::F;
        table[34] = KeyCode::G;
        table[35] = KeyCode::H;
        table[36] = KeyCode::J;
        table[37] = KeyCode::K;
        table[38] = KeyCode::L;
        table[39] = KeyCode::Semicolon;
        table[40] = KeyCode::Apostrophe;
        table[41] = KeyCode::Grave;
        table[42] = KeyCode::LeftShift;
        table[43] = KeyCode::Backslash;
        table[44] = KeyCode::Z;
        table[45] = KeyCode::X;
        table[46] = KeyCode::C;
        table[47] = KeyCode::V;
        table[48] = KeyCode::B;
        table[49] = KeyCode::N;
        table[50] = KeyCode::M;
        table[51] = KeyCode::Comma;
        table[52] = KeyCode::Period;
        table[53] = KeyCode::Slash;
        table[54] = KeyCode::RightShift;
        table[55] = KeyCode::KeypadMultiply;
        table[56] = KeyCode::LeftAlt;
        table[57] = KeyCode::Space;
        table[58] = KeyCode::CapsLock;
        table[59] = KeyCode::F1;
        table[60] = KeyCode::F2;
        table[61] = KeyCode::F3;
        table[62] = KeyCode::F4;
        table[63] = KeyCode::F5;
        table[64] = KeyCode::F6;
        table[65] = KeyCode::F7;
        table[66] = KeyCode::F8;
        table[67] = KeyCode::F9;
        table[68] = KeyCode::F10;
        table[69] = KeyCode::NumLock;
        table[70] = KeyCode::ScrollLock;
        table[71] = KeyCode::Keypad7;
        table[72] = KeyCode::Keypad8;
        table[73] = KeyCode::Keypad9;
        table[74] = KeyCode::KeypadSubtract;
        table[75] = KeyCode::Keypad4;
        table[76] = KeyCode::Keypad5;
        table[77] = KeyCode::Keypad6;
        table[78] = KeyCode::KeypadAdd;
        table[79] = KeyCode::Keypad1;
        table[80] = KeyCode::Keypad2;
        table[81] = KeyCode::Keypad3;
        table[82] = KeyCode::Keypad0;
        table[83] = KeyCode::KeypadDecimal;
        table[86] = KeyCode::NonUsBackslash;
        table[87] = KeyCode::F11;
        table[88] = KeyCode::F12;
        table[89] = KeyCode::International1;
        table[92] = KeyCode::International4;
        table[93] = KeyCode::International2;
        table[94] = KeyCode::International5;
        table[95] = KeyCode::KeypadComma;
        table[96] = KeyCode::KeypadEnter;
        table[97] = KeyCode::RightCtrl;
        table[98] = KeyCode::KeypadDivide;
        table[99] = KeyCode::PrintScreen;
        table[100] = KeyCode::RightAlt;
        table[102] = KeyCode::Home;
        table[103] = KeyCode::Up;
        table[104] = KeyCode::PageUp;
        table[105] = KeyCode::Left;
        table[106] = KeyCode::Right;
        table[107] = KeyCode::End;
        table[108] = KeyCode::Down;
        table[109] = KeyCode::PageDown;
        table[110] = KeyCode::Insert;
        table[111] = KeyCode::Delete;
        table[113] = KeyCode::Mute;
        table[114] = KeyCode::VolumeDown;
        table[115] = KeyCode::VolumeUp;
        table[116] = KeyCode::Power;
        table[117] = KeyCode::KeypadEqual;
        table[119] = KeyCode::Pause;
        table[121] = KeyCode::KeypadComma;
        table[122] = KeyCode::Lang1;
        table[123] = KeyCode::Lang2;
        table[124] = KeyCode::International3;
        table[125] = KeyCode::LeftSuper;
        table[126] = KeyCode::RightSuper;
        table[127] = KeyCode::Application;
        table[138] = KeyCode::Help;
        table[139] = KeyCode::Menu;
        table[183] = KeyCode::F13;
        table[184] = KeyCode::F14;
        table[185] = KeyCode::F15;
        table[186] = KeyCode::F16;
        table[187] = KeyCode::F17;
        table[188] = KeyCode::F18;
        table[189] = KeyCode::F19;
        table[190] = KeyCode::F20;
        table[191] = KeyCode::F21;
        table[192] = KeyCode::F22;
        table[193] = KeyCode::F23;
        table[194] = KeyCode::F24;
        return table;
    }
    ();

    /** Translate an X11 hardware key code into a KeyCode.
     *  Returns KeyCode::Unknown for keys outside the table. */
    static KeyCode TranslateX11KeyCode ( unsigned int aKeyCode )
    {
        // Xorg reserves key codes 0-7, so evdev codes start at 8.
        if ( aKeyCode < 8 )
        {
            return KeyCode::Unknown;
        }
        const size_t evdev = aKeyCode - 8;
        return ( evdev < kEvdevKeyCount ) ? kEvdevToKeyCode[evdev] : KeyCode::Unknown;
    }

    /** Returns the KeyModifier bit a key contributes, or KeyModifier_None. */
    static uint32_t ModifierBitFor ( KeyCode aKeyCode )
    {
        switch ( aKeyCode )
        {
        case KeyCode::LeftShift:
        case KeyCode::RightShift:
            return KeyModifier_Shift;
        case KeyCode::LeftCtrl:
        case KeyCode::RightCtrl:
            return KeyModifier_Ctrl;
        case KeyCode::LeftAlt:
        case KeyCode::RightAlt:
            return KeyModifier_Alt;
        case KeyCode::LeftSuper:
        case KeyCode::RightSuper:
            return KeyModifier_Super;
        default:
            return KeyModifier_None;
        }
    }

    /** Build the modifier mask for a key event.
     *  XKeyEvent::state describes the modifiers as they were *before* the
     *  event, so pressing Shift would otherwise report Shift as still up for
     *  one event; fold the event's own key into the mask. */
    static uint32_t ModifiersForKeyEvent ( const XKeyEvent& aKeyEvent, KeyCode aKeyCode, bool aPressed )
    {
        uint32_t mods = TranslateX11Modifiers ( aKeyEvent.state );
        const uint32_t bit = ModifierBitFor ( aKeyCode );
        if ( aPressed )
        {
            mods |= bit;
        }
        else
        {
            mods &= ~bit;
        }
        return mods;
    }

    /** Decode the UTF-8 sequence starting at aIndex, advancing aIndex past it.
     *  Returns 0 for malformed input, having skipped the offending bytes. */
    static uint32_t DecodeUtf8 ( const char* aBytes, int aLength, int& aIndex )
    {
        const unsigned char lead = static_cast<unsigned char> ( aBytes[aIndex] );
        uint32_t codepoint{};
        int continuation_count{};
        if ( lead < 0x80 )
        {
            codepoint = lead;
        }
        else if ( ( lead & 0xE0 ) == 0xC0 )
        {
            codepoint = lead & 0x1Fu;
            continuation_count = 1;
        }
        else if ( ( lead & 0xF0 ) == 0xE0 )
        {
            codepoint = lead & 0x0Fu;
            continuation_count = 2;
        }
        else if ( ( lead & 0xF8 ) == 0xF0 )
        {
            codepoint = lead & 0x07u;
            continuation_count = 3;
        }
        else
        {
            ++aIndex;
            return 0;
        }
        if ( aIndex + continuation_count >= aLength )
        {
            aIndex = aLength;
            return 0;
        }
        for ( int i = 0; i < continuation_count; ++i )
        {
            const unsigned char continuation = static_cast<unsigned char> ( aBytes[aIndex + 1 + i] );
            if ( ( continuation & 0xC0 ) != 0x80 )
            {
                aIndex += 1 + i;
                return 0;
            }
            codepoint = ( codepoint << 6 ) | ( continuation & 0x3Fu );
        }
        aIndex += continuation_count + 1;
        return codepoint;
    }

    Window::Window ( const std::string& aRendererName, int32_t aX, int32_t aY, uint32_t aWidth, uint32_t aHeight,
                     bool aFullScreen, const RendererSettings& aRendererSettings ) :
        mDisplay{XOpenDisplay ( nullptr ) }
    {
        ::Window root = DefaultRootWindow ( mDisplay );
        XVisualInfo* xvi = glXChooseVisual ( mDisplay, DefaultScreen ( mDisplay ), choose_visual_attribs );
        if ( xvi == nullptr )
        {
            throw std::runtime_error ( "No supported visual reported by glXChooseVisual." );
        }
        mColorMap = XCreateColormap ( mDisplay, root, xvi->visual, AllocNone );
        XSetWindowAttributes swa
        {
            .background_pixmap = None,
            .background_pixel  = 0,
            .border_pixel      = 0,
            .event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask
            | ButtonPressMask | ButtonReleaseMask | PointerMotionMask
            | FocusChangeMask | ExposureMask,
            .colormap = mColorMap,
        };
        mWindowId = XCreateWindow (
                        mDisplay,
                        root,
                        aX, aY,
                        aWidth, aHeight,
                        0,
                        xvi->depth, InputOutput, xvi->visual, CWBackPixmap | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &swa
                    );
        XFree ( xvi );
        XStoreName ( mDisplay, mWindowId, "AeonGames" );
        // Without this, X11 auto-repeat delivers a synthetic KeyRelease before
        // every repeated KeyPress, so a held key reads as released for one
        // event and edge-triggered queries fire on every repeat.
        Bool detectable_auto_repeat_supported{False};
        XkbSetDetectableAutoRepeat ( mDisplay, True, &detectable_auto_repeat_supported );
        if ( !detectable_auto_repeat_supported )
        {
            std::cout << LogLevel::Warning
                      << "X server does not support detectable auto-repeat; held keys may flicker."
                      << std::endl;
        }
        XFlush ( mDisplay );
        // An input context turns keystrokes into UTF-8 text, handling dead
        // keys, compose sequences and IME preedit that raw KeySyms cannot.
        mInputMethod = XOpenIM ( mDisplay, nullptr, nullptr, nullptr );
        if ( mInputMethod != nullptr )
        {
            mInputContext = XCreateIC ( mInputMethod,
                                        XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                                        XNClientWindow, mWindowId,
                                        XNFocusWindow, mWindowId,
                                        nullptr );
        }
        if ( mInputContext != nullptr )
        {
            unsigned long filter_events{};
            if ( XGetICValues ( mInputContext, XNFilterEvents, &filter_events, nullptr ) == nullptr )
            {
                XSelectInput ( mDisplay, mWindowId, swa.event_mask | filter_events );
            }
        }
        else
        {
            std::cout << LogLevel::Warning
                      << "No X input context; text input is limited to ASCII."
                      << std::endl;
        }
        mRenderer = ConstructRenderer ( aRendererName, reinterpret_cast<void*> ( mWindowId ), aRendererSettings );
        EnumerateGuiOverlayConstructors ( [this] ( const StringId & aIdentifier ) -> bool
        {
            mGuiOverlay = ConstructGuiOverlay ( aIdentifier, reinterpret_cast<void*> ( mWindowId ) );
            return mGuiOverlay == nullptr;
        } );
        EnumerateInputSystemConstructors ( [this] ( const StringId & aIdentifier ) -> bool
        {
            mInputSystem = ConstructInputSystem ( aIdentifier );
            return mInputSystem == nullptr;
        } );
        SetInputSystem ( mInputSystem.get() );
    }

    Window::~Window()
    {
        SetInputSystem ( nullptr );
        if ( mRenderer )
        {
            mRenderer->DetachWindow ( this );
        }
        if ( mInputContext )
        {
            XDestroyIC ( mInputContext );
        }
        if ( mInputMethod )
        {
            XCloseIM ( mInputMethod );
        }
        if ( mWindowId )
        {
            XDestroyWindow ( mDisplay, mWindowId );
        }
        if ( mColorMap )
        {
            XFreeColormap ( mDisplay, mColorMap );
        }
        XCloseDisplay ( mDisplay );
        mDisplay = None;
    }

    uint32_t Window::Resize ( uint32_t aWidth, uint32_t aHeight )
    {
        if ( aWidth && aHeight && mRenderer )
        {
            mRenderer->ResizeViewport ( reinterpret_cast<void*> ( mWindowId ), 0, 0, aWidth, aHeight );
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
        XStoreName ( mDisplay, mWindowId, aTitle.c_str() );
    }

    void Window::Run ( Scene& aScene )
    {
        bool running{true};
        XEvent xevent;
        aScene.SetInputSystem ( mInputSystem.get() );
        Atom wm_delete_window = XInternAtom ( mDisplay, "WM_DELETE_WINDOW", 0 );
        XSetWMProtocols ( mDisplay, mWindowId, &wm_delete_window, 1 );
        std::chrono::high_resolution_clock::time_point last_time{std::chrono::high_resolution_clock::now() };

        // Build a 1x1 invisible cursor used while the cursor is captured.
        Cursor invisible_cursor = None;
        Pixmap blank = XCreateBitmapFromData ( mDisplay, mWindowId, "\0", 1, 1 );
        if ( blank != None )
        {
            XColor dummy{};
            invisible_cursor = XCreatePixmapCursor ( mDisplay, blank, blank, &dummy, &dummy, 0, 0 );
            XFreePixmap ( mDisplay, blank );
        }
        bool prev_cursor_captured = false;

        XMapWindow ( mDisplay, mWindowId );
        while ( running )
        {
            while ( ( XPending ( mDisplay ) > 0 ) && running )
            {
                XNextEvent ( mDisplay, &xevent );
                // Gives the input method first refusal: it swallows the
                // keystrokes that make up a dead key or compose sequence and
                // re-emits the composed character as a later KeyPress.
                if ( XFilterEvent ( &xevent, None ) )
                {
                    continue;
                }
                switch ( xevent.type )
                {
                case Expose:
                {
                    XWindowAttributes xwa;
                    XGetWindowAttributes ( mDisplay, mWindowId, &xwa );
                    Resize ( xwa.width, xwa.height );
                }
                break;
                case KeyPress:
                {
                    KeyCode key = TranslateX11KeyCode ( xevent.xkey.keycode );
                    if ( mInputSystem )
                    {
                        mInputSystem->SetKeyModifiers ( ModifiersForKeyEvent ( xevent.xkey, key, true ) );
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
                        // F2/F3/F4 toggle a whole light type on/off (debugging
                        // aid: e.g. disable point and spot lights to isolate the
                        // directional light's shadow).
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
                    // Translate to text for text input. Route through the GUI
                    // overlay first; only forward to the InputSystem if the
                    // overlay does not consume the codepoint.
                    char buffer[64] = {};
                    KeySym sym = NoSymbol;
                    int length{};
                    // Without an input context only Latin-1 is available, so
                    // each byte is already a codepoint; with one the buffer is
                    // UTF-8 and has to be decoded.
                    const bool utf8 = mInputContext != nullptr;
                    if ( utf8 )
                    {
                        Status lookup_status = XLookupNone;
                        length = Xutf8LookupString ( mInputContext, &xevent.xkey, buffer, sizeof ( buffer ) - 1, &sym, &lookup_status );
                        if ( lookup_status != XLookupChars && lookup_status != XLookupBoth )
                        {
                            length = 0;
                        }
                    }
                    else
                    {
                        length = XLookupString ( &xevent.xkey, buffer, sizeof ( buffer ) - 1, &sym, nullptr );
                    }
                    int index{};
                    while ( index < length )
                    {
                        uint32_t codepoint{};
                        if ( utf8 )
                        {
                            codepoint = DecodeUtf8 ( buffer, length, index );
                        }
                        else
                        {
                            codepoint = static_cast<unsigned char> ( buffer[index] );
                            ++index;
                        }
                        if ( codepoint < 0x20 || codepoint == 0x7F )
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
                case KeyRelease:
                {
                    KeyCode key = TranslateX11KeyCode ( xevent.xkey.keycode );
                    if ( mInputSystem )
                    {
                        mInputSystem->SetKeyModifiers ( ModifiersForKeyEvent ( xevent.xkey, key, false ) );
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
                case ButtonPress:
                {
                    // X11 wheel events arrive as button presses 4 (up), 5 (down),
                    // 6 (left), 7 (right). Translate these into mouse-wheel events.
                    if ( xevent.xbutton.button >= 4 && xevent.xbutton.button <= 7 )
                    {
                        float dx = 0.0f;
                        float dy = 0.0f;
                        switch ( xevent.xbutton.button )
                        {
                        case 4:
                            dy = 1.0f;
                            break;
                        case 5:
                            dy = -1.0f;
                            break;
                        case 6:
                            dx = -1.0f;
                            break;
                        case 7:
                            dx = 1.0f;
                            break;
                        }
                        bool consumed = mGuiOverlay && mGuiOverlay->OnMouseWheel ( dx, dy );
                        if ( !consumed && mInputSystem )
                        {
                            mInputSystem->OnMouseWheel ( dx, dy );
                        }
                    }
                    else
                    {
                        int32_t btn = TranslateX11Button ( xevent.xbutton.button );
                        if ( btn >= 0 )
                        {
                            bool consumed = mGuiOverlay && mGuiOverlay->OnMouseButton ( btn, true, xevent.xbutton.x, xevent.xbutton.y );
                            if ( !consumed && mInputSystem )
                            {
                                mInputSystem->OnMouseButton ( btn, true, xevent.xbutton.x, xevent.xbutton.y );
                            }
                        }
                    }
                }
                break;
                case ButtonRelease:
                {
                    // X11 emits a release for wheel events too; ignore those
                    // since wheel is delta-only.
                    if ( xevent.xbutton.button >= 4 && xevent.xbutton.button <= 7 )
                    {
                        break;
                    }
                    int32_t btn = TranslateX11Button ( xevent.xbutton.button );
                    if ( btn >= 0 )
                    {
                        bool consumed = mGuiOverlay && mGuiOverlay->OnMouseButton ( btn, false, xevent.xbutton.x, xevent.xbutton.y );
                        if ( !consumed && mInputSystem )
                        {
                            mInputSystem->OnMouseButton ( btn, false, xevent.xbutton.x, xevent.xbutton.y );
                        }
                    }
                }
                break;
                case MotionNotify:
                {
                    bool consumed = mGuiOverlay && mGuiOverlay->OnMouseMove ( xevent.xmotion.x, xevent.xmotion.y );
                    if ( !consumed && mInputSystem )
                    {
                        mInputSystem->OnMouseMove ( xevent.xmotion.x, xevent.xmotion.y );
                    }
                }
                break;
                case FocusIn:
                    if ( mInputContext )
                    {
                        XSetICFocus ( mInputContext );
                    }
                    if ( mInputSystem )
                    {
                        mInputSystem->OnFocusGained();
                    }
                    break;
                case FocusOut:
                    if ( mInputContext )
                    {
                        XUnsetICFocus ( mInputContext );
                    }
                    if ( mInputSystem )
                    {
                        mInputSystem->OnFocusLost();
                    }
                    break;
                case ClientMessage:
                    if ( static_cast<Atom> ( xevent.xclient.data.l[0] ) == wm_delete_window )
                    {
                        running = false;
                    }
                    break;
                case ConfigureNotify:
                    break;
                default:
                    std::cout << LogLevel::Info <<  "Received Event Type: " <<  xevent.type << std::endl;
                    break;
                }
            }
            std::chrono::high_resolution_clock::time_point current_time {std::chrono::high_resolution_clock::now() };
            std::chrono::duration<double> delta{std::chrono::duration_cast<std::chrono::duration<double >> ( current_time - last_time ) };
            if ( mInputSystem )
            {
                bool cursor_captured = mInputSystem->IsCursorCaptured() || mInputSystem->IsRelativeMouseMode();
                if ( cursor_captured != prev_cursor_captured )
                {
                    if ( cursor_captured )
                    {
                        if ( invisible_cursor != None )
                        {
                            XDefineCursor ( mDisplay, mWindowId, invisible_cursor );
                        }
                        XGrabPointer ( mDisplay, mWindowId, True,
                                       ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                                       GrabModeAsync, GrabModeAsync,
                                       mWindowId, None, CurrentTime );
                    }
                    else
                    {
                        XUngrabPointer ( mDisplay, CurrentTime );
                        XUndefineCursor ( mDisplay, mWindowId );
                    }
                    prev_cursor_captured = cursor_captured;
                }
            }
            // Scene update reads input state (key/button polling and mouse
            // deltas) for this frame. It must run BEFORE the relative-mouse
            // recentre and InputSystem::Update() below, otherwise the mouse
            // delta would be zeroed out before any component could read it
            // and edge-triggered key queries would always miss.
            aScene.Update ( delta.count() );
            last_time = current_time;
            if ( mRenderer )
            {
                // Wait for the previous frame to finish before overwriting the
                // per-frame uniform buffers. The view/projection matrices and
                // light buffers are single-buffered and host-visible, so they
                // must not be written while the previous frame's GPU work may
                // still be reading them. BeginFrame() waits on the frame fence;
                // doing it first closes a write-after-read race that shows up as
                // geometry flicker while the camera is moving. BeginFrame() is
                // idempotent, so the later BeginRender() reuses this recording.
                mRenderer->BeginFrame ( reinterpret_cast<void*> ( mWindowId ) );
                if ( aScene.GetCamera() )
                {
                    mRenderer->SetViewMatrix ( reinterpret_cast<void*> ( mWindowId ), aScene.GetViewMatrix() );
                    Matrix4x4 projection {};
                    projection.Perspective ( aScene.GetFieldOfView(), mAspectRatio, aScene.GetNear(), aScene.GetFar() );
                    mRenderer->SetProjectionMatrix ( reinterpret_cast<void*> ( mWindowId ), projection );
                }
                mRenderer->SetLights ( reinterpret_cast<void*> ( mWindowId ), aScene.GetFrameLights() );
                mRenderer->SetGlobals ( reinterpret_cast<void*> ( mWindowId ), aScene.GetGlobals() );
                if ( mGuiOverlay )
                {
                    mGuiOverlay->BeginFrame ( reinterpret_cast<void*> ( mWindowId ), delta.count() );
                    mGuiOverlay->EndFrame ( reinterpret_cast<void*> ( mWindowId ) );
                }
                // Compute skinning pre-pass: dispatch skinning before the render
                // pass begins so the skinned vertex buffers are ready for both
                // the depth and shading traversals.
                aScene.LoopTraverseDFSPreOrder ( [this] ( const Node & aNode )
                {
                    aNode.Skin ( *mRenderer, reinterpret_cast<void*> ( mWindowId ) );
                } );
                // Hand the whole frame to the renderer: it brackets
                // BeginRender/EndRender, builds the render queue from the window
                // frustum, runs the depth pre-pass and light culling when the
                // scene has a lighting pipeline, submits the shading pass and
                // composites the overlay.
                BeginScreenshotFrame ( reinterpret_cast<void*> ( mWindowId ) );
                mRenderer->RenderScene ( reinterpret_cast<void*> ( mWindowId ), aScene, mGuiOverlay.get() );
                EndScreenshotFrame ( reinterpret_cast<void*> ( mWindowId ) );
            }
            // End-of-frame input bookkeeping. Done after the scene has read
            // this frame's input so deltas/edges are valid during Update().
            if ( mInputSystem )
            {
                // In relative-mouse mode, recentre the pointer so the next
                // frame's delta is measured from the window centre and the
                // pointer never drifts off-window during a long look.
                if ( mInputSystem->IsRelativeMouseMode() )
                {
                    XWindowAttributes xwa{};
                    XGetWindowAttributes ( mDisplay, mWindowId, &xwa );
                    int cx = xwa.width / 2;
                    int cy = xwa.height / 2;
                    XWarpPointer ( mDisplay, None, mWindowId, 0, 0, 0, 0, cx, cy );
                    mInputSystem->OnMouseMove ( cx, cy );
                }
                // Snapshot current state as "previous" for next frame's
                // edge-trigger and delta queries.
                mInputSystem->Update();
            }
        }
        XUnmapWindow ( mDisplay, mWindowId );
        if ( prev_cursor_captured )
        {
            XUngrabPointer ( mDisplay, CurrentTime );
            XUndefineCursor ( mDisplay, mWindowId );
        }
        if ( invisible_cursor != None )
        {
            XFreeCursor ( mDisplay, invisible_cursor );
        }
    }
}
