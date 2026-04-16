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
        ///@}

        ///@name Mouse
        ///@{
        /** Returns true if the given mouse button is currently pressed. */
        virtual bool IsMouseButtonDown ( int32_t aButton ) const = 0;
        /** Returns the current mouse X position in window coordinates. */
        virtual int32_t GetMouseX() const = 0;
        /** Returns the current mouse Y position in window coordinates. */
        virtual int32_t GetMouseY() const = 0;
        /** Returns the mouse X movement delta since the last frame. */
        virtual int32_t GetMouseDeltaX() const = 0;
        /** Returns the mouse Y movement delta since the last frame. */
        virtual int32_t GetMouseDeltaY() const = 0;
        ///@}

        ///@name Event Injection
        ///@{
        /** Injects a key event into the input system.
         * @param aScanCode Key scan-code.
         * @param aPressed true if the key was pressed, false if released.
         */
        virtual void OnKeyEvent ( uint32_t aScanCode, bool aPressed ) = 0;
        /** Injects a mouse move event into the input system.
         * @param aX X coordinate in window space.
         * @param aY Y coordinate in window space.
         */
        virtual void OnMouseMove ( int32_t aX, int32_t aY ) = 0;
        /** Injects a mouse button event into the input system.
         * @param aButton Button identifier.
         * @param aPressed true if the button was pressed, false if released.
         * @param aX X coordinate in window space.
         * @param aY Y coordinate in window space.
         */
        virtual void OnMouseButton ( int32_t aButton, bool aPressed, int32_t aX, int32_t aY ) = 0;
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
