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
#ifndef AEONGAMES_ARCHIVE_H
#define AEONGAMES_ARCHIVE_H

#include <unordered_map>
#include <memory>
#include <algorithm>
#include "UniqueAnyPtr.h"

namespace AeonGames
{
    template<class K, class T>
    class Archive
    {
    public:
        template <typename... Args>
        T* Store ( const K& k, Args... args )
        {
            mStorage.emplace ( std::make_pair<> ( k, std::make_unique<T> ( args... ) ) );
            return mStorage[k].get();
        }
        T* Store ( const K& k, std::unique_ptr<T>&& pointer )
        {
            mStorage.emplace ( std::make_pair<> ( k, std::move ( pointer ) ) );
            return mStorage[k].get();
        }
        std::unique_ptr<T> Dispose ( const K& k )
        {
            std::unique_ptr<T> result{};
            auto i = mStorage.find ( k );
            if ( i != mStorage.end() )
            {
                result = std::move ( ( *i ).second );
                mStorage.erase ( i );
            }
            return result;
        }
        const T* Get ( const K& k ) const
        {
            auto i = mStorage.find ( k );
            if ( i != mStorage.end() )
            {
                return ( *i ).second.get();
            }
            return nullptr;
        }
        T* Get ( const K& k )
        {
            return const_cast<T*> ( static_cast<const Archive<K, T>*> ( this )->Get ( k ) );
        }
        const K& GetKey ( const T* t ) const
        {
            auto i = std::find_if ( mStorage.begin(), mStorage.end(), [t] ( const std::pair<const K, std::unique_ptr<T>>& ref )
            {
                return t == ref.second.get();
            } );
            if ( i != mStorage.end() )
            {
                return ( ( *i ).first );
            }
            throw std::runtime_error ( "Key not found." );
        }
    private:
        std::unordered_map<K, std::unique_ptr<T>> mStorage{};
    };

    template<class K>
    class ArchiveAny
    {
    public:
        template <class T, typename... Args>
        T* Store ( const K& k, Args... args )
        {
            mStorage.emplace ( std::make_pair<> ( k, MakeUniqueAny<T> ( args... ) ) );
            return mStorage[k].template Get<T>();
        }

        template <class T>
        T* Store ( const K& k, std::unique_ptr<T>&& pointer )
        {
            mStorage.emplace ( std::make_pair<> ( k, std::move ( pointer ) ) );
            return mStorage[k].template Get<T>();
        }

        template <class T>
        T* Store ( const K& k, UniqueAnyPtr&& pointer )
        {
            mStorage.emplace ( std::make_pair<> ( k, std::move ( pointer ) ) );
            return mStorage[k].template Get<T>();
        }

        template <class T>
        std::unique_ptr<T> Dispose ( const K& k )
        {
            std::unique_ptr<T> result{};
            auto i = mStorage.find ( k );
            if ( i != mStorage.end() )
            {
                result = ( *i ).second.template UniquePointer<T>();
                mStorage.erase ( i );
            }
            return result;
        }

        template <class T>
        const T* Get ( const K& k ) const
        {
            auto i = mStorage.find ( k );
            if ( i != mStorage.end() )
            {
                return ( *i ).second.template Get<T>();
            }
            return nullptr;
        }

        template <class T>
        T* Get ( const K& k )
        {
            return const_cast<T*> ( static_cast<const ArchiveAny<K>*> ( this )->template Get<T> ( k ) );
        }

        const K& GetKey ( const void* t ) const
        {
            auto i = std::find_if ( mStorage.begin(), mStorage.end(), [t] ( const std::pair<const K, UniqueAnyPtr>& ref )
            {
                return t == ref.second.GetRaw();
            } );
            if ( i != mStorage.end() )
            {
                return ( ( *i ).first );
            }
            throw std::runtime_error ( "Key not found." );
        }
    private:
        std::unordered_map<K, UniqueAnyPtr> mStorage{};
    };
}
#endif
