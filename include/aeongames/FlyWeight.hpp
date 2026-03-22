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
#ifndef AEONGAMES_FLYWEIGHT_STORE_H
#define AEONGAMES_FLYWEIGHT_STORE_H

#include <unordered_map>
#include <mutex>

namespace AeonGames
{
    /** @brief Flyweight pattern implementation that stores shared objects keyed by a unique identifier.
     *  @tparam Key The type used as a unique identifier for stored objects.
     *  @tparam Value The derived type that inherits from FlyWeight.
     */
    template<class Key, class Value>
    class FlyWeight
    {
        friend Value;
        FlyWeight() = default;
        FlyWeight ( const FlyWeight& aFlyWeight ) : mKey{}
        {
            /* A Copy cannot have the same key as the original so do nothing*/
        }
        FlyWeight ( FlyWeight&& aFlyWeight ) : mKey ( std::move ( aFlyWeight.mKey ) )
        {
            static std::mutex m;
            std::lock_guard<std::mutex> hold ( m );
            // A Moved object's key is moved.
            aFlyWeight.mKey = Key{};
            mStore[mKey] = this;
        }
        const FlyWeight& operator= ( const FlyWeight& aFlyWeight )
        {
            /* A Copy cannot have the same key as the original*/
            if ( &aFlyWeight != this )
            {
                mKey = Key{};
            }
            return *this;
        }
        const FlyWeight& operator= ( FlyWeight&& aFlyWeight )
        {
            if ( &aFlyWeight != this )
            {
                static std::mutex m;
                std::lock_guard<std::mutex> hold ( m );
                // A Moved object's key is moved.
                aFlyWeight.mKey = Key{};
                mKey = std::move ( aFlyWeight.mKey );
                mStore[mKey] = this;
            }
            return *this;
        }
        FlyWeight ( const Key& aKey ) : mKey ( aKey )
        {
            Pack ( aKey );
        }
        virtual ~FlyWeight ()
        {
            Unpack();
        }
        Key mKey{};
        static std::unordered_map<Key, FlyWeight<Key, Value>*> mStore;
    public:
        /** @brief Lightweight handle providing read-only access to a packed FlyWeight object. */
        class Handle
        {
            const Key mKey{};
        public:
            /** @brief Construct a Handle for the given key.
             *  @param aKey The key identifying the FlyWeight object. Must not be the null/empty key.
             *  @throw std::runtime_error if aKey is the null/empty key.
             */
            Handle ( const Key& aKey ) : mKey{aKey}
            {
                if ( mKey == Key{} )
                {
                    throw std::runtime_error ( "The null/empty key is reserved for unpacked objects." );
                }
            }
            /** @brief Retrieve a pointer to the stored Value.
             *  @return Pointer to the Value if found, nullptr otherwise.
             */
            const Value* const Get() const
            {
                auto it = FlyWeight<Key, Value>::mStore.find ( mKey );
                if ( it != FlyWeight<Key, Value>::mStore.end() )
                {
                    return reinterpret_cast<Value*> ( it->second );
                }
                return nullptr;
            }
            /** @brief Get a copy of this Handle.
             *  @return A new Handle with the same key.
             */
            const Handle GetHandle() const
            {
                return Handle ( mKey );
            }
            /** @brief Check whether the referenced FlyWeight object is currently packed.
             *  @return true if the object exists in the store, false otherwise.
             */
            bool IsValid() const
            {
                return Get() != nullptr;
            }
            /** @brief Arrow operator for accessing the stored Value.
             *  @return Pointer to the Value.
             *  @throw std::runtime_error if the object is not packed.
             */
            const Value* const operator->() const
            {
                auto result = Get();
                if ( !result )
                {
                    throw std::runtime_error ( "Invalid FlyWeight Object." );
                }
                return result;
            }
            /** @brief Dereference operator for accessing the stored Value.
             *  @return Reference to the Value.
             *  @throw std::runtime_error if the object is not packed.
             */
            const Value& operator*() const
            {
                auto result = Get();
                if ( !result )
                {
                    throw std::runtime_error ( "Invalid FlyWeight Object." );
                }
                return *result;
            }
            /** @brief Address-of operator for obtaining a pointer to the stored Value.
             *  @return Pointer to the Value.
             *  @throw std::runtime_error if the object is not packed.
             */
            const Value* const operator&() const
            {
                auto result = Get();
                if ( !result )
                {
                    throw std::runtime_error ( "Invalid FlyWeight Object." );
                }
                return result;
            }
        };
        /** @brief Register this object in the store with the given key.
         *  @param aKey The unique key to associate with this object.
         *  @return A Handle to the packed object.
         *  @throw std::runtime_error if aKey is null/empty or already in use.
         */
        const Handle Pack ( const Key& aKey )
        {
            static std::mutex m;
            std::lock_guard<std::mutex> hold ( m );
            if ( aKey == Key{} )
            {
                throw std::runtime_error ( "The null/empty key is reserved for unpacked objects." );
            }
            mKey = aKey;
            Handle handle{mKey};
            if ( handle.Get() )
            {
                throw std::runtime_error ( "An object with the same key has already been packed." );
            }
            mStore[mKey] = this;
            return handle;
        }
        /** @brief Remove this object from the store and reset its key. */
        void Unpack()
        {
            static std::mutex m;
            std::lock_guard<std::mutex> hold ( m );
            mStore.erase ( mKey );
            mKey = Key{};
        }
        /** @brief Obtain a Handle to this packed object.
         *  @return A Handle for the current key.
         *  @throw std::runtime_error if the object is not packed.
         */
        const Handle GetHandle() const
        {
            if ( mKey == Key{} )
            {
                throw std::runtime_error ( "Object is not packed." );
            }
            return Handle{mKey};
        }
    };
    /// @brief Global store mapping keys to FlyWeight instances.
    template <class Key, class Value>
    std::unordered_map<Key, FlyWeight<Key, Value>*> FlyWeight<Key, Value>::mStore{};
}
#endif
