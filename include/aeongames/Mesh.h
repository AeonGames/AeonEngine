/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MESH_H
#define AEONGAMES_MESH_H
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include "aeongames/AABB.h"
#include "aeongames/Resource.h"

namespace AeonGames
{
    class MeshMsg;
    class Mesh final : public Resource
    {
    public:
        enum AttributeSemantic : uint8_t
        {
            POSITION                      = 0,
            NORMAL                        = 1,
            TANGENT                       = 2,
            BITANGENT                     = 3,
            TEXCOORD                      = 4,
            WEIGHT_INDEX                  = 5,
            WEIGHT_VALUE                  = 6,
            COLOR                         = 7,
            SEMANTIC_COUNT
        };

        enum AttributeType : uint8_t
        {
            BYTE             =  0,
            UNSIGNED_BYTE    =  1,
            SHORT            =  2,
            UNSIGNED_SHORT   =  3,
            HALF_FLOAT       =  4,
            INT              =  5,
            UNSIGNED_INT     =  6,
            FLOAT            =  7,
            FIXED            =  8,
            DOUBLE           =  9,
        };
        using AttributeSize       = uint8_t;
        using AttributeNormalized = uint8_t;
        using AttributeTuple = std::tuple<AttributeSemantic, AttributeSize, AttributeType, AttributeNormalized>;
        DLL Mesh();
        DLL ~Mesh() final;
        DLL void LoadFromPBMsg ( const MeshMsg& aMeshMsg );
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        DLL void Unload() final;
        DLL const std::vector<AttributeTuple>& GetAttributes() const;
        DLL uint32_t GetIndexSize() const;
        DLL uint32_t GetIndexCount() const;
        DLL uint32_t GetVertexCount() const;
        DLL const std::vector<uint8_t>& GetVertexBuffer() const;
        DLL const std::vector<uint8_t>& GetIndexBuffer() const;
        DLL const AABB& GetAABB() const;
        DLL uint32_t GetStride() const;
    private:
        AABB mAABB{};
        std::vector<uint8_t> mVertexBuffer{};
        std::vector<uint8_t> mIndexBuffer{};
        std::vector<AttributeTuple> mAttributes{};
        uint32_t mVertexCount{};
        uint32_t mIndexSize{};
        uint32_t mIndexCount{};
    };
    DLL size_t GetAttributeTotalSize ( const Mesh::AttributeTuple& aAttributeTuple );
}
#endif
