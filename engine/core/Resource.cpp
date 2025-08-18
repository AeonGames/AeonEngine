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
#include <atomic>
#include "aeongames/Resource.hpp"
#include "aeongames/ResourceCache.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/CRC.hpp"

namespace AeonGames
{
    size_t GetNextConsecutiveId()
    {
        static std::atomic<std::size_t> consecutive{};
        return consecutive++;
    }

    void Resource::LoadFromId ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        LoadFromMemory ( buffer.data(), buffer.size() );
    }

    void Resource::LoadFromFile ( const std::string& aFilename )
    {
        LoadFromId ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }

    size_t Resource::GetConsecutiveId() const
    {
        return mConsecutiveId;
    }
}
