/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
    Mesh::Mesh ( const std::string& aFilename )
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
    Mesh::~Mesh()
    {
        Finalize();
    }

    const float * const Mesh::GetCenterRadii() const
    {
        return mCenterRadii;
    }

    void Mesh::Initialize()
    {
        static MeshBuffer mesh_buffer;
        LoadProtoBufObject<MeshBuffer> ( mesh_buffer, mFilename, "AEONMSH" );

        if ( mesh_buffer.trianglegroup().size() == 1 )
        {
            /* This avoids having to duplicate the values on single triangle group meshes.*/
            // Extract Center
            mCenterRadii[0] = mesh_buffer.trianglegroup().Get ( 0 ).center().x();
            mCenterRadii[1] = mesh_buffer.trianglegroup().Get ( 0 ).center().y();
            mCenterRadii[2] = mesh_buffer.trianglegroup().Get ( 0 ).center().z();
            // Extract Radius
            mCenterRadii[3] = mesh_buffer.trianglegroup().Get ( 0 ).radii().x();
            mCenterRadii[4] = mesh_buffer.trianglegroup().Get ( 0 ).radii().y();
            mCenterRadii[5] = mesh_buffer.trianglegroup().Get ( 0 ).radii().z();
        }
        else
        {
            // Extract Center
            mCenterRadii[0] = mesh_buffer.center().x();
            mCenterRadii[1] = mesh_buffer.center().y();
            mCenterRadii[2] = mesh_buffer.center().z();
            // Extract Radius
            mCenterRadii[3] = mesh_buffer.radii().x();
            mCenterRadii[4] = mesh_buffer.radii().y();
            mCenterRadii[5] = mesh_buffer.radii().z();
        }

        mTriangleGroups.reserve ( mesh_buffer.trianglegroup().size() );
        for ( auto& i : mesh_buffer.trianglegroup() )
        {
            mTriangleGroups.emplace_back();

            // Extract Center
            mTriangleGroups.back().mCenterRadii[0] = i.center().x();
            mTriangleGroups.back().mCenterRadii[1] = i.center().y();
            mTriangleGroups.back().mCenterRadii[2] = i.center().z();
            // Extract Radius
            mTriangleGroups.back().mCenterRadii[3] = i.radii().x();
            mTriangleGroups.back().mCenterRadii[4] = i.radii().y();
            mTriangleGroups.back().mCenterRadii[5] = i.radii().z();

            mTriangleGroups.back().mVertexCount = i.vertexcount();
            mTriangleGroups.back().mIndexCount = i.indexcount();
            mTriangleGroups.back().mIndexType = 0x1400 | i.indextype();

            mTriangleGroups.back().mVertexFlags = i.vertexflags();
        }
        mesh_buffer.Clear();
    }

    void Mesh::Finalize()
    {
    }

    uint32_t Mesh::GetStride ( uint32_t aFlags ) const
    {
        uint32_t stride = 0;
        if ( aFlags & POSITION_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & NORMAL_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & TANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & BITANGENT_BIT )
        {
            stride += sizeof ( float ) * 3;
        }
        if ( aFlags & UV_BIT )
        {
            stride += sizeof ( float ) * 2;
        }
        if ( aFlags & WEIGHT_BIT )
        {
            stride += sizeof ( uint8_t ) * 8;
        }
        return stride;
    }

    uint32_t Mesh::GetIndexSize ( uint32_t aIndexType ) const
    {
        /**@todo change the magick numbers to an index type enum.*/
        switch ( aIndexType )
        {
        case 1:
            return 1;
        case 3:
            return 2;
        case 5:
            return 4;
        };
        assert ( 0 && "Invalid Index Type." );
        return 0;
    }
}
