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
#ifndef AEONGAMES_COMPONENTPROPERTIES_HPP
#define AEONGAMES_COMPONENTPROPERTIES_HPP
#include <array>
#include <functional>
#include <type_traits>
#include <utility>
#include "aeongames/Component.hpp"

namespace AeonGames
{
    /** @brief Static property identifier and accessors for a concrete component.
     *  @tparam T The component type owning the property.
     */
    template<class T>
    struct ComponentPropertyDescriptor
    {
        StringId mId;
        Property ( *mGetter ) ( const T& );
        void ( *mSetter ) ( T&, const Property& );

        /** @brief Bind plain member storage with exact Property type checking.
         *  @tparam aMember Pointer to the component's data member.
         *  @param aId The property identifier.
         */
        template<auto aMember>
        static constexpr ComponentPropertyDescriptor Member ( const StringId& aId )
        {
            using Value = std::remove_cvref_t<std::invoke_result_t<decltype ( aMember ), const T&>>;
            return
            {
                aId,
                [] ( const T & aComponent ) -> Property
                {
                    return aComponent.*aMember;
                },
                [] ( T & aComponent, const Property & aProperty )
                {
                    if ( const Value * value = std::get_if<Value> ( &aProperty ) )
                    {
                        aComponent.*aMember = *value;
                    }
                }
            };
        }

        /** @brief Bind accessors, type-checking setters unless they explicitly accept Property.
         *  @tparam aGetter Getter callable accepting a const component.
         *  @tparam aSetter Setter callable accepting a component and its property value.
         *  @param aId The property identifier.
         */
        template<auto aGetter, auto aSetter>
        static constexpr ComponentPropertyDescriptor Create ( const StringId& aId )
        {
            using Value = std::remove_cvref_t<std::invoke_result_t<decltype ( aGetter ), const T&>>;
            return
            {
                aId,
                [] ( const T & aComponent ) -> Property
                {
                    return std::invoke ( aGetter, aComponent );
                },
                [] ( T & aComponent, const Property & aProperty )
                {
                    if constexpr ( std::is_invocable_v<decltype ( aSetter ), T&, const Property&> )
                    {
                        std::invoke ( aSetter, aComponent, aProperty );
                    }
                    else if ( const Value * value = std::get_if<Value> ( &aProperty ) )
                    {
                        std::invoke ( aSetter, aComponent, *value );
                    }
                }
            };
        }
    };

    /** @brief Header-only property dispatch through T::kPropertyDescriptors.
     *  @tparam T The concrete component, which grants this helper access to its table.
     */
    template<class T>
    class ComponentProperties : public Component
    {
    public:
        using Component::SetProperty;

        /** @name Component property overrides */
        ///@{
        size_t GetPropertyCount() const override
        {
            return T::kPropertyDescriptors.size();
        }

        const StringId* GetPropertyInfoArray() const override
        {
            if constexpr ( T::kPropertyDescriptors.empty() )
            {
                return nullptr;
            }
            else
            {
                static constexpr auto identifiers = []
                {
                    std::array<StringId, T::kPropertyDescriptors.size() > result{};
                    for ( size_t index = 0; index < result.size(); ++index )
                    {
                        result[index] = T::kPropertyDescriptors[index].mId;
                    }
                    return result;
                }();
                return identifiers.data();
            }
        }

        Property GetProperty ( const StringId& aId ) const override
        {
            for ( const auto& descriptor : T::kPropertyDescriptors )
            {
                if ( descriptor.mId == aId )
                {
                    return descriptor.mGetter ( static_cast<const T&> ( *this ) );
                }
            }
            return Property{};
        }

        void SetProperty ( uint32_t aId, const Property& aProperty ) override
        {
            for ( const auto& descriptor : T::kPropertyDescriptors )
            {
                if ( descriptor.mId == aId )
                {
                    descriptor.mSetter ( static_cast<T&> ( *this ), aProperty );
                    return;
                }
            }
        }
        ///@}
    };
}
#endif