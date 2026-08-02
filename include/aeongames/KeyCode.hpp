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
#ifndef AEONGAMES_KEYCODE_H
#define AEONGAMES_KEYCODE_H

#include <cstdint>
#include <string_view>
#include "aeongames/Platform.hpp"

namespace AeonGames
{
    /** @brief Engine wide physical key identifier.
     *
     * Values are USB HID Keyboard/Keypad usage IDs (usage page 0x07), so a
     * KeyCode names a key by its *position*, not by the character the active
     * layout prints on it: KeyCode::W is the key above the home row's second
     * column whether the layout calls it W (QWERTY) or Z (AZERTY). This is
     * what movement bindings such as WASD want; text entry is served by
     * InputSystem::OnChar instead, which carries layout translated codepoints.
     *
     * Platform front-ends translate native key events into these values
     * before calling InputSystem::OnKeyEvent, so bindings stay portable.
     */
    enum class KeyCode : uint32_t
    {
        Unknown = 0x00,

        A = 0x04,
        B = 0x05,
        C = 0x06,
        D = 0x07,
        E = 0x08,
        F = 0x09,
        G = 0x0A,
        H = 0x0B,
        I = 0x0C,
        J = 0x0D,
        K = 0x0E,
        L = 0x0F,
        M = 0x10,
        N = 0x11,
        O = 0x12,
        P = 0x13,
        Q = 0x14,
        R = 0x15,
        S = 0x16,
        T = 0x17,
        U = 0x18,
        V = 0x19,
        W = 0x1A,
        X = 0x1B,
        Y = 0x1C,
        Z = 0x1D,

        Num1 = 0x1E,
        Num2 = 0x1F,
        Num3 = 0x20,
        Num4 = 0x21,
        Num5 = 0x22,
        Num6 = 0x23,
        Num7 = 0x24,
        Num8 = 0x25,
        Num9 = 0x26,
        Num0 = 0x27,

        Enter = 0x28,
        Escape = 0x29,
        Backspace = 0x2A,
        Tab = 0x2B,
        Space = 0x2C,
        Minus = 0x2D,
        Equal = 0x2E,
        LeftBracket = 0x2F,
        RightBracket = 0x30,
        Backslash = 0x31,
        NonUsHash = 0x32,
        Semicolon = 0x33,
        Apostrophe = 0x34,
        Grave = 0x35,
        Comma = 0x36,
        Period = 0x37,
        Slash = 0x38,
        CapsLock = 0x39,

        F1 = 0x3A,
        F2 = 0x3B,
        F3 = 0x3C,
        F4 = 0x3D,
        F5 = 0x3E,
        F6 = 0x3F,
        F7 = 0x40,
        F8 = 0x41,
        F9 = 0x42,
        F10 = 0x43,
        F11 = 0x44,
        F12 = 0x45,

        PrintScreen = 0x46,
        ScrollLock = 0x47,
        Pause = 0x48,
        Insert = 0x49,
        Home = 0x4A,
        PageUp = 0x4B,
        Delete = 0x4C,
        End = 0x4D,
        PageDown = 0x4E,
        Right = 0x4F,
        Left = 0x50,
        Down = 0x51,
        Up = 0x52,

        NumLock = 0x53,
        KeypadDivide = 0x54,
        KeypadMultiply = 0x55,
        KeypadSubtract = 0x56,
        KeypadAdd = 0x57,
        KeypadEnter = 0x58,
        Keypad1 = 0x59,
        Keypad2 = 0x5A,
        Keypad3 = 0x5B,
        Keypad4 = 0x5C,
        Keypad5 = 0x5D,
        Keypad6 = 0x5E,
        Keypad7 = 0x5F,
        Keypad8 = 0x60,
        Keypad9 = 0x61,
        Keypad0 = 0x62,
        KeypadDecimal = 0x63,

        NonUsBackslash = 0x64,
        Application = 0x65,
        Power = 0x66,
        KeypadEqual = 0x67,

        F13 = 0x68,
        F14 = 0x69,
        F15 = 0x6A,
        F16 = 0x6B,
        F17 = 0x6C,
        F18 = 0x6D,
        F19 = 0x6E,
        F20 = 0x6F,
        F21 = 0x70,
        F22 = 0x71,
        F23 = 0x72,
        F24 = 0x73,

        Help = 0x75,
        Menu = 0x76,

        Mute = 0x7F,
        VolumeUp = 0x80,
        VolumeDown = 0x81,

        KeypadComma = 0x85,

        International1 = 0x87,
        International2 = 0x88,
        International3 = 0x89,
        International4 = 0x8A,
        International5 = 0x8B,

        Lang1 = 0x90,
        Lang2 = 0x91,

        LeftCtrl = 0xE0,
        LeftShift = 0xE1,
        LeftAlt = 0xE2,
        LeftSuper = 0xE3,
        RightCtrl = 0xE4,
        RightShift = 0xE5,
        RightAlt = 0xE6,
        RightSuper = 0xE7,

        /// One past the highest usable usage ID; sizes key state arrays.
        Count = 0x100
    };

    /** @brief Returns the canonical name for a key code.
     *  @param aKeyCode The key code to name.
     *  @return A stable, human readable name suitable for configuration files
     *          ("W", "Left Shift", "Keypad 0"), or "Unknown" if unnamed.
     */
    DLL std::string_view KeyCodeToString ( KeyCode aKeyCode );

    /** @brief Returns the key code for a canonical name.
     *  @param aName A name as produced by KeyCodeToString. Matching is case
     *               insensitive and ignores spaces, so "leftshift" and
     *               "Left Shift" both resolve.
     *  @return The matching key code, or KeyCode::Unknown if the name is not
     *          recognized.
     */
    DLL KeyCode KeyCodeFromString ( std::string_view aName );
}
#endif
