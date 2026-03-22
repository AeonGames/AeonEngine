/*
Copyright (C) 2018,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/UniqueAnyPtr.hpp"

namespace AeonGames
{
    /**
     * @brief Key-value archive that owns stored objects of type T, keyed by K.
     * @tparam K Key type (must be hashable).
     * @tparam T Value type.
     */
    template<class K, class T>
    class Archive
    {
    public:
        /**
         * @brief Construct and store a new object under the given key.
         * @tparam Args Constructor argument types.
         * @param k    Key to associate with the new object.
         * @param args Arguments forwarded to the T constructor.
         * @return Raw pointer to the stored object.
         */
        template <typename... Args>
        T* Store ( const K& k, Args... args )
        {
            mStorage.emplace ( std::make_pair<> ( k, std::make_unique<T> ( args... ) ) );
            return mStorage[k].get();
        }
        /**
         * @brief Store an existing uniquely-owned object under the given key.
         * @param k       Key to associate with the object.
         * @param pointer Unique pointer to the object to store (moved).
         * @return Raw pointer to the stored object.
         */
        T* Store ( const K& k, std::unique_ptr<T>&& pointer )
        {
            mStorage.emplace ( std::make_pair<> ( k, std::move ( pointer ) ) );
            return mStorage[k].get();
        }
        /**
         * @brief Remove and return the object associated with the given key.
         * @param k Key of the object to dispose.
         * @return Unique pointer to the removed object, or nullptr if not found.
         */
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
        /**
         * @brief Retrieve a stored object by key (const).
         * @param k Key to look up.
         * @return Pointer to the object, or nullptr if not found.
         */
        const T* Get ( const K& k ) const
        {
            auto i = mStorage.find ( k );
            if ( i != mStorage.end() )
            {
                return ( *i ).second.get();
            }
            return nullptr;
        }
        /**
         * @brief Retrieve a stored object by key (mutable).
         * @param k Key to look up.
         * @return Pointer to the object, or nullptr if not found.
         */
        T* Get ( const K& k )
        {
            return const_cast<T*> ( static_cast<const Archive<K, T>*> ( this )->Get ( k ) );
        }
        /**
         * @brief Find the key associated with a stored object.
         * @param t Pointer to the object to look up.
         * @return Const reference to the key.
         * @throw std::runtime_error if the object is not found.
         */
        const K& GetKey ( const T* t ) const
        {
            auto i = std::find_if ( mStorage.begin(), mStorage.end(), [t] ( const std::pair<const K, std::unique_ptr<T >> & ref )
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
        std::unordered_map<K, std::unique_ptr<T >> mStorage{};
    };

    /**
     * @brief Type-erased key-value archive that stores UniqueAnyPtr values.
     * @tparam K Key type (must be hashable).
     */
    template<class K>
    class ArchiveAny
    {
    public:
        /**
         * @brief Store a type-erased pointer under the given key.
         * @param k       Key to associate with the pointer.
         * @param pointer UniqueAnyPtr to store (moved).
         * @return Const reference to the stored pointer.
         */
        const UniqueAnyPtr& Store ( const K& k, UniqueAnyPtr&& pointer )
        {
            mStorage.emplace ( std::make_pair<> ( k, std::move ( pointer ) ) );
            return mStorage[k];
        }

        /**
         * @brief Remove and return the pointer associated with the given key.
         * @param k Key of the entry to dispose.
         * @return The removed UniqueAnyPtr, or a null pointer if not found.
         */
        UniqueAnyPtr Dispose ( const K& k )
        {
            UniqueAnyPtr result{};
            auto i = mStorage.find ( k );
            if ( i != mStorage.end() )
            {
                result.Swap ( ( *i ).second );
                mStorage.erase ( i );
            }
            return result;
        }

        /**
         * @brief Retrieve a stored pointer by key (const).
         * @param k Key to look up.
         * @return Const reference to the stored UniqueAnyPtr, or a null pointer if not found.
         */
        const UniqueAnyPtr& Get ( const K& k ) const
        {
            static const UniqueAnyPtr unique_nullptr{nullptr};
            auto i = mStorage.find ( k );
            if ( i != mStorage.end() )
            {
                return ( *i ).second;
            }
            return unique_nullptr;
        }

        /**
         * @brief Retrieve a stored pointer by key (mutable).
         * @param k Key to look up.
         * @return Reference to the stored UniqueAnyPtr, or a null pointer if not found.
         */
        const UniqueAnyPtr& Get ( const K& k )
        {
            return const_cast<UniqueAnyPtr&> ( static_cast<const ArchiveAny<K>*> ( this )->Get ( k ) );
        }

        /**
         * @brief Find the key associated with a stored raw pointer.
         * @param t Raw pointer to look up.
         * @return Const reference to the key.
         * @throw std::runtime_error if the pointer is not found.
         */
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
