/*
Copyright (C) 2014-2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_COMPONENT_H
#define AEONGAMES_COMPONENT_H
#include <memory>
#include <functional>
#include <variant>
#include <string>
#include <type_traits>
#include "aeongames/Platform.h"
#include "aeongames/StringId.h"
#include "aeongames/Property.h"

namespace AeonGames
{
    class Node;
    class Window;
    class Component
    {
    public:
        DLL virtual ~Component() = 0;
        DLL void SetProperty ( const StringId& aId, const Property& aProperty );
        DLL void SetProperty ( const std::string& aId, const Property& aProperty );
        virtual const StringId& GetId() const = 0;
        virtual size_t GetPropertyCount () const = 0;
        virtual const StringId* GetPropertyInfoArray () const = 0;
        virtual Property GetProperty ( const StringId& aId ) const = 0;
        /**
         * Set the value aProperty for the property identified by aId.
         * @note If the type of the value passed does not match the expected types no change should be made.
        */
        virtual void SetProperty ( uint32_t aId, const Property& aProperty ) = 0;
        virtual void Update ( Node& aNode, double aDelta ) = 0;
        virtual void Render ( const Node& aNode, const Window& aWindow ) const = 0;
        virtual void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) = 0;
    };
    /**@name Factory Functions */
    /*@{*/
    DLL std::unique_ptr<Component> ConstructComponent ( uint32_t aIdentifier );
    DLL std::unique_ptr<Component> ConstructComponent ( const std::string& aIdentifier );
    DLL std::unique_ptr<Component> ConstructComponent ( const StringId& aIdentifier );
    /** Registers a Component loader for a specific identifier.*/
    DLL bool RegisterComponentConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<Component>() >& aConstructor );
    /** Unregisters a Component loader for a specific identifier.*/
    DLL bool UnregisterComponentConstructor ( const StringId& aIdentifier );
    /** Enumerates Component loader identifiers via an enumerator functor.*/
    DLL void EnumerateComponentConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /*@}*/
}
#endif
