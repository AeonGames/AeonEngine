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
#ifndef AEONGAMES_UNIQUEANYPTR_H
#define AEONGAMES_UNIQUEANYPTR_H
#include <memory>
#include <typeinfo>

namespace AeonGames
{
    class UniqueAnyPtr
    {
    public:
        UniqueAnyPtr() noexcept = default;
        UniqueAnyPtr ( const UniqueAnyPtr& aUniqueResource ) = delete;
        UniqueAnyPtr& operator= ( const UniqueAnyPtr& aUniqueResource ) = delete;
        UniqueAnyPtr ( UniqueAnyPtr&& aUniqueResource ) noexcept :
            mPointer{std::move ( aUniqueResource.mPointer ) },
                 mManager{std::move ( aUniqueResource.mManager ) }
        {
            aUniqueResource.mPointer = nullptr;
            aUniqueResource.mManager = nullptr;
        }
        UniqueAnyPtr& operator= ( UniqueAnyPtr&& aUniqueResource ) noexcept
        {
            mPointer = std::move ( aUniqueResource.mPointer );
            mManager = std::move ( aUniqueResource.mManager );
            aUniqueResource.mPointer = nullptr;
            aUniqueResource.mManager = nullptr;
            return *this;
        }

        template<class T>
        UniqueAnyPtr ( std::unique_ptr<T>&& aUniquePointer ) noexcept :
            mPointer{aUniquePointer.release() },
                 mManager{Manager<T>}
        {}

        template<class T>
        UniqueAnyPtr& operator= ( std::unique_ptr<T>&& aUniquePointer ) noexcept
        {
            mPointer = aUniquePointer.release();
            mManager = Manager<T>;
            return *this;
        }

        template<class T>
        UniqueAnyPtr ( T* aPointer ) noexcept :
            mPointer{aPointer},
                 mManager{Manager<T>}
        {}

        template<class T>
        UniqueAnyPtr& operator= ( T* aPointer ) noexcept
        {
            mPointer = aPointer;
            mManager = mManager;
            return *this;
        }

        ~UniqueAnyPtr()
        {
            if ( mManager )
            {
                mManager ( mPointer );
            };
        }

        template<class T> T* Get() const
        {
            if ( !HasType<T>() )
            {
                throw std::runtime_error ( "Unique resource of a different type" );
            }
            return reinterpret_cast<T*> ( mPointer );
        }
        template<class T> T* Get()
        {
            // EC++ Item 3
            return const_cast<T*> ( static_cast<const UniqueAnyPtr*> ( this )->Get<T>() );
        }

        template<class T> T* Release()
        {
            if ( !HasType<T>() )
            {
                throw std::runtime_error ( "Unique resource of a different type" );
            }
            void* pointer = mPointer;
            mPointer = nullptr;
            mManager = nullptr;
            return reinterpret_cast<T*> ( pointer );
        }

        template<class T> std::unique_ptr<T> UniquePointer()
        {
            return std::unique_ptr<T> ( Release<T>() );
        }

        void Reset()
        {
            if ( mManager )
            {
                mManager ( mPointer );
                mPointer = nullptr;
                mManager = nullptr;
            }
        }

        const std::type_info& GetTypeInfo() const
        {
            if ( mManager )
            {
                return mManager ( nullptr );
            }
            return typeid ( nullptr );
        }

        template<class T>
        bool HasType() const
        {
            return GetTypeInfo().hash_code() == typeid ( T ).hash_code();
        }

    private:
        void* mPointer{};
        const std::type_info& ( *mManager ) ( void* aPointer ) {};
        template<typename T> static const std::type_info& Manager ( void* aPointer )
        {
            if ( aPointer )
            {
                std::default_delete<T>() ( reinterpret_cast<T*> ( aPointer ) );
            }
            return typeid ( T );
        }
    };

    template<typename T, typename... Ts>
    UniqueAnyPtr MakeUniqueAny ( Ts&&... params )
    {
        return UniqueAnyPtr ( new T ( ::std::forward<Ts> ( params )... ) );
    }
}
#endif
