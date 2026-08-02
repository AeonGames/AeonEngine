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
#include <algorithm>
#include <array>
#include <cctype>
#include "aeongames/KeyCode.hpp"

namespace AeonGames
{
    namespace
    {
        struct KeyCodeName
        {
            KeyCode mKeyCode;
            std::string_view mName;
        };

        constexpr std::array kKeyCodeNames
        {
            KeyCodeName{KeyCode::A, "A"},
            KeyCodeName{KeyCode::B, "B"},
            KeyCodeName{KeyCode::C, "C"},
            KeyCodeName{KeyCode::D, "D"},
            KeyCodeName{KeyCode::E, "E"},
            KeyCodeName{KeyCode::F, "F"},
            KeyCodeName{KeyCode::G, "G"},
            KeyCodeName{KeyCode::H, "H"},
            KeyCodeName{KeyCode::I, "I"},
            KeyCodeName{KeyCode::J, "J"},
            KeyCodeName{KeyCode::K, "K"},
            KeyCodeName{KeyCode::L, "L"},
            KeyCodeName{KeyCode::M, "M"},
            KeyCodeName{KeyCode::N, "N"},
            KeyCodeName{KeyCode::O, "O"},
            KeyCodeName{KeyCode::P, "P"},
            KeyCodeName{KeyCode::Q, "Q"},
            KeyCodeName{KeyCode::R, "R"},
            KeyCodeName{KeyCode::S, "S"},
            KeyCodeName{KeyCode::T, "T"},
            KeyCodeName{KeyCode::U, "U"},
            KeyCodeName{KeyCode::V, "V"},
            KeyCodeName{KeyCode::W, "W"},
            KeyCodeName{KeyCode::X, "X"},
            KeyCodeName{KeyCode::Y, "Y"},
            KeyCodeName{KeyCode::Z, "Z"},
            KeyCodeName{KeyCode::Num1, "1"},
            KeyCodeName{KeyCode::Num2, "2"},
            KeyCodeName{KeyCode::Num3, "3"},
            KeyCodeName{KeyCode::Num4, "4"},
            KeyCodeName{KeyCode::Num5, "5"},
            KeyCodeName{KeyCode::Num6, "6"},
            KeyCodeName{KeyCode::Num7, "7"},
            KeyCodeName{KeyCode::Num8, "8"},
            KeyCodeName{KeyCode::Num9, "9"},
            KeyCodeName{KeyCode::Num0, "0"},
            KeyCodeName{KeyCode::Enter, "Enter"},
            KeyCodeName{KeyCode::Escape, "Escape"},
            KeyCodeName{KeyCode::Backspace, "Backspace"},
            KeyCodeName{KeyCode::Tab, "Tab"},
            KeyCodeName{KeyCode::Space, "Space"},
            KeyCodeName{KeyCode::Minus, "Minus"},
            KeyCodeName{KeyCode::Equal, "Equal"},
            KeyCodeName{KeyCode::LeftBracket, "Left Bracket"},
            KeyCodeName{KeyCode::RightBracket, "Right Bracket"},
            KeyCodeName{KeyCode::Backslash, "Backslash"},
            KeyCodeName{KeyCode::NonUsHash, "Non US Hash"},
            KeyCodeName{KeyCode::Semicolon, "Semicolon"},
            KeyCodeName{KeyCode::Apostrophe, "Apostrophe"},
            KeyCodeName{KeyCode::Grave, "Grave"},
            KeyCodeName{KeyCode::Comma, "Comma"},
            KeyCodeName{KeyCode::Period, "Period"},
            KeyCodeName{KeyCode::Slash, "Slash"},
            KeyCodeName{KeyCode::CapsLock, "Caps Lock"},
            KeyCodeName{KeyCode::F1, "F1"},
            KeyCodeName{KeyCode::F2, "F2"},
            KeyCodeName{KeyCode::F3, "F3"},
            KeyCodeName{KeyCode::F4, "F4"},
            KeyCodeName{KeyCode::F5, "F5"},
            KeyCodeName{KeyCode::F6, "F6"},
            KeyCodeName{KeyCode::F7, "F7"},
            KeyCodeName{KeyCode::F8, "F8"},
            KeyCodeName{KeyCode::F9, "F9"},
            KeyCodeName{KeyCode::F10, "F10"},
            KeyCodeName{KeyCode::F11, "F11"},
            KeyCodeName{KeyCode::F12, "F12"},
            KeyCodeName{KeyCode::PrintScreen, "Print Screen"},
            KeyCodeName{KeyCode::ScrollLock, "Scroll Lock"},
            KeyCodeName{KeyCode::Pause, "Pause"},
            KeyCodeName{KeyCode::Insert, "Insert"},
            KeyCodeName{KeyCode::Home, "Home"},
            KeyCodeName{KeyCode::PageUp, "Page Up"},
            KeyCodeName{KeyCode::Delete, "Delete"},
            KeyCodeName{KeyCode::End, "End"},
            KeyCodeName{KeyCode::PageDown, "Page Down"},
            KeyCodeName{KeyCode::Right, "Right"},
            KeyCodeName{KeyCode::Left, "Left"},
            KeyCodeName{KeyCode::Down, "Down"},
            KeyCodeName{KeyCode::Up, "Up"},
            KeyCodeName{KeyCode::NumLock, "Num Lock"},
            KeyCodeName{KeyCode::KeypadDivide, "Keypad Divide"},
            KeyCodeName{KeyCode::KeypadMultiply, "Keypad Multiply"},
            KeyCodeName{KeyCode::KeypadSubtract, "Keypad Subtract"},
            KeyCodeName{KeyCode::KeypadAdd, "Keypad Add"},
            KeyCodeName{KeyCode::KeypadEnter, "Keypad Enter"},
            KeyCodeName{KeyCode::Keypad1, "Keypad 1"},
            KeyCodeName{KeyCode::Keypad2, "Keypad 2"},
            KeyCodeName{KeyCode::Keypad3, "Keypad 3"},
            KeyCodeName{KeyCode::Keypad4, "Keypad 4"},
            KeyCodeName{KeyCode::Keypad5, "Keypad 5"},
            KeyCodeName{KeyCode::Keypad6, "Keypad 6"},
            KeyCodeName{KeyCode::Keypad7, "Keypad 7"},
            KeyCodeName{KeyCode::Keypad8, "Keypad 8"},
            KeyCodeName{KeyCode::Keypad9, "Keypad 9"},
            KeyCodeName{KeyCode::Keypad0, "Keypad 0"},
            KeyCodeName{KeyCode::KeypadDecimal, "Keypad Decimal"},
            KeyCodeName{KeyCode::NonUsBackslash, "Non US Backslash"},
            KeyCodeName{KeyCode::Application, "Application"},
            KeyCodeName{KeyCode::Power, "Power"},
            KeyCodeName{KeyCode::KeypadEqual, "Keypad Equal"},
            KeyCodeName{KeyCode::F13, "F13"},
            KeyCodeName{KeyCode::F14, "F14"},
            KeyCodeName{KeyCode::F15, "F15"},
            KeyCodeName{KeyCode::F16, "F16"},
            KeyCodeName{KeyCode::F17, "F17"},
            KeyCodeName{KeyCode::F18, "F18"},
            KeyCodeName{KeyCode::F19, "F19"},
            KeyCodeName{KeyCode::F20, "F20"},
            KeyCodeName{KeyCode::F21, "F21"},
            KeyCodeName{KeyCode::F22, "F22"},
            KeyCodeName{KeyCode::F23, "F23"},
            KeyCodeName{KeyCode::F24, "F24"},
            KeyCodeName{KeyCode::Help, "Help"},
            KeyCodeName{KeyCode::Menu, "Menu"},
            KeyCodeName{KeyCode::Mute, "Mute"},
            KeyCodeName{KeyCode::VolumeUp, "Volume Up"},
            KeyCodeName{KeyCode::VolumeDown, "Volume Down"},
            KeyCodeName{KeyCode::KeypadComma, "Keypad Comma"},
            KeyCodeName{KeyCode::International1, "International 1"},
            KeyCodeName{KeyCode::International2, "International 2"},
            KeyCodeName{KeyCode::International3, "International 3"},
            KeyCodeName{KeyCode::International4, "International 4"},
            KeyCodeName{KeyCode::International5, "International 5"},
            KeyCodeName{KeyCode::Lang1, "Lang 1"},
            KeyCodeName{KeyCode::Lang2, "Lang 2"},
            KeyCodeName{KeyCode::LeftCtrl, "Left Ctrl"},
            KeyCodeName{KeyCode::LeftShift, "Left Shift"},
            KeyCodeName{KeyCode::LeftAlt, "Left Alt"},
            KeyCodeName{KeyCode::LeftSuper, "Left Super"},
            KeyCodeName{KeyCode::RightCtrl, "Right Ctrl"},
            KeyCodeName{KeyCode::RightShift, "Right Shift"},
            KeyCodeName{KeyCode::RightAlt, "Right Alt"},
            KeyCodeName{KeyCode::RightSuper, "Right Super"},
        };

