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
#ifndef AEONGAMES_PROPERTY
#define AEONGAMES_PROPERTY
#include <cstdint>
#include <array>
#include <typeinfo>
#include <iostream>
#include <functional>

namespace AeonGames
{
    class PropertyRef
    {
    public:
        /// @name Construction/Copy/Destruction
        ///@{
        PropertyRef() = delete;
        template<typename T>
        PropertyRef ( const char* aName, T& aRef ) noexcept : mName{aName}, mPointer{std::addressof ( aRef ) }, mTypeId{TypeId<T>} {}
        template<typename T>
        PropertyRef ( T&& ) = delete;
        PropertyRef ( const PropertyRef& ) noexcept = default;
        ///@}
        /// @name Assignment
        ///@{
        PropertyRef& operator= ( const PropertyRef& ) noexcept = default;
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

        const char* GetName() const
        {
            return mName;
        }

    private:
        const char* mName{};
        void* mPointer;
        const std::type_info& ( *mTypeId ) () {};
        template<typename T> static const std::type_info& TypeId()
        {
            return typeid ( T );
        }
    };

    class TypedPointer
    {
    public:
        TypedPointer() = default;
        template<typename T> TypedPointer ( const T& t ) : mTypeId{TypeId<T>}, mPointer{&t} {}
        template<typename T> TypedPointer ( T&& t ) : mTypeId{TypeId<T>}, mPointer{&t} {}
        template<typename T> TypedPointer ( T* t ) : mTypeId{TypeId<T>}, mPointer{t} {}
        template<typename T> TypedPointer& operator= ( T* t )
        {
            mTypeId = TypeId<T>;
            mPointer = t;
            return *this;
        }
        template<typename T> const T* Get() const
        {
            if ( !mTypeId || !mPointer )
            {
                return nullptr;
            }
            if ( mTypeId().hash_code() != typeid ( T ).hash_code() )
            {
                throw std::runtime_error ( "Bad AeonGames::TypedPointer Cast." );
            }
            return reinterpret_cast<const T*> ( mPointer );
        }
        template<typename T> T* Get()
        {
            // EC++ Item 3
            return const_cast<T*> ( static_cast<const TypedPointer*> ( this )->Get<T>() );
        }
        const void* Get() const
        {
            return mPointer;
        }
        void* Get()
        {
            // EC++ Item 3
            return const_cast<void*> ( static_cast<const TypedPointer*> ( this )->Get() );
        }
        const std::type_info& GetTypeInfo() const
        {
            if ( !mTypeId || !mPointer )
            {
                return typeid ( nullptr );
            }
            return mTypeId();
        }
    private:
        const std::type_info& ( *mTypeId ) () {};
        void* mPointer{};
        template<typename T> static const std::type_info& TypeId()
        {
            return typeid ( T );
        }
    };

    template<std::size_t max_size>
    class Property
    {
        enum class Operation {Copy, Move, Assign, Retrieve, Destroy, TypeId};
    public:
        template<typename T> Property ( const char* aName, const T& t ) :
            mName{aName},
            mManager{Manager<T>}
        {
            static_assert ( sizeof ( T ) <= max_size, "Size of T is higher than max allocation size." );
            new ( mData.data() ) T{t};
        }

        template<typename T> Property ( const char* aName, T&& t ) :
            mName{aName},
            mManager{Manager<T>}
        {
            static_assert ( sizeof ( T ) <= max_size, "Size of T is higher than max allocation size." );
            new ( mData.data() ) T{t};
        }

        Property ( const Property& aProperty ) :
            mName{aProperty.mName},
            mManager{aProperty.mManager}
        {
            mManager ( Operation::Copy, mData.data(), aProperty.mData.data() );
        }

        Property ( Property&& aProperty ) :
            mName{aProperty.mName},
            mManager{aProperty.mManager}
        {
            mManager ( Operation::Move, mData.data(), aProperty.mData.data() );
        }

        void Set ( const TypedPointer& aValue )
        {
            if ( GetTypeInfo().hash_code() != aValue.GetTypeInfo().hash_code() )
            {
                throw std::runtime_error ( "AeonGames::Property::Set called with different types." );
            }
            mManager ( Operation::Assign, mData.data(), aValue.Get() );
        }

        const TypedPointer Get() const
        {
            TypedPointer pointer;
            mManager ( Operation::Retrieve, &pointer, mData.data() );
            return pointer;
        }

        TypedPointer Get()
        {
            // EC++ Item 3
            return static_cast<const Property&> ( *this ).Get();
        }
        ~Property()
        {
            mManager ( Operation::Destroy, mData.data(), nullptr );
        }
        template<typename T> bool HasType() const
        {
            return typeid ( T ).hash_code() == mManager ( Operation::TypeId, nullptr, nullptr ).hash_code();
        }
        const char* GetName() const
        {
            return mName;
        }
        const std::type_info& GetTypeInfo() const
        {
            return mManager ( Operation::TypeId, nullptr, nullptr );
        }
    private:
        const char* mName{};
        const std::type_info& ( *mManager ) ( Operation, void*, const void* ) {};
        std::array<uint8_t, max_size> mData{};
        template<typename T> static const std::type_info& Manager ( Operation op, void* dst, const void* src )
        {
            switch ( op )
            {
            case Operation::Copy:
                new ( dst ) T{*reinterpret_cast<const T*> ( src ) };
                break;
            case Operation::Move:
                new ( dst ) T{std::move ( *reinterpret_cast<const T*> ( src ) ) };
                break;
            case Operation::Assign:
                *reinterpret_cast<T*> ( dst ) = *reinterpret_cast<const T*> ( src );
                break;
            case Operation::Retrieve:
                *reinterpret_cast<TypedPointer*> ( dst ) = reinterpret_cast<T*> ( const_cast<void*> ( src ) );
                break;
            case Operation::Destroy:
                reinterpret_cast<T*> ( dst )->T::~T();
                break;
            case Operation::TypeId:
                break;
            }
            return typeid ( T );
        }
    };
}
#endif
