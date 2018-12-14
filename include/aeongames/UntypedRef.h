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
#ifndef AEONGAMES_UNTYPEDREF_H
#define AEONGAMES_UNTYPEDREF_H
#include <cstdint>
#include <array>
#include <typeinfo>
#include <iostream>
#include <functional>
#include <type_traits>

namespace AeonGames
{
    class UntypedRef
    {
    public:
        /// @name Construction/Copy/Destruction
        ///@{
        UntypedRef() = delete;
        UntypedRef ( std::nullptr_t ) noexcept {};
        template<typename T>
        UntypedRef ( T& aRef ) noexcept :
            mPointer{ const_cast<typename std::remove_const<T>::type*> ( std::addressof ( aRef ) ) },
        mTypeId{TypeId<T>} {}
        template<typename T>
        UntypedRef ( T&& ) = delete;
        UntypedRef ( const UntypedRef& ) noexcept = default;
        UntypedRef ( UntypedRef&& ) noexcept = default;
        ///@}
        /// @name Assignment
        ///@{
        UntypedRef& operator= ( const UntypedRef& ) noexcept = default;
        ///@}
        /// @name Access
        ///@{
        template<typename T>
        T& Get() const
        {
            if ( GetTypeInfo().hash_code() != typeid ( T ).hash_code() )
            {
                throw std::runtime_error ( "PropertyRef Invalid cast." );
            }
            return *reinterpret_cast<T*> ( mPointer );
        }
        template<typename T>
        void Set ( const T& aRef ) const
        {
            if ( GetTypeInfo().hash_code() != typeid ( T ).hash_code() )
            {
                throw std::runtime_error ( "PropertyRef Invalid cast." );
            }
            *reinterpret_cast<T*> ( mPointer ) = aRef;
        }
        ///@}

        template<typename T>
        bool HasType() const
        {
            return GetTypeInfo().hash_code() == typeid ( T ).hash_code();
        }

        const std::type_info& GetTypeInfo() const
        {
            return mTypeId();
        }

    private:
        void* mPointer{};
        const std::type_info& ( *mTypeId ) ()
        {
            TypeId<std::nullptr_t>
        };
        template<typename T> static const std::type_info& TypeId()
        {
            return typeid ( T );
        }
    };
}
#endif