        /// Compares two names ignoring ASCII case and spaces, so "Left Shift",
        /// "left shift" and "LeftShift" all denote the same key.
        bool NamesMatch ( std::string_view aLhs, std::string_view aRhs )
        {
            size_t lhs_index{};
            size_t rhs_index{};
            while ( true )
            {
                while ( lhs_index < aLhs.size() && aLhs[lhs_index] == ' ' )
                {
                    ++lhs_index;
                }
                while ( rhs_index < aRhs.size() && aRhs[rhs_index] == ' ' )
                {
                    ++rhs_index;
                }
                if ( lhs_index >= aLhs.size() || rhs_index >= aRhs.size() )
                {
                    return lhs_index >= aLhs.size() && rhs_index >= aRhs.size();
                }
                if ( std::tolower ( static_cast<unsigned char> ( aLhs[lhs_index] ) ) !=
                     std::tolower ( static_cast<unsigned char> ( aRhs[rhs_index] ) ) )
                {
                    return false;
                }
                ++lhs_index;
                ++rhs_index;
            }
        }
    }

    std::string_view KeyCodeToString ( KeyCode aKeyCode )
    {
        auto it = std::find_if ( kKeyCodeNames.begin(), kKeyCodeNames.end(),
                                 [aKeyCode] ( const KeyCodeName & aEntry )
        {
            return aEntry.mKeyCode == aKeyCode;
        } );
        return ( it != kKeyCodeNames.end() ) ? it->mName : std::string_view{"Unknown"};
    }

    KeyCode KeyCodeFromString ( std::string_view aName )
    {
        auto it = std::find_if ( kKeyCodeNames.begin(), kKeyCodeNames.end(),
                                 [aName] ( const KeyCodeName & aEntry )
        {
            return NamesMatch ( aEntry.mName, aName );
        } );
        return ( it != kKeyCodeNames.end() ) ? it->mKeyCode : KeyCode::Unknown;
    }
}
