/*
Copyright (C) 2018,2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_RESOURCEFACTORY_H
#define AEONGAMES_RESOURCEFACTORY_H

#include <algorithm>
#include <functional>
#include "aeongames/Platform.hpp"
#include "aeongames/UniqueAnyPtr.hpp"

namespace AeonGames
{
    class ResourceId;
    DLL UniqueAnyPtr ConstructResource ( const ResourceId& aResourceId );
    DLL const UniqueAnyPtr& GetDefaultResource ( uint32_t aType );
    DLL bool RegisterResourceConstructor ( uint32_t aType, const std::function < UniqueAnyPtr ( uint32_t ) > & aConstructor, UniqueAnyPtr&& aDefaultResource = nullptr );
    DLL bool UnregisterResourceConstructor ( uint32_t aType );
    DLL void EnumerateResourceConstructors ( const std::function<bool ( uint32_t ) >& aEnumerator );
}

#endif
