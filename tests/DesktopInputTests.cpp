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

#include "aeongames/InputSystem.hpp"
#include "aeongames/StringId.hpp"
#include "gtest/gtest.h"

namespace AeonGames
{
    TEST ( DesktopInput, EventsRemainVisibleUntilFrameUpdate )
    {
        const StringId identifier{"Desktop"};
        std::unique_ptr<InputSystem> input = ConstructInputSystem ( identifier );
        ASSERT_NE ( input, nullptr );

        input->OnMouseMove ( 100, 100 );
        input->Update();

        input->OnKeyEvent ( KeyCode::W, true );
        input->SetKeyModifiers ( KeyModifier_Shift );
        input->OnMouseButton ( MouseButton_Right, true, 100, 100 );
        input->OnMouseDelta ( 6, -6 );
        input->OnMouseWheel ( -1.0f, 2.0f );
        input->OnChar ( 0x00E9 );

        EXPECT_TRUE ( input->IsKeyDown ( KeyCode::W ) );
        EXPECT_TRUE ( input->IsKeyPressed ( KeyCode::W ) );
        EXPECT_EQ ( input->GetKeyModifiers(), KeyModifier_Shift );
        EXPECT_TRUE ( input->IsMouseButtonPressed ( MouseButton_Right ) );
        EXPECT_EQ ( input->GetMouseX(), 100 );
        EXPECT_EQ ( input->GetMouseY(), 100 );
        EXPECT_EQ ( input->GetMouseDeltaX(), 6 );
        EXPECT_EQ ( input->GetMouseDeltaY(), -6 );
        EXPECT_FLOAT_EQ ( input->GetMouseWheelDeltaH(), -1.0f );
        EXPECT_FLOAT_EQ ( input->GetMouseWheelDelta(), 2.0f );
        EXPECT_EQ ( input->ConsumeTextInput(), std::vector<uint32_t> ( {0x00E9} ) );

        input->Update();

        EXPECT_FALSE ( input->IsKeyPressed ( KeyCode::W ) );
        EXPECT_FALSE ( input->IsMouseButtonPressed ( MouseButton_Right ) );
        EXPECT_EQ ( input->GetMouseDeltaX(), 0 );
        EXPECT_EQ ( input->GetMouseDeltaY(), 0 );
        EXPECT_FLOAT_EQ ( input->GetMouseWheelDeltaH(), 0.0f );
        EXPECT_FLOAT_EQ ( input->GetMouseWheelDelta(), 0.0f );

        input->OnFocusLost();
        EXPECT_FALSE ( input->IsKeyDown ( KeyCode::W ) );
        EXPECT_TRUE ( input->IsKeyReleased ( KeyCode::W ) );
        EXPECT_FALSE ( input->IsMouseButtonDown ( MouseButton_Right ) );
        EXPECT_TRUE ( input->IsMouseButtonReleased ( MouseButton_Right ) );
        EXPECT_EQ ( input->GetKeyModifiers(), KeyModifier_None );
    }
}
