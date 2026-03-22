/*
Copyright (C) 2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_RESOURCE_H
#define AEONGAMES_RESOURCE_H

#include <string>
#include "aeongames/Platform.hpp"

namespace AeonGames
{
    /** @brief Returns the next available consecutive resource identifier.
     *  @return A unique, monotonically increasing identifier. */
    DLL size_t GetNextConsecutiveId();
    /** @brief Base class for loadable engine resources.
     *
     *  Provides a common interface for loading resource data from files,
     *  memory buffers, or numeric identifiers, and assigns each instance
     *  a unique consecutive id for runtime tracking. */
    class Resource
    {
    public:
        /** @brief Virtual destructor. */
        virtual ~Resource() {}
        /** @brief Load the resource from a raw memory buffer.
         *  @param aBuffer     Pointer to the data buffer.
         *  @param aBufferSize Size of the data buffer in bytes. */
        virtual void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) = 0;
        /** @brief Release all data held by this resource. */
        virtual void Unload () = 0;
        /** @brief Load the resource identified by a numeric id.
         *  @param aId Resource identifier. */
        DLL void LoadFromId ( uint32_t aId );
        /** @brief Load the resource from a file on disk.
         *  @param aFilename Path to the resource file. */
        DLL void LoadFromFile ( const std::string& aFilename );
        /**
         * @brief Get the Consecutive Id for the resource object.
         * The consecutive Id is different from the resource Id,
         * it is a unique number per resource that increments as each
         * object is created, as such it is a runtime value that will
         * most likely change each run and cannot be relied on
         * for serialization purposes, it is intended as an identifier
         * for other classes who need to cache their own resource data
         * linked to the original resource such as renderers.
         * @return Unique Consecutive Id of the object.
         */
        DLL size_t GetConsecutiveId() const;
    private:
        size_t mConsecutiveId{ GetNextConsecutiveId() };
    };
}

#endif
