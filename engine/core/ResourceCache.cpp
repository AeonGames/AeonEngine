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
#include <unordered_map>
#include "aeongames/ResourceCache.h"
#include "aeongames/ResourceFactory.h"
#include "aeongames/ResourceId.h"

namespace AeonGames
{
    static std::unordered_map<uint32_t, UniqueAnyPtr> gResourceStore{};

    void ClearAllResources()
    {
        gResourceStore.clear();
    }

    void EnumerateResources ( const std::function<bool ( uint32_t, const UniqueAnyPtr& ) >& aEnumerator )
    {
        for ( auto& i : gResourceStore )
        {
            if ( !aEnumerator ( i.first, i.second ) )
            {
                return;
            }
        }
    }

    const UniqueAnyPtr& StoreResource ( uint32_t aKey, UniqueAnyPtr&& pointer )
    {
        gResourceStore.emplace ( std::make_pair<> ( aKey, std::move ( pointer ) ) );
        return gResourceStore[aKey];
    }

    UniqueAnyPtr DisposeResource ( uint32_t aKey )
    {
        UniqueAnyPtr result{};
        auto i = gResourceStore.find ( aKey );
        if ( i != gResourceStore.end() )
        {
            result.Swap ( ( *i ).second );
            gResourceStore.erase ( i );
        }
        return result;
    }

    const UniqueAnyPtr& GetResource ( uint32_t aKey )
    {
        static const UniqueAnyPtr unique_nullptr{nullptr};
        auto i = gResourceStore.find ( aKey );
        if ( i != gResourceStore.end() )
        {
            return ( *i ).second;
        }
        return unique_nullptr;
    }

    const UniqueAnyPtr& GetResource ( const ResourceId& aResourceId )
    {
        auto i = gResourceStore.find ( aResourceId.GetPath() );
        if ( i != gResourceStore.end() )
        {
            return ( *i ).second;
        }
        return GetDefaultResource ( aResourceId.GetType() );
    }
}
