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

#include <fstream>
#include <sstream>
#include <exception>
#include <vector>
#include <cassert>
#include <cstring>
#include <mutex>
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "mesh.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/AeonEngine.h"
#include "aeongames/ResourceCache.h"
#include "aeongames/Utilities.h"
#include "aeongames/Mesh.h"
#include "aeongames/CRC.h"

namespace AeonGames
{
    Mesh::Mesh() : mAABB{}
    {
    }
    Mesh::Mesh ( const std::string&  aFilename )
    {
        try
        {
            Load ( aFilename );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }
    Mesh::Mesh ( const void * aBuffer, size_t aBufferSize )
    {
        if ( !aBuffer && !aBufferSize )
        {
            throw std::runtime_error ( "Cannot initialize mesh object with null data." );
            return;
        }
        try
        {
            Load ( aBuffer, aBufferSize );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }
    Mesh::Mesh ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    Mesh::~Mesh() = default;

    const std::shared_ptr<Mesh> Mesh::GetMesh ( uint32_t aId )
    {
        return ResourceCache<uint32_t, Mesh>::Get ( aId, aId );
    }

    const std::shared_ptr<Mesh> Mesh::GetMesh ( const std::string& aPath )
    {
        uint32_t id = crc32i ( aPath.c_str(), aPath.size() );
        return Mesh::GetMesh ( id );
    }

    const AABB& Mesh::GetAABB() const
    {
        return mAABB;
    }

    void Mesh::Load ( const std::string& aFilename )
    {
        static std::mutex m;
        static MeshBuffer mesh_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<MeshBuffer> ( mesh_buffer, aFilename, "AEONMSH" );
        Load ( mesh_buffer );
        mesh_buffer.Clear();
    }

    void Mesh::Load ( const void * aBuffer, size_t aBufferSize )
    {
        static std::mutex m;
        static MeshBuffer mesh_buffer;
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject<MeshBuffer> ( mesh_buffer, aBuffer, aBufferSize, "AEONMSH" );
        Load ( mesh_buffer );
        mesh_buffer.Clear();
    }

    void Mesh::Load ( const MeshBuffer & aMeshBuffer )
    {
        mAABB = AABB
        {
            {
                aMeshBuffer.center().x(),
                aMeshBuffer.center().y(),
                aMeshBuffer.center().z()
            },
            {
                aMeshBuffer.radii().x(),
                aMeshBuffer.radii().y(),
                aMeshBuffer.radii().z()
            }
        };
        mVertexCount = aMeshBuffer.vertexcount();
        mIndexCount = aMeshBuffer.indexcount();
        mIndexType = aMeshBuffer.indextype();

        mVertexFlags = aMeshBuffer.vertexflags();
        // Sadly we must copy here
        if ( aMeshBuffer.vertexcount() )
        {
            mVertexBuffer = aMeshBuffer.vertexbuffer();
        }
        if ( aMeshBuffer.indexcount() )
        {
            mIndexBuffer = aMeshBuffer.indexbuffer();
        }
    }

    void Mesh::Unload()
    {
        mAABB = {};
        mVertexFlags =
            mVertexCount =
                mIndexType =
                    mIndexCount = 0;
        mVertexBuffer.clear();
        mIndexBuffer.clear();
    }

    uint32_t Mesh::GetStride () const
    {
        uint32_t stride = 0;
        if ( mVertexFlags & POSITION_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( mVertexFlags & NORMAL_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( mVertexFlags & TANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( mVertexFlags & BITANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( mVertexFlags & UV_BIT )
        {
            stride += sizeof ( float ) * 2;
        }
        if ( mVertexFlags & WEIGHT_BIT )
        {
            stride += sizeof ( uint8_t ) * 8;
        }
        return stride;
    }

    uint32_t Mesh::GetIndexSize() const
    {
        switch ( mIndexType )
        {
        case BYTE:
        case UNSIGNED_BYTE:
            return 1;
        case SHORT:
        case UNSIGNED_SHORT:
        case TWO_BYTES:
            return 2;
        case THREE_BYTES:
            return 3;
        case INT:
        case UNSIGNED_INT:
        case FLOAT:
        case FOUR_BYTES:
            return 4;
        case DOUBLE:
            return 8;
        };
        assert ( 0 && "Invalid Index Type." );
        return 0;
    }

    uint32_t Mesh::GetVertexFlags() const
    {
        return mVertexFlags;
    }
    uint32_t Mesh::GetVertexCount() const
    {
        return mVertexCount;
    }
    uint32_t Mesh::GetIndexType() const
    {
        return mIndexType;
    }
    uint32_t Mesh::GetIndexCount() const
    {
        return mIndexCount;
    }
    const std::string& Mesh::GetVertexBuffer() const
    {
        return mVertexBuffer;
    }
    const std::string& Mesh::GetIndexBuffer() const
    {
        return mIndexBuffer;
    }
}
