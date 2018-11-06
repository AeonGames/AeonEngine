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
#ifndef AEONGAMES_RESOURCEID_H
#define AEONGAMES_RESOURCEID_H

#include <cstdint>
#include <string>
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"

namespace AeonGames
{
    class ResourceId
    {
    public:
        ResourceId() = default;
        ResourceId ( const ResourceId& ) = default;
        ResourceId ( uint32_t aType, uint32_t aPath ) :
            mType{aType}, mPath{aPath} {}
        ResourceId ( const std::string& aType, const std::string& aPath ) :
            mType{crc32i ( aType.data(), aType.length() ) }, mPath{crc32i ( aPath.data(), aPath.length() ) } {}
        ResourceId ( const std::string& aType, uint32_t aPath ) :
            mType{crc32i ( aType.data(), aType.length() ) }, mPath{aPath} {}
        ~ResourceId() = default;
        uint32_t GetType() const
        {
            return mType;
        }
        uint32_t GetPath() const
        {
            return mPath;
        }
    private:
        uint32_t mType{};
        uint32_t mPath{};
    };
}

#endif