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
#ifndef AEONGAMES_INPUTSYSTEM_H
#define AEONGAMES_INPUTSYSTEM_H

#include "aeongames/Platform.hpp"
#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace AeonGames
{
    class StringId;

    /** @brief Cross-platform mouse button identifiers.
     *
     * Front-ends MUST translate platform-native button codes into these
     * values before calling InputSystem::OnMouseButton, and back-ends
     * MUST interpret aButton in IsMouseButtonDown/Pressed/Released as
     * one of these identifiers.
     */
    enum MouseButton : int32_t
    {
        MouseButton_Left   = 0,
        MouseButton_Right  = 1,
        MouseButton_Middle = 2,
        MouseButton_X1     = 3,
        MouseButton_X2     = 4,
        MouseButton_Count  = 8
    };

    /** @brief Bitmask flags for keyboard modifier state. */
    enum KeyModifier : uint32_t
    {
        KeyModifier_None  = 0,
        KeyModifier_Shift = 1u << 0,
        KeyModifier_Ctrl  = 1u << 1,
        KeyModifier_Alt   = 1u << 2,
        KeyModifier_Super = 1u << 3
    };

    /** @brief Abstract interface for an input system back-end.
     *
     * Provides a polling-based API for keyboard and mouse state,
     * plus event-driven callbacks for input events.
     * Concrete implementations handle platform-specific input
     * (e.g., desktop keyboard/mouse, gamepad controllers).
     */
    class InputSystem
    {
    public:
        DLL virtual ~InputSystem() = 0;

        ///@name Keyboard
        ///@{
        /** Returns true if the given key scan-code is currently pressed. */
        virtual bool IsKeyDown ( uint32_t aScanCode ) const = 0;
        /** Returns true if the given key was pressed this frame (edge trigger). */
        virtual bool IsKeyPressed ( uint32_t aScanCode ) const = 0;
        /** Returns true if the given key was released this frame (edge trigger). */
        virtual bool IsKeyReleased ( uint32_t aScanCode ) const = 0;
        /** Returns the current keyboard modifier bitmask (see KeyModifier). */
        virtual uint32_t GetKeyModifiers() const = 0;
        ///@}

        ///@name Mouse
        ///@{
        /** Returns true if the given mouse button is currently pressed. */
        virtual bool IsMouseButtonDown ( int32_t aButton ) const = 0;
        /** Returns true if the given mouse button was pressed this frame (edge trigger). */
        virtual bool IsMouseButtonPressed ( int32_t aButton ) const = 0;
        /** Returns true if the given mouse button was released this frame (edge trigger). */
        virtual bool IsMouseButtonReleased ( int32_t aButton ) const = 0;
        /** Returns the current mouse X position in window coordinates. */
        virtual int32_t GetMouseX() const = 0;
        /** Returns the current mouse Y position in window coordinates. */
        virtual int32_t GetMouseY() const = 0;
        /** Returns the mouse X movement delta since the last frame. */
        virtual int32_t GetMouseDeltaX() const = 0;
        /** Returns the mouse Y movement delta since the last frame. */
        virtual int32_t GetMouseDeltaY() const = 0;
        /** Returns the vertical mouse-wheel delta accumulated since the last frame (in wheel notches). */
        virtual float GetMouseWheelDelta() const = 0;
        /** Returns the horizontal mouse-wheel delta accumulated since the last frame (in wheel notches). */
        virtual float GetMouseWheelDeltaH() const = 0;
        ///@}

        ///@name Cursor
        ///@{
        /** Requests that the cursor be confined to the window (and typically hidden).
         *  Front-ends should observe IsCursorCaptured() each frame and apply the
         *  platform-specific clip/hide calls. */
        virtual void SetCursorCaptured ( bool aCaptured ) = 0;
        /** Returns the current cursor-capture request state. */
        virtual bool IsCursorCaptured() const = 0;
        /** Requests relative-mouse mode (mouse deltas without absolute position).
         *  Front-ends should observe IsRelativeMouseMode() and warp the cursor
         *  to a fixed point each frame so deltas accumulate without the cursor
         *  leaving the window. */
        virtual void SetRelativeMouseMode ( bool aRelative ) = 0;
        /** Returns the current relative-mouse-mode request state. */
        virtual bool IsRelativeMouseMode() const = 0;
        ///@}

        ///@name Text Input
        ///@{
        /** Returns and clears the buffer of UTF-32 codepoints accumulated since
         *  the last call. Useful for text-entry widgets. */
        virtual std::vector<uint32_t> ConsumeTextInput() = 0;
        ///@}

        ///@name Gamepad (optional - default impl reports nothing connected)
        ///@{
        virtual bool IsGamepadConnected ( uint32_t aIndex ) const
        {
            ( void ) aIndex;
            return false;
        }
        virtual bool IsGamepadButtonDown ( uint32_t aIndex, uint32_t aButton ) const
        {
            ( void ) aIndex;
            ( void ) aButton;
            return false;
        }
        virtual float GetGamepadAxis ( uint32_t aIndex, uint32_t aAxis ) const
        {
            ( void ) aIndex;
            ( void ) aAxis;
            return 0.0f;
        }
        ///@}

        ///@name Event Injection
        ///@{
        /** Injects a key event into the input system.
         * @param aScanCode Key scan-code.
         * @param aPressed true if the key was pressed, false if released.
         */
        virtual void OnKeyEvent ( uint32_t aScanCode, bool aPressed ) = 0;
        /** Updates the current keyboard modifier bitmask. */
        virtual void SetKeyModifiers ( uint32_t aModifiers ) = 0;
        /** Injects a Unicode character (UTF-32 codepoint) for text input. */
        virtual void OnChar ( uint32_t aCodepoint ) = 0;
        /** Injects a mouse move event into the input system.
         * @param aX X coordinate in window space.
         * @param aY Y coordinate in window space.
         */
        virtual void OnMouseMove ( int32_t aX, int32_t aY ) = 0;
        /** Injects a mouse button event into the input system.
         * @param aButton Normalized button identifier (see MouseButton).
         * @param aPressed true if the button was pressed, false if released.
         * @param aX X coordinate in window space.
         * @param aY Y coordinate in window space.
         */
        virtual void OnMouseButton ( int32_t aButton, bool aPressed, int32_t aX, int32_t aY ) = 0;
        /** Injects a mouse-wheel event into the input system.
         * @param aDeltaX Horizontal wheel notches (positive = right).
         * @param aDeltaY Vertical wheel notches (positive = up / away from user).
         */
        virtual void OnMouseWheel ( float aDeltaX, float aDeltaY ) = 0;
        /** Notifies the input system that the window lost focus.
         *  Implementations should clear any latched key / button state to
         *  avoid keys getting "stuck" pressed when focus returns. */
        virtual void OnFocusLost() = 0;
        /** Notifies the input system that the window regained focus. */
        virtual void OnFocusGained() = 0;
        ///@}

        ///@name Frame Lifecycle
        ///@{
        /** Called once per frame to update internal state (e.g., edge-trigger detection, deltas). */
        virtual void Update() = 0;
        ///@}
    };

    /**@name Factory Functions */
    /*@{*/
    /** @brief Construct an InputSystem back-end for the given identifier.
     *  @param aIdentifier Identifier selecting the back-end implementation.
     *  @return A unique_ptr owning the newly created InputSystem, or nullptr on failure. */
    DLL std::unique_ptr<InputSystem> ConstructInputSystem ( const StringId& aIdentifier );
    /** Registers an InputSystem constructor for a specific identifier.*/
    DLL bool RegisterInputSystemConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<InputSystem>() >& aConstructor );
    /** Unregisters an InputSystem constructor for a specific identifier.*/
    DLL bool UnregisterInputSystemConstructor ( const StringId& aIdentifier );
    /** Enumerates InputSystem constructor identifiers via an enumerator functor.*/
    DLL void EnumerateInputSystemConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /** Returns the names of all registered InputSystem constructors. */
    DLL std::vector<std::string> GetInputSystemConstructorNames();
    /*@}*/
}
#endif
