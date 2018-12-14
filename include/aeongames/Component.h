/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#include <cstdint>
#include <vector>
#include <functional>
#include <typeinfo>
#include <typeindex>
#include "aeongames/Platform.h"
#include <memory>

namespace AeonGames
{
    class StringId;
    class Window;
    class Node;
    class Component
    {
    public:
        virtual const char* GetTypeName() const = 0;
        virtual uint32_t GetTypeId() const = 0;
        virtual std::vector<uint32_t> GetDependencies() const = 0;
        virtual void Update ( Node& aNode, double aDelta ) = 0;
        virtual void Render ( const Node& aNode, const Window& aWindow ) const = 0;
#if 0
        /**@name Property Interface */
        /*@{*/
        /** Retrieve the list of properties of the component.*/
        virtual std::vector<PropertyRef> GetProperties() const = 0;
        /*@}*/
#endif
        DLL virtual ~Component() = 0;
    };
    /**@name Factory Functions */
    /*@{*/
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
