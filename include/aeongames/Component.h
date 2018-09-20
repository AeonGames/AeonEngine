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
#include <any>
#include "aeongames/Platform.h"
#include "aeongames/Memory.h"
namespace google
{
    namespace protobuf
    {
        class Message;
    }
}
namespace AeonGames
{
    class Window;
    class Node;
    class Component
    {
    public:
        struct PropertyRecord
        {
            const char* Name{};
            std::type_index TypeIndex{typeid ( nullptr ) };
        };
        virtual const char* GetTypeName() const = 0;
        virtual uint32_t GetTypeId() const = 0;
        virtual std::vector<uint32_t> GetDependencies() const = 0;
        virtual void Update ( Node& aNode, double aDelta ) = 0;
        virtual void Render ( const Node& aNode, const Window& aWindow ) const = 0;
        /**@name Property Interface */
        /*@{*/
        /** Query available properties in the component.
            @param aPropertyCount is a pointer to a size_t related to the number of property records available or queried.
            @param aRecords Properties is either nullptr or a pointer to an array of PropertyRecord structures.
            @return true on success, false on error.
            @note If aRecords is nullptr, then the number of properties available is returned in aPropertyCount. Otherwise,
            aPropertyCount must point to a variable set by the user to the number of elements in the aRecords array,
            and on return the variable is overwritten with the number of structures actually written to aRecords.
         */
        virtual bool EnumerateProperties ( size_t* aPropertyCount, PropertyRecord* aRecords ) const = 0;
        /*@}*/
        /** If any changes to the properties message need to be processed or executed do so. */
        virtual void CommitPropertyChanges () = 0;
        virtual google::protobuf::Message* GetProperties() = 0;
        virtual const google::protobuf::Message* GetProperties() const = 0;
        DLL virtual ~Component() = 0;
    };
    /**@name Factory Functions */
    /*@{*/
    DLL std::unique_ptr<Component> ConstructComponent ( const std::string& aIdentifier );
    /** Registers a Component loader for a specific identifier.*/
    DLL bool RegisterComponentConstructor ( const std::string& aIdentifier, const std::function<std::unique_ptr<Component>() >& aConstructor );
    /** Unregisters a Component loader for a specific identifier.*/
    DLL bool UnregisterComponentConstructor ( const std::string& aIdentifier );
    /** Enumerates Component loader identifiers via an enumerator functor.*/
    DLL void EnumerateComponentConstructors ( const std::function<bool ( const std::string& ) >& aEnumerator );
    /*@}*/
}
#endif
