/*
Copyright (C) 2016-2020 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AABB.h"

namespace AeonGames
{
    class MeshBuffer;
    class Mesh
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
        DLL virtual ~Mesh() = 0;
        virtual void Load ( uint32_t aId ) = 0;
        virtual void Load ( const std::string& aFilename ) = 0;
        virtual void Load ( const void* aBuffer, size_t aBufferSize ) = 0;
        virtual void Load ( const MeshBuffer& aMeshBuffer ) = 0;
        virtual void Unload () = 0;
        virtual uint32_t GetIndexSize () const = 0;
        virtual uint32_t GetIndexCount() const = 0;
        virtual uint32_t GetVertexCount() const = 0;
        virtual const AABB& GetAABB() const = 0;
    };

    inline uint32_t GetStride ( uint32_t aVertexFlags )
    {
        uint32_t stride = 0;
        if ( aVertexFlags & Mesh::AttributeMask::POSITION_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aVertexFlags & Mesh::AttributeMask::NORMAL_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aVertexFlags & Mesh::AttributeMask::TANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aVertexFlags & Mesh::AttributeMask::BITANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aVertexFlags & Mesh::AttributeMask::UV_BIT )
        {
            stride += sizeof ( float ) * 2;
        }
        if ( aVertexFlags & Mesh::AttributeMask::WEIGHT_IDX_BIT )
        {
            stride += sizeof ( uint8_t ) * 4;
        }
        if ( aVertexFlags & Mesh::AttributeMask::WEIGHT_BIT )
        {
            stride += sizeof ( uint8_t ) * 4;
        }
        if ( aVertexFlags & Mesh::AttributeMask::COLOR_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        return stride;
    }
}
#endif
