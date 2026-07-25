/*
Copyright (C) 2016-2018,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.hpp"
#include <algorithm>
#include "aeongames/ProtoBufHelpers.hpp"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Mesh.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/AeonEngine.hpp"

namespace AeonGames
{
    Mesh::Mesh() = default;
    Mesh::~Mesh()
    {
        Unload();
    }

    const std::vector<Mesh::AttributeTuple>& Mesh::GetAttributes () const
    {
        return mAttributes;
    }

    uint32_t Mesh::GetIndexSize () const
    {
        return mIndexSize;
    }

    uint32_t Mesh::GetAttributeOffset ( const AttributeTuple& aAttribute ) const
    {
        return std::get<4> ( aAttribute );
    }

    uint32_t Mesh::GetIndexCount() const
    {
        return mIndexCount;
    }

    uint32_t Mesh::GetVertexCount() const
    {
        return mVertexCount;
    }

    const std::vector<uint8_t>& Mesh::GetVertexBuffer() const
    {
        return mVertexBuffer;
    }

    const std::vector<uint8_t>& Mesh::GetIndexBuffer() const
    {
        return mIndexBuffer;
    }

    const AABB& Mesh::GetAABB() const
    {
        return mAABB;
    }

    size_t GetAttributeTotalSize ( const Mesh::AttributeTuple& aAttributeTuple )
    {
        switch ( std::get<Mesh::AttributeType> ( aAttributeTuple ) )
        {
        case Mesh::BYTE:
        case Mesh::UNSIGNED_BYTE:
            return std::get<1> ( aAttributeTuple );
            break;
        case Mesh::SHORT:
        case Mesh::UNSIGNED_SHORT:
        case Mesh::HALF_FLOAT:
            return 2 * std::get<1> ( aAttributeTuple );
            break;
        case Mesh::INT:
        case Mesh::UNSIGNED_INT:
        case Mesh::FLOAT:
        case Mesh::FIXED:
            return 4 * std::get<1> ( aAttributeTuple );
        case Mesh::DOUBLE:
            return 8 * std::get<1> ( aAttributeTuple );
            break;
        }
        return 0;
    }

    size_t Mesh::GetStride () const
    {
        if ( mVertexStride != 0 )
        {
            return mVertexStride;
        }
        size_t stride = 0;
        for ( const auto& i : mAttributes )
        {
            stride = std::max ( stride, static_cast<size_t> ( std::get<4> ( i ) ) + GetAttributeTotalSize ( i ) );
        }
        return stride;
    }

    void Mesh::LoadFromMemory ( const void* aBuffer, size_t aBufferSize )
    {
        LoadFromProtoBufObject<Mesh, MeshMsg, "AEONMSH"_mgk> ( *this, aBuffer, aBufferSize );
    }

    void Mesh::LoadFromPBMsg ( const MeshMsg& aMeshMsg )
    {
        mAABB = AABB
        {
            {
                aMeshMsg.center().x(),
                        aMeshMsg.center().y(),
                        aMeshMsg.center().z()
            },
            {
                aMeshMsg.radii().x(),
                        aMeshMsg.radii().y(),
                        aMeshMsg.radii().z()
            }
        };

        mVertexCount = aMeshMsg.vertexcount();
        mIndexCount = aMeshMsg.indexcount();
        mIndexSize = aMeshMsg.indexsize();
        mVertexStride = 0;
        mAttributes.reserve ( aMeshMsg.attribute().size() );
        uint32_t packed_offset = 0;
        for ( const auto& i : aMeshMsg.attribute() )
        {
            const uint32_t offset = i.has_offset() ? i.offset() : packed_offset;
            mAttributes.emplace_back (
                static_cast<AttributeSemantic> ( i.semantic() ),
                static_cast<AttributeSize> ( i.size() ),
                static_cast<AttributeType> ( i.type() ),
                static_cast<AttributeFlags> ( i.flags() ),
                offset
            );
            packed_offset = std::max ( packed_offset, offset + static_cast<uint32_t> ( GetAttributeTotalSize ( mAttributes.back() ) ) );
        }
        mVertexStride = aMeshMsg.has_vertexstride() && aMeshMsg.vertexstride() != 0 ? aMeshMsg.vertexstride() : packed_offset;

        mVertexBuffer.clear();
        mVertexBuffer.reserve ( aMeshMsg.vertexbuffer().size() );
        std::copy ( aMeshMsg.vertexbuffer().begin(), aMeshMsg.vertexbuffer().end(), std::back_inserter ( mVertexBuffer ) );

        mIndexBuffer.clear();
        mIndexBuffer.reserve ( aMeshMsg.indexbuffer().size() );
        std::copy ( aMeshMsg.indexbuffer().begin(), aMeshMsg.indexbuffer().end(), std::back_inserter ( mIndexBuffer ) );
    }
    void Mesh::Unload()
    {
        mAABB = AABB{};

        mVertexCount = 0;
        mIndexCount = 0;
        mIndexSize = 0;
        mVertexStride = 0;

        mAttributes.clear();
        mVertexBuffer.clear();
        mIndexBuffer.clear();
    }
}
