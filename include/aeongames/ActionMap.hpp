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
#ifndef AEONGAMES_ACTIONMAP_H
#define AEONGAMES_ACTIONMAP_H

#include <cstdint>
#include <unordered_map>
#include <vector>
#include "aeongames/Platform.hpp"
#include "aeongames/StringId.hpp"

namespace AeonGames
{
    class InputSystem;

    /** @brief Maps named game actions to raw input bindings.
     *
     * Provides a layer of indirection between raw input (scan-codes,
     * mouse buttons) and game-level actions identified by StringId.
     * Supports multiple bindings per action and queries the current
     * InputSystem state each frame.
     */
    class ActionMap
    {
    public:
        /** @brief Type of raw input binding. */
        enum class BindingType : uint8_t
        {
            Key,         ///< Keyboard scan-code.
            MouseButton  ///< Mouse button index.
        };

        /** @brief A single raw input binding. */
        struct Binding
        {
            BindingType mType{BindingType::Key};
            uint32_t    mCode{0}; ///< Scan-code or mouse button index.
        };

        DLL ActionMap();
        DLL ~ActionMap();

        /** @brief Bind a raw input to a named action.
         *  @param aAction  The action identifier.
         *  @param aBinding The raw input binding.
         */
        DLL void Bind ( uint32_t aAction, const Binding& aBinding );

        /** @brief Remove all bindings for a named action.
         *  @param aAction The action identifier.
         */
        DLL void Unbind ( uint32_t aAction );

        /** @brief Remove a specific binding from a named action.
         *  @param aAction  The action identifier.
         *  @param aBinding The raw input binding to remove.
         */
        DLL void Unbind ( uint32_t aAction, const Binding& aBinding );

        /** @brief Remove all bindings. */
        DLL void Clear();

        /** @brief Check if an action is currently active (any bound input is held).
         *  @param aAction      The action identifier.
         *  @param aInputSystem The InputSystem to query.
         *  @return true if any binding for this action is active.
         */
        DLL bool IsActionDown ( uint32_t aAction, const InputSystem& aInputSystem ) const;

        /** @brief Check if an action was triggered this frame (edge-on).
         *  @param aAction      The action identifier.
         *  @param aInputSystem The InputSystem to query.
         *  @return true if any binding for this action was pressed this frame.
         */
        DLL bool IsActionPressed ( uint32_t aAction, const InputSystem& aInputSystem ) const;

        /** @brief Check if an action was released this frame (edge-off).
         *  @param aAction      The action identifier.
         *  @param aInputSystem The InputSystem to query.
         *  @return true if any binding for this action was released this frame.
         */
        DLL bool IsActionReleased ( uint32_t aAction, const InputSystem& aInputSystem ) const;

    private:
        std::unordered_map<uint32_t, std::vector<Binding>> mBindings;
    };
}
#endif
