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
#ifndef AEONGAMES_DESKTOPINPUT_HPP
#define AEONGAMES_DESKTOPINPUT_HPP

#include "aeongames/InputSystem.hpp"
#include <array>
#include <vector>

namespace AeonGames
{
    /** @brief Desktop keyboard and mouse input back-end.
     *
     * Tracks key and mouse-button state in fixed-size arrays,
     * providing both level (IsKeyDown) and edge (IsKeyPressed/Released) queries.
     */
    class DesktopInput final : public InputSystem
    {
    public:
        DesktopInput();
        ~DesktopInput() override;

        ///@name Keyboard
        ///@{
        bool IsKeyDown ( uint32_t aScanCode ) const override;
        bool IsKeyPressed ( uint32_t aScanCode ) const override;
        bool IsKeyReleased ( uint32_t aScanCode ) const override;
        uint32_t GetKeyModifiers() const override;
        ///@}

        ///@name Mouse
        ///@{
        bool IsMouseButtonDown ( int32_t aButton ) const override;
        bool IsMouseButtonPressed ( int32_t aButton ) const override;
        bool IsMouseButtonReleased ( int32_t aButton ) const override;
        int32_t GetMouseX() const override;
        int32_t GetMouseY() const override;
        int32_t GetMouseDeltaX() const override;
        int32_t GetMouseDeltaY() const override;
        float GetMouseWheelDelta() const override;
        float GetMouseWheelDeltaH() const override;
        ///@}

        ///@name Cursor
        ///@{
        void SetCursorCaptured ( bool aCaptured ) override;
        bool IsCursorCaptured() const override;
        void SetRelativeMouseMode ( bool aRelative ) override;
        bool IsRelativeMouseMode() const override;
        ///@}

        ///@name Text Input
        ///@{
        std::vector<uint32_t> ConsumeTextInput() override;
        ///@}

        ///@name Event Injection
        ///@{
        void OnKeyEvent ( uint32_t aScanCode, bool aPressed ) override;
        void SetKeyModifiers ( uint32_t aModifiers ) override;
        void OnChar ( uint32_t aCodepoint ) override;
        void OnMouseMove ( int32_t aX, int32_t aY ) override;
        void OnMouseButton ( int32_t aButton, bool aPressed, int32_t aX, int32_t aY ) override;
        void OnMouseWheel ( float aDeltaX, float aDeltaY ) override;
        void OnFocusLost() override;
        void OnFocusGained() override;
        ///@}

        ///@name Frame Lifecycle
        ///@{
        void Update() override;
        ///@}

    private:
        static constexpr size_t MaxKeys = 512;
        static constexpr size_t MaxMouseButtons = MouseButton_Count;

        std::array<bool, MaxKeys> mKeyState{};
        std::array<bool, MaxKeys> mPrevKeyState{};
        std::array<bool, MaxMouseButtons> mMouseButtonState{};
        std::array<bool, MaxMouseButtons> mPrevMouseButtonState{};

        int32_t mMouseX{};
        int32_t mMouseY{};
        int32_t mPrevMouseX{};
        int32_t mPrevMouseY{};

        float mWheelDeltaX{};
        float mWheelDeltaY{};

        uint32_t mKeyModifiers{};

        bool mCursorCaptured{};
        bool mRelativeMouseMode{};

        std::vector<uint32_t> mTextInput{};
    };
}
#endif
