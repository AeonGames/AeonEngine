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
#ifndef AEONGAMES_FLYWEIGHT_STORE_H
#define AEONGAMES_FLYWEIGHT_STORE_H

#include <unordered_map>
#include <mutex>

namespace AeonGames
{
    template<class Key, class Value>
    class FlyWeight
    {
        friend Value;
        FlyWeight() = default;
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
        class Handle
        {
            const Key mKey{};
        public:
            Handle ( const Key& aKey ) : mKey{aKey}
            {
                if ( mKey == Key{} )
                {
                    throw std::runtime_error ( "The null/empty key is reserved for unpacked objects." );
                }
            }
            const Value* const Get() const
            {
                auto it = FlyWeight<Key, Value>::mStore.find ( mKey );
                if ( it != FlyWeight<Key, Value>::mStore.end() )
                {
                    return reinterpret_cast<Value*> ( it->second );
                }
                return nullptr;
            }
            const Handle GetHandle() const
            {
                return Handle ( mKey );
            }
            bool IsValid() const
            {
                return Get() != nullptr;
            }
            const Value* const operator->() const
            {
                auto result = Get();
                if ( !result )
                {
                    throw std::runtime_error ( "Invalid FlyWeight Object." );
                }
                return result;
            }
        };
        const Handle Pack ( const Key& aKey )
        {
            if ( aKey == Key{} )
            {
                throw std::runtime_error ( "The null/empty key is reserved for unpacked objects." );
            }
            mKey = aKey;
            static std::mutex m;
            std::lock_guard<std::mutex> hold ( m );
            mStore[mKey] = this;
            return Handle{mKey};
        }
        void Unpack()
        {
            static std::mutex m;
            std::lock_guard<std::mutex> hold ( m );
            mStore.erase ( mKey );
            mKey = Key{};
        }
    };
    template <class Key, class Value>
    std::unordered_map<Key, FlyWeight<Key, Value>*> FlyWeight<Key, Value>::mStore{};
}
#endif
