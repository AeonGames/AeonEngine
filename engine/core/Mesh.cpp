/*
Copyright (C) 2016-2018,2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Mesh.h"
#include "aeongames/Renderer.h"
#include "aeongames/AeonEngine.h"

namespace AeonGames
{
    Mesh::Mesh() = default;
    Mesh::~Mesh()
    {
        Unload();
    }
    uint32_t Mesh::GetVertexFlags () const
    {
        return mVertexFlags;
    }

    uint32_t Mesh::GetIndexSize () const
    {
        return mIndexSize;
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

    uint32_t Mesh::GetStride () const
    {
        uint32_t stride = 0;
        if ( mVertexFlags & Mesh::AttributeMask::POSITION_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( mVertexFlags & Mesh::AttributeMask::NORMAL_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( mVertexFlags & Mesh::AttributeMask::TANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( mVertexFlags & Mesh::AttributeMask::BITANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( mVertexFlags & Mesh::AttributeMask::UV_BIT )
        {
            stride += sizeof ( float ) * 2;
        }
        if ( mVertexFlags & Mesh::AttributeMask::WEIGHT_IDX_BIT )
        {
            stride += sizeof ( uint8_t ) * 4;
        }
        if ( mVertexFlags & Mesh::AttributeMask::WEIGHT_BIT )
        {
            stride += sizeof ( uint8_t ) * 4;
        }
        if ( mVertexFlags & Mesh::AttributeMask::COLOR_BIT )
        {
            stride += sizeof ( float ) * 3;
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
        mVertexFlags = aMeshMsg.vertexflags();

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
        mVertexFlags = 0;

        mVertexBuffer.clear();
        mIndexBuffer.clear();
    }
}
