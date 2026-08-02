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

#include "aeongames/KeyCode.hpp"
#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( KeyCode, UsbHidUsageIds )
    {
        // Front-ends translate into USB HID usage page 0x07; a drifting value
        // would silently remap every binding.
        EXPECT_EQ ( static_cast<uint32_t> ( KeyCode::A ), 0x04u );
        EXPECT_EQ ( static_cast<uint32_t> ( KeyCode::Z ), 0x1Du );
        EXPECT_EQ ( static_cast<uint32_t> ( KeyCode::Num0 ), 0x27u );
        EXPECT_EQ ( static_cast<uint32_t> ( KeyCode::Escape ), 0x29u );
        EXPECT_EQ ( static_cast<uint32_t> ( KeyCode::Up ), 0x52u );
        EXPECT_EQ ( static_cast<uint32_t> ( KeyCode::LeftShift ), 0xE1u );
        EXPECT_EQ ( static_cast<uint32_t> ( KeyCode::RightSuper ), 0xE7u );
    }

    TEST ( KeyCode, NameRoundTrip )
    {
        EXPECT_EQ ( KeyCodeFromString ( KeyCodeToString ( KeyCode::W ) ), KeyCode::W );
        EXPECT_EQ ( KeyCodeFromString ( KeyCodeToString ( KeyCode::LeftShift ) ), KeyCode::LeftShift );
        EXPECT_EQ ( KeyCodeFromString ( KeyCodeToString ( KeyCode::Keypad0 ) ), KeyCode::Keypad0 );
        EXPECT_EQ ( KeyCodeFromString ( KeyCodeToString ( KeyCode::F24 ) ), KeyCode::F24 );
    }

    TEST ( KeyCode, NameLookupIgnoresCaseAndSpaces )
    {
        EXPECT_EQ ( KeyCodeFromString ( "left shift" ), KeyCode::LeftShift );
        EXPECT_EQ ( KeyCodeFromString ( "LeftShift" ), KeyCode::LeftShift );
        EXPECT_EQ ( KeyCodeFromString ( "  Left   Shift " ), KeyCode::LeftShift );
        EXPECT_EQ ( KeyCodeFromString ( "w" ), KeyCode::W );
    }

    TEST ( KeyCode, UnknownNames )
    {
        EXPECT_EQ ( KeyCodeFromString ( "Not A Key" ), KeyCode::Unknown );
        EXPECT_EQ ( KeyCodeFromString ( "" ), KeyCode::Unknown );
        EXPECT_EQ ( KeyCodeToString ( KeyCode::Unknown ), "Unknown" );
    }
}
