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
        enum AttributeMask
        {
            POSITION_BIT =   0b1,
            NORMAL_BIT =     0b10,
            TANGENT_BIT =    0b100,
            BITANGENT_BIT =  0b1000,
            UV_BIT =         0b10000,
            WEIGHT_IDX_BIT = 0b100000,
            WEIGHT_BIT =     0b1000000,
            COLOR_BIT =      0b10000000,
        };
        enum IndexType
        {
            /**@todo Refactor to store index size rather than type.
               This will involve changing the exporter and the msh format specification.*/
            BYTE = 0x00,
            UNSIGNED_BYTE = 0x01,
            SHORT = 0x02,
            UNSIGNED_SHORT = 0x03,
            INT = 0x04,
            UNSIGNED_INT = 0x05,
            FLOAT = 0x06,
            TWO_BYTES = 0x07,
            THREE_BYTES = 0x08,
            FOUR_BYTES = 0x09,
            DOUBLE = 0x0A
        };
        DLL Mesh();
        DLL ~Mesh() final;
        DLL void LoadFromPBMsg ( const MeshMsg& aMeshMsg );
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        DLL void Unload() final;
        DLL uint32_t GetVertexFlags() const;
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
        uint32_t mVertexFlags{};
        uint32_t mVertexCount{};
        uint32_t mIndexSize{};
        uint32_t mIndexCount{};
    };
}
#endif
