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

#include "aeongames/Utilities.h"
#include "aeongames/Mesh.h"

namespace AeonGames
{
    Mesh::Mesh ( const std::string&  aFilename )
        :
        mFilename ( aFilename )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }
    Mesh::Mesh ( const void * aBuffer, size_t aBufferSize )
        :
        mFilename{}
    {
        if ( !aBuffer && !aBufferSize )
        {
            throw std::runtime_error ( "Cannot initialize mesh object with null data." );
        }
        try
        {
            Initialize ( aBuffer, aBufferSize );
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }
    Mesh::~Mesh()
    {
        Finalize();
    }

    const float * const Mesh::GetCenterRadii() const
    {
        return mCenterRadii;
    }

    void Mesh::Initialize ( const void * aBuffer, size_t aBufferSize )
    {
        static MeshBuffer mesh_buffer;
        if ( !aBuffer && !aBufferSize )
        {
            LoadProtoBufObject<MeshBuffer> ( mesh_buffer, mFilename, "AEONMSH" );
        }
        else
        {
            LoadProtoBufObject<MeshBuffer> ( mesh_buffer, aBuffer, aBufferSize, "AEONMSH" );
        }
        // Extract Center
        mCenterRadii[0] = mesh_buffer.center().x();
        mCenterRadii[1] = mesh_buffer.center().y();
        mCenterRadii[2] = mesh_buffer.center().z();
        // Extract Radius
        mCenterRadii[3] = mesh_buffer.radii().x();
        mCenterRadii[4] = mesh_buffer.radii().y();
        mCenterRadii[5] = mesh_buffer.radii().z();

        mVertexCount = mesh_buffer.vertexcount();
        mIndexCount = mesh_buffer.indexcount();
        mIndexType = mesh_buffer.indextype();

        mVertexFlags = mesh_buffer.vertexflags();
        // Sadly we must copy here (or do we?)
        if ( mesh_buffer.vertexcount() )
        {
            mVertexBuffer = mesh_buffer.vertexbuffer();
        }
        if ( mesh_buffer.indexcount() )
        {
            mIndexBuffer = mesh_buffer.indexbuffer();
        }

        mesh_buffer.Clear();
    }

    void Mesh::Finalize()
    {
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
