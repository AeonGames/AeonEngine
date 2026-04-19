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
#include "DesktopInput.hpp"

namespace AeonGames
{
    DesktopInput::DesktopInput()
    {
        mKeyState.fill ( false );
        mPrevKeyState.fill ( false );
        mMouseButtonState.fill ( false );
        mPrevMouseButtonState.fill ( false );
    }

    DesktopInput::~DesktopInput() = default;

    bool DesktopInput::IsKeyDown ( uint32_t aScanCode ) const
    {
        if ( aScanCode >= MaxKeys )
        {
            return false;
        }
        return mKeyState[aScanCode];
    }

    bool DesktopInput::IsKeyPressed ( uint32_t aScanCode ) const
    {
        if ( aScanCode >= MaxKeys )
        {
            return false;
        }
        return mKeyState[aScanCode] && !mPrevKeyState[aScanCode];
    }

    bool DesktopInput::IsKeyReleased ( uint32_t aScanCode ) const
    {
        if ( aScanCode >= MaxKeys )
        {
            return false;
        }
        return !mKeyState[aScanCode] && mPrevKeyState[aScanCode];
    }

    uint32_t DesktopInput::GetKeyModifiers() const
    {
        return mKeyModifiers;
    }

    bool DesktopInput::IsMouseButtonDown ( int32_t aButton ) const
    {
        if ( aButton < 0 || static_cast<size_t> ( aButton ) >= MaxMouseButtons )
        {
            return false;
        }
        return mMouseButtonState[static_cast<size_t> ( aButton )];
    }

    bool DesktopInput::IsMouseButtonPressed ( int32_t aButton ) const
    {
        if ( aButton < 0 || static_cast<size_t> ( aButton ) >= MaxMouseButtons )
        {
            return false;
        }
        const size_t i = static_cast<size_t> ( aButton );
        return mMouseButtonState[i] && !mPrevMouseButtonState[i];
    }

    bool DesktopInput::IsMouseButtonReleased ( int32_t aButton ) const
    {
        if ( aButton < 0 || static_cast<size_t> ( aButton ) >= MaxMouseButtons )
        {
            return false;
        }
        const size_t i = static_cast<size_t> ( aButton );
        return !mMouseButtonState[i] && mPrevMouseButtonState[i];
    }

    int32_t DesktopInput::GetMouseX() const
    {
        return mMouseX;
    }

    int32_t DesktopInput::GetMouseY() const
    {
        return mMouseY;
    }

    int32_t DesktopInput::GetMouseDeltaX() const
    {
        return mMouseX - mPrevMouseX;
    }

    int32_t DesktopInput::GetMouseDeltaY() const
    {
        return mMouseY - mPrevMouseY;
    }

    float DesktopInput::GetMouseWheelDelta() const
    {
        return mWheelDeltaY;
    }

    float DesktopInput::GetMouseWheelDeltaH() const
    {
        return mWheelDeltaX;
    }

    void DesktopInput::SetCursorCaptured ( bool aCaptured )
    {
        mCursorCaptured = aCaptured;
    }

    bool DesktopInput::IsCursorCaptured() const
    {
        return mCursorCaptured;
    }

    void DesktopInput::SetRelativeMouseMode ( bool aRelative )
    {
        mRelativeMouseMode = aRelative;
    }

    bool DesktopInput::IsRelativeMouseMode() const
    {
        return mRelativeMouseMode;
    }

    std::vector<uint32_t> DesktopInput::ConsumeTextInput()
    {
        std::vector<uint32_t> out;
        out.swap ( mTextInput );
        return out;
    }

    void DesktopInput::OnKeyEvent ( uint32_t aScanCode, bool aPressed )
    {
        if ( aScanCode < MaxKeys )
        {
            mKeyState[aScanCode] = aPressed;
        }
    }

    void DesktopInput::SetKeyModifiers ( uint32_t aModifiers )
    {
        mKeyModifiers = aModifiers;
    }

    void DesktopInput::OnChar ( uint32_t aCodepoint )
    {
        mTextInput.push_back ( aCodepoint );
    }

    void DesktopInput::OnMouseMove ( int32_t aX, int32_t aY )
    {
        mMouseX = aX;
        mMouseY = aY;
    }

    void DesktopInput::OnMouseButton ( int32_t aButton, bool aPressed, int32_t aX, int32_t aY )
    {
        if ( aButton >= 0 && static_cast<size_t> ( aButton ) < MaxMouseButtons )
        {
            mMouseButtonState[static_cast<size_t> ( aButton )] = aPressed;
        }
        mMouseX = aX;
        mMouseY = aY;
    }

    void DesktopInput::OnMouseWheel ( float aDeltaX, float aDeltaY )
    {
        mWheelDeltaX += aDeltaX;
        mWheelDeltaY += aDeltaY;
    }

    void DesktopInput::OnFocusLost()
    {
        // Clear latched state so keys/buttons don't stay "pressed" when focus
        // returns. Edge-trigger Released queries will still fire next Update.
        mKeyState.fill ( false );
        mMouseButtonState.fill ( false );
        mKeyModifiers = KeyModifier_None;
        mTextInput.clear();
    }

    void DesktopInput::OnFocusGained()
    {
        // No-op for now; the platform layer will resync modifier state if needed.
    }

    void DesktopInput::Update()
    {
        mPrevKeyState = mKeyState;
        mPrevMouseButtonState = mMouseButtonState;
        mPrevMouseX = mMouseX;
        mPrevMouseY = mMouseY;
        mWheelDeltaX = 0.0f;
        mWheelDeltaY = 0.0f;
    }
}
