/*
Copyright (C) 2021,2025 Rodrigo Jose Hernandez Cordoba

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
    DLL size_t GetNextConsecutiveId();
    class Resource
    {
    public:
        virtual ~Resource() {}
        virtual void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) = 0;
        virtual void Unload () = 0;
        DLL void LoadFromId ( uint32_t aId );
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
