/*
Copyright (C) 2014-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Platform.hpp"
#include "aeongames/StringId.hpp"
#include "aeongames/Property.hpp"

namespace AeonGames
{
    class Node;
    class Window;
    class Renderer;
    /** @brief Abstract base class for node components.
     *
     * Components encapsulate reusable behavior and data that can be attached to
     * a Node. Concrete implementations provide domain-specific logic such as
     * rendering, physics, or audio.
     */
    class Component
    {
    public:
        /** @brief Pure virtual destructor. */
        DLL virtual ~Component() = 0;
        /** @brief Set a property by its StringId identifier.
         *  @param aId       Identifier of the property to set.
         *  @param aProperty Value to assign to the property.
         */
        DLL void SetProperty ( const StringId& aId, const Property& aProperty );
        /** @brief Set a property by its string name.
         *  @param aId       Name of the property to set.
         *  @param aProperty Value to assign to the property.
         */
        DLL void SetProperty ( const std::string& aId, const Property& aProperty );
        /** @brief Get the unique identifier for this component type.
         *  @return Reference to the component's StringId.
         */
        virtual const StringId& GetId() const = 0;
        /** @brief Get the number of properties exposed by this component.
         *  @return Property count.
         */
        virtual size_t GetPropertyCount () const = 0;
        /** @brief Get the array of property identifiers.
         *  @return Pointer to the first element of the property info array.
         */
        virtual const StringId* GetPropertyInfoArray () const = 0;
        /** @brief Get the value of a property.
         *  @param aId Identifier of the property to retrieve.
         *  @return The property value.
         */
        virtual Property GetProperty ( const StringId& aId ) const = 0;
        /**
         * Set the value aProperty for the property identified by aId.
         * @note If the type of the value passed does not match the expected types no change should be made.
        */
        virtual void SetProperty ( uint32_t aId, const Property& aProperty ) = 0;
        /** @brief Update the component state.
         *  @param aNode  Node this component is attached to.
         *  @param aDelta Elapsed time since the last update, in seconds.
         */
        virtual void Update ( Node& aNode, double aDelta ) = 0;
        /** @brief Render the component.
         *  @param aNode     Node this component is attached to.
         *  @param aRenderer Renderer used for drawing.
         *  @param aWindowId Platform-specific window identifier.
         */
        virtual void Render ( const Node& aNode, Renderer& aRenderer, void* aWindowId ) = 0;
        /** @brief Process an incoming message.
         *  @param aNode        Node this component is attached to.
         *  @param aMessageType Type identifier of the message.
         *  @param aMessageData Pointer to message-specific data.
         */
        virtual void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) = 0;
    };
    /**@name Factory Functions */
    /*@{*/
    /** @brief Construct a component from a numeric identifier.
     *  @param aIdentifier Numeric identifier of the component type.
     *  @return Owning pointer to the newly created component, or nullptr on failure.
     */
    DLL std::unique_ptr<Component> ConstructComponent ( uint32_t aIdentifier );
    /** @brief Construct a component from a string name.
     *  @param aIdentifier String name of the component type.
     *  @return Owning pointer to the newly created component, or nullptr on failure.
     */
    DLL std::unique_ptr<Component> ConstructComponent ( const std::string& aIdentifier );
    /** @brief Construct a component from a StringId.
     *  @param aIdentifier StringId of the component type.
     *  @return Owning pointer to the newly created component, or nullptr on failure.
     */
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
