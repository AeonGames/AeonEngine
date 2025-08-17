/*
Copyright (C) 2016-2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/CRC.h"
#include "aeongames/Resource.h"

namespace AeonGames
{
    class MeshMsg;
    class Mesh final : public Resource
    {
    public:
        enum AttributeSemantic : uint32_t
        {
            POSITION                      = "VertexPosition"_crc32,
            NORMAL                        = "VertexNormal"_crc32,
            TANGENT                       = "VertexTangent"_crc32,
            BITANGENT                     = "VertexBitangent"_crc32,
            TEXCOORD                      = "VertexUV"_crc32,
            WEIGHT_INDEX                  = "VertexWeightIndices"_crc32,
            WEIGHT_VALUE                  = "VertexWeights"_crc32,
            COLOR                         = "VertexColor"_crc32,
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

        enum AttributeFlag : uint8_t
        {
            NORMALIZED       =  0b00000001,
            INTEGER          =  0b00000010,
        };

        using AttributeSize       = uint8_t;
        using AttributeFlags      = uint8_t;
        using AttributeTuple = std::tuple<AttributeSemantic, AttributeSize, AttributeType, AttributeFlags>;
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
        DLL size_t GetStride() const;
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
