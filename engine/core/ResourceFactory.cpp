/*
Copyright (C) 2018,2022 Rodrigo Jose Hernandez Cordoba

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
#include <tuple>
#include <sstream>
#include <exception>
#include <unordered_map>
#include "aeongames/ResourceFactory.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/ResourceId.h"

namespace AeonGames
{
    static std::unordered_map < uint32_t, std::tuple<std::function < UniqueAnyPtr ( uint32_t ) >, UniqueAnyPtr>> Constructors;

    UniqueAnyPtr ConstructResource ( const ResourceId& aResourceId )
    {
        auto it = Constructors.find ( aResourceId.GetType() );
        if ( it != Constructors.end() )
        {
            return std::get<0> ( it->second ) ( aResourceId.GetPath() );
        }
        std::ostringstream stream;
        stream << "No constructor registered for type " << aResourceId.GetType();
        throw std::runtime_error ( stream.str().c_str() );
    }
    bool RegisterResourceConstructor ( uint32_t aType, const std::function < UniqueAnyPtr ( uint32_t ) > & aConstructor, UniqueAnyPtr&& aDefaultResource )
    {
        if ( Constructors.find ( aType ) == Constructors.end() )
        {
            Constructors[aType] = std::make_pair ( aConstructor, std::move ( aDefaultResource ) );
            return true;
        }
        return false;
    }
    bool UnregisterResourceConstructor ( uint32_t aType )
    {
        auto it = Constructors.find ( aType );
        if ( it != Constructors.end() )
        {
            Constructors.erase ( it );
            return true;
        }
        return false;
    }
    void EnumerateResourceConstructors ( const std::function<bool ( uint32_t ) >& aEnumerator )
    {
        for ( auto& i : Constructors )
        {
            if ( !aEnumerator ( i.first ) )
            {
                return;
            }
        }
    }

    const UniqueAnyPtr& GetDefaultResource ( uint32_t aType )
    {
        static const UniqueAnyPtr unique_any_nullptr{nullptr};
        auto it = Constructors.find ( aType );
        if ( it != Constructors.end() )
        {
            return std::get<1> ( it->second );
        }
        return unique_any_nullptr;
    }
}
