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
#include <sstream>
#include <memory>
#include <typeinfo>
#include <algorithm>

namespace AeonGames
{
    class UniqueAnyPtr
    {
        void* mPointer{};
        const std::type_info& ( *mManager ) ( void* aPointer ) {};
    public:
        void Swap ( UniqueAnyPtr& aUniqueAnyPtr ) noexcept
        {
            std::swap ( mPointer, aUniqueAnyPtr.mPointer );
            std::swap ( mManager, aUniqueAnyPtr.mManager );
        }
        /** @name Creation and Destruction. */
        /**@{*/
        UniqueAnyPtr() noexcept = default;
        UniqueAnyPtr ( std::nullptr_t ) noexcept {};
        UniqueAnyPtr ( const UniqueAnyPtr& aUniqueResource ) = delete;
        UniqueAnyPtr& operator= ( const UniqueAnyPtr& aUniqueResource ) = delete;
        UniqueAnyPtr ( UniqueAnyPtr&& aUniqueAnyPtr ) noexcept
        {
            aUniqueAnyPtr.Swap ( *this );
        }

        template<class T>
        UniqueAnyPtr ( std::unique_ptr<T>&& aUniquePointer ) noexcept :
            mPointer{ aUniquePointer.release() }, mManager{Manager<T>}
        {}

        template<class T>
        UniqueAnyPtr ( T* aPointer ) noexcept :
            mPointer{aPointer}, mManager{Manager<T>}
        {}

        template<class T>
        UniqueAnyPtr ( T&& aValue ) noexcept :
            mPointer{new T ( std::move ( aValue ) ) }, mManager{Manager<T>}
        {}

        ~UniqueAnyPtr()
        {
            if ( mManager )
            {
                mManager ( mPointer );
            };
        }
        /**@}*/

        /** @name Assignment */
        /**@{*/
        UniqueAnyPtr& operator= ( UniqueAnyPtr&& aUniqueAnyPtr ) noexcept
        {
            aUniqueAnyPtr.Swap ( *this );
            return *this;
        }

        template<class T>
        UniqueAnyPtr& operator= ( std::unique_ptr<T>&& aUniquePointer ) noexcept
        {
            if ( mManager )
            {
                mManager ( mPointer );
            };
            mPointer = aUniquePointer.release();
            mManager = Manager<T>;
            return *this;
        }

        template<class T>
        UniqueAnyPtr& operator= ( T* aPointer ) noexcept
        {
            if ( mManager )
            {
                mManager ( mPointer );
            };
            mPointer = aPointer;
            mManager = Manager<T>;
            return *this;
        }
        /**@{*/

        const void* GetRaw() const
        {
            return mPointer;
        }

        template<class T> T* Get() const
        {
            if ( !mPointer )
            {
                return nullptr;
            }
            else if ( !HasType<T>() )
            {
                std::ostringstream stream;
                stream << "Unique Any Pointer of different type, Requested: " << typeid ( T ).name() << " Contained: " << GetTypeInfo().name();
                throw std::runtime_error ( stream.str().c_str() );
            }
            return reinterpret_cast<T*> ( mPointer );
        }

        template<class T> T* Get()
        {
            // EC++ Item 3
            return const_cast<T*> ( static_cast<const UniqueAnyPtr*> ( this )->Get<T>() );
        }

        void* ReleaseRaw()
        {
            void* pointer = mPointer;
            mPointer = nullptr;
            mManager = nullptr;
            return pointer;
        }

        template<class T> T* Release()
        {
            if ( !HasType<T>() )
            {
                std::ostringstream stream;
                stream << "Unique Any Pointer of different type, Requested: " << typeid ( T ).name() << " Contained: " << GetTypeInfo().name();
                throw std::runtime_error ( stream.str().c_str() );
            }
            return reinterpret_cast<T*> ( ReleaseRaw() );
        }

        template<class T> std::unique_ptr<T> UniquePointer()
        {
            return std::unique_ptr<T> ( Release<T>() );
        }

        void Reset()
        {
            UniqueAnyPtr unique_any_ptr{nullptr};
            unique_any_ptr.Swap ( *this );
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
