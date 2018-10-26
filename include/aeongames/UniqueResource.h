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
#ifndef AEONGAMES_UNIQUERESOURCE_H
#include <memory>
#include <typeinfo>

namespace AeonGames
{
    class UniqueResource
    {
    public:
        UniqueResource() noexcept = default;
        UniqueResource ( const UniqueResource& aUniqueResource ) = delete;
        UniqueResource& operator= ( const UniqueResource& aUniqueResource ) = delete;
        UniqueResource ( UniqueResource&& aUniqueResource ) noexcept :
            mPointer{std::move ( aUniqueResource.mPointer ) },
                 mTypeId{std::move ( aUniqueResource.mTypeId ) },
                 mDeleter{std::move ( aUniqueResource.mDeleter ) }
        {
            aUniqueResource.mPointer = nullptr;
            aUniqueResource.mTypeId = nullptr;
            aUniqueResource.mDeleter = nullptr;
        }
        UniqueResource& operator= ( UniqueResource&& aUniqueResource ) noexcept
        {
            mPointer = std::move ( aUniqueResource.mPointer );
            mTypeId = std::move ( aUniqueResource.mTypeId );
            mDeleter = std::move ( aUniqueResource.mDeleter );
            aUniqueResource.mPointer = nullptr;
            aUniqueResource.mTypeId = nullptr;
            aUniqueResource.mDeleter = nullptr;
            return *this;
        }

        template<class T>
        UniqueResource ( std::unique_ptr<T>&& aUniquePointer ) noexcept :
            mPointer{aUniquePointer.release() },
                 mTypeId{TypeId<T>},
                 mDeleter{Deleter<T>}
        {}

        template<class T>
        UniqueResource& operator= ( std::unique_ptr<T>&& aUniquePointer ) noexcept
        {
            mPointer = aUniquePointer.release();
            mTypeId = TypeId<T>;
            mDeleter = Deleter<T>;
            return *this;
        }

        template<class T>
        UniqueResource ( T* aPointer ) noexcept :
            mPointer{aPointer},
                 mTypeId{TypeId<T>},
                 mDeleter{Deleter<T>}
        {}

        template<class T>
        UniqueResource& operator= ( T* aPointer ) noexcept
        {
            mPointer = aPointer;
            mTypeId = TypeId<T>;
            mDeleter = std::default_delete<T>();
            return *this;
        }

        ~UniqueResource()
        {
            if ( mDeleter )
            {
                mDeleter ( mPointer );
            };
        }

        template<class T> T* Get()
        {
            return reinterpret_cast<T*> ( mPointer );
        }

        template<class T> T* Release()
        {
            void* pointer = mPointer;
            mPointer = nullptr;
            mTypeId = nullptr;
            mDeleter = nullptr;
            return reinterpret_cast<T*> ( pointer );
        }

        template<class T> std::unique_ptr<T> UniquePointer()
        {
            return std::unique_ptr<T> ( Release<T>() );
        }

        void Reset()
        {
            if ( mDeleter )
            {
                mDeleter ( mPointer );
                mPointer = nullptr;
                mTypeId = nullptr;
                mDeleter = nullptr;
            }
        }

        const std::type_info& GetTypeInfo() const
        {
            if ( mTypeId )
            {
                return mTypeId();
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
        const std::type_info& ( *mTypeId ) () {};
        void ( *mDeleter ) ( void* aPointer ) {};
        template<typename T> static const std::type_info& TypeId()
        {
            return typeid ( T );
        }
        template<typename T> static void Deleter ( void* aPointer )
        {
            std::default_delete<T>() ( reinterpret_cast<T*> ( aPointer ) );
        }
    };
}
#endif
