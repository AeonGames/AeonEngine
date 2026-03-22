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
#ifndef AEONGAMES_RESOURCEFACTORY_H
#define AEONGAMES_RESOURCEFACTORY_H

#include <algorithm>
#include <functional>
#include "aeongames/Platform.hpp"
#include "aeongames/UniqueAnyPtr.hpp"

namespace AeonGames
{
    class ResourceId;
    /** @brief Construct a resource identified by a ResourceId.
     *  @param aResourceId The identifier for the resource to construct.
     *  @return A UniqueAnyPtr owning the constructed resource.
     */
    DLL UniqueAnyPtr ConstructResource ( const ResourceId& aResourceId );
    /** @brief Get the default resource for a given type.
     *  @param aType The resource type identifier.
     *  @return Reference to the default resource.
     */
    DLL const UniqueAnyPtr& GetDefaultResource ( uint32_t aType );
    /** @brief Register a constructor for a resource type.
     *  @param aType The resource type identifier.
     *  @param aConstructor Callable that constructs the resource given a type.
     *  @param aDefaultResource Optional default resource instance.
     *  @return True if registration succeeded.
     */
    DLL bool RegisterResourceConstructor ( uint32_t aType, const std::function < UniqueAnyPtr ( uint32_t ) > & aConstructor, UniqueAnyPtr&& aDefaultResource = nullptr );
    /** @brief Unregister a resource constructor.
     *  @param aType The resource type identifier to unregister.
     *  @return True if unregistration succeeded.
     */
    DLL bool UnregisterResourceConstructor ( uint32_t aType );
    /** @brief Enumerate all registered resource constructors.
     *  @param aEnumerator Callback invoked for each type; return false to stop.
     */
    DLL void EnumerateResourceConstructors ( const std::function<bool ( uint32_t ) >& aEnumerator );
}

#endif
