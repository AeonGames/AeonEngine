/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_AEONENGINE_H
#define AEONGAMES_AEONENGINE_H
#include "Platform.h"
#include "Memory.h"
#include <string>
#include <vector>

namespace AeonGames
{
    DLL bool Initialize();
    DLL void Finalize();
    DLL std::vector<std::string> GetResourcePath();
    DLL void SetResourcePath ( const std::vector<std::string>& aPath );
    /*! Returns the resource size referenced by its CRC value. */
    DLL size_t GetResourceSize ( uint32_t crc );
    /*! Returns the resource size referenced by its file name. */
    DLL size_t GetResourceSize ( const std::string& aFileName );
    /*! Returns the resource path referenced by its id. */
    DLL std::string GetResourcePath ( uint32_t crc );
    /*! Loads a specific resource referenced by its CRC into the provided buffer. */
    DLL void LoadResource ( uint32_t crc, void* buffer, size_t buffer_size );
    /*! Loads a specific resource referenced by its path into the provided buffer. */
    DLL void LoadResource ( const std::string& aFileName, void* buffer, size_t buffer_size );
}
#endif
