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
#include <algorithm>

namespace AeonGames
{
    DesktopInput::DesktopInput()
    {
        mKeyState.fill ( false );
        mPrevKeyState.fill ( false );
        mMouseButtonState.fill ( false );
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

    bool DesktopInput::IsMouseButtonDown ( int32_t aButton ) const
    {
        if ( aButton < 0 || static_cast<size_t> ( aButton ) >= MaxMouseButtons )
        {
            return false;
        }
        return mMouseButtonState[static_cast<size_t> ( aButton )];
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

    void DesktopInput::OnKeyEvent ( uint32_t aScanCode, bool aPressed )
    {
        if ( aScanCode < MaxKeys )
        {
            mKeyState[aScanCode] = aPressed;
        }
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

    void DesktopInput::Update()
    {
        mPrevKeyState = mKeyState;
        mPrevMouseX = mMouseX;
        mPrevMouseY = mMouseY;
    }
}
