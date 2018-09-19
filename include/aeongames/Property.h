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
namespace AeonGames
{
    template<std::size_t max_size>
    class Property
    {
        enum class Operation {Copy, Move, Destroy, TypeId};
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

        template<typename T> void Set ( const T& t )
        {
            ( *reinterpret_cast<T*> ( mData.data() ) ) = t;
        }

        template<typename T> const T& Get() const
        {
            return *reinterpret_cast<const T*> ( mData.data() );
        }

        template<typename T> T& Get()
        {
            // EC++ Item 3
            return const_cast<T&> ( static_cast<const Property&> ( *this ).Get<T>() );
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
