/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_RESOURCECACHE_H
#define AEONGAMES_RESOURCECACHE_H
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <mutex>

namespace AeonGames
{
    template<class T>
    std::unique_ptr<T> Load ( const std::string& key )
    {
        return std::make_unique<T> ( key );
    }
    /** This was inspired by the fastLoadWidget
     * presented in Effective Modern C++ (EMC++20) by Scott Meyers,
     * which is in turn a version of Herb Sutter's
     * 'My Favorite C++ 10-liner' :
     * https://channel9.msdn.com/Events/GoingNative/2013/My-Favorite-Cpp-10-Liner
     * @param B Base class of the resource.
     * @param D Derived class of the resource (optional).
     * @return Shared pointer to the only instance of the resource in memory.
     * */
    template<class B, class D = B>
    std::shared_ptr<B> Get ( const std::string& key, std::unique_ptr<B> ( loader ) ( const std::string& ) = Load<D> )
    {
        ///@todo Maybe replace unordered_map with a vector.
        static std::unordered_map<std::string, std::weak_ptr<B>> cache;
        static std::mutex m;
        std::lock_guard<std::mutex> hold ( m );
        auto iter = cache.find ( key );
        if ( iter == cache.end() )
        {
            auto retval = std::shared_ptr<B> ( loader ( key ).release(), [key] ( B * t )
            {
                ///@todo Should this use the unique pointer deleter?
                delete ( t );
                cache.erase ( cache.find ( key ) );
            } );
            cache[key] = retval;
            return retval;
        }
        return iter->second.lock();
    }
}
#endif
