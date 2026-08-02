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
#include "aeongames/ActionMap.hpp"
#include "aeongames/InputSystem.hpp"

namespace AeonGames
{
    ActionMap::ActionMap() = default;
    ActionMap::~ActionMap() = default;

    void ActionMap::Bind ( uint32_t aAction, const Binding& aBinding )
    {
        mBindings[aAction].push_back ( aBinding );
    }

    void ActionMap::Unbind ( uint32_t aAction )
    {
        mBindings.erase ( aAction );
    }

    void ActionMap::Unbind ( uint32_t aAction, const Binding& aBinding )
    {
        auto it = mBindings.find ( aAction );
        if ( it != mBindings.end() )
        {
            auto& bindings = it->second;
            bindings.erase (
                std::remove_if ( bindings.begin(), bindings.end(),
                                 [&aBinding] ( const Binding & b )
            {
                if ( b.mType != aBinding.mType )
                {
                    return false;
                }
                return ( b.mType == BindingType::Key ) ?
                       b.mKeyCode == aBinding.mKeyCode :
                       b.mMouseButton == aBinding.mMouseButton;
            } ), bindings.end() );
            if ( bindings.empty() )
            {
                mBindings.erase ( it );
            }
        }
    }

    void ActionMap::Clear()
    {
        mBindings.clear();
    }

    static bool IsBindingDown ( const ActionMap::Binding& aBinding, const InputSystem& aInputSystem )
    {
        switch ( aBinding.mType )
        {
        case ActionMap::BindingType::Key:
            return aInputSystem.IsKeyDown ( aBinding.mKeyCode );
        case ActionMap::BindingType::MouseButton:
            return aInputSystem.IsMouseButtonDown ( aBinding.mMouseButton );
        }
        return false;
    }

    static bool IsBindingPressed ( const ActionMap::Binding& aBinding, const InputSystem& aInputSystem )
    {
        switch ( aBinding.mType )
        {
        case ActionMap::BindingType::Key:
            return aInputSystem.IsKeyPressed ( aBinding.mKeyCode );
        case ActionMap::BindingType::MouseButton:
            return aInputSystem.IsMouseButtonPressed ( aBinding.mMouseButton );
        }
        return false;
    }

    static bool IsBindingReleased ( const ActionMap::Binding& aBinding, const InputSystem& aInputSystem )
    {
        switch ( aBinding.mType )
        {
        case ActionMap::BindingType::Key:
            return aInputSystem.IsKeyReleased ( aBinding.mKeyCode );
        case ActionMap::BindingType::MouseButton:
            return aInputSystem.IsMouseButtonReleased ( aBinding.mMouseButton );
        }
        return false;
    }

    bool ActionMap::IsActionDown ( uint32_t aAction, const InputSystem& aInputSystem ) const
    {
        auto it = mBindings.find ( aAction );
        if ( it == mBindings.end() )
        {
            return false;
        }
        for ( const auto& binding : it->second )
        {
            if ( IsBindingDown ( binding, aInputSystem ) )
            {
                return true;
            }
        }
        return false;
    }

    bool ActionMap::IsActionPressed ( uint32_t aAction, const InputSystem& aInputSystem ) const
    {
        auto it = mBindings.find ( aAction );
        if ( it == mBindings.end() )
        {
            return false;
        }
        for ( const auto& binding : it->second )
        {
            if ( IsBindingPressed ( binding, aInputSystem ) )
            {
                return true;
            }
        }
        return false;
    }

    bool ActionMap::IsActionReleased ( uint32_t aAction, const InputSystem& aInputSystem ) const
    {
        auto it = mBindings.find ( aAction );
        if ( it == mBindings.end() )
        {
            return false;
        }
        for ( const auto& binding : it->second )
        {
            if ( IsBindingReleased ( binding, aInputSystem ) )
            {
                return true;
            }
        }
        return false;
    }
}
