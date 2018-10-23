/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#include <utility>
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
#include "aeongames/AeonEngine.h"
#include "aeongames/Utilities.h"
#include "aeongames/CRC.h"
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer ( aVulkanRenderer ), mBuffer ( mVulkanRenderer )
    {
    }
    VulkanMesh::~VulkanMesh()
    {
        Unload();
    }

    VkIndexType VulkanMesh::GetIndexType() const
    {
        switch ( mIndexSize )
        {
        case 1:
        case 2:
            return VK_INDEX_TYPE_UINT16;
        case 4:
            return VK_INDEX_TYPE_UINT32;
        default:
            break;
        };
        throw std::runtime_error ( "Invalid Index Size." );
    }

    const VkBuffer & VulkanMesh::GetBuffer() const
    {
        return mBuffer.GetBuffer();
    }
    void VulkanMesh::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }
    void VulkanMesh::Load ( uint32_t aId )
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
    void VulkanMesh::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static MeshBuffer mesh_buffer;
        LoadProtoBufObject ( mesh_buffer, aBuffer, aBufferSize, "AEONMSH" );
        Load ( mesh_buffer );
        mesh_buffer.Clear();
    }

    void VulkanMesh::Load ( const MeshBuffer& aMeshBuffer )
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
        mIndexSize = aMeshBuffer.indexsize();
        mVertexFlags = aMeshBuffer.vertexflags();

        // Vulkan Specific code
        const VkDeviceSize vertex_buffer_size = ( sizeof ( Vertex ) * GetVertexCount() );
        // We need to expand 1 byte indices to 2 since they're not supported on Vulkan
        const VkDeviceSize index_buffer_size = aMeshBuffer.indexbuffer().size() * ( ( aMeshBuffer.indexsize() == 1 ) ? 2 : 1 );
        VkDeviceSize buffer_size = vertex_buffer_size + index_buffer_size;
        VkBufferUsageFlags buffer_usage = ( ( mVertexCount ) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0 ) | ( ( mIndexCount ) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0 );
        mBuffer.Initialize ( buffer_size, buffer_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        std::vector<uint8_t> buffer ( buffer_size );
        if ( GetVertexCount() )
        {
            auto* vertices = reinterpret_cast<Vertex*> ( buffer.data() );
            memset ( vertices, 0, vertex_buffer_size );
            uintptr_t offset = 0;
            for ( uint32_t j = 0; j < GetVertexCount(); ++j )
            {
                if ( mVertexFlags & Mesh::POSITION_BIT )
                {
                    memcpy ( vertices[j].position, aMeshBuffer.vertexbuffer().data() + offset, sizeof ( Vertex::position ) );
                    offset += sizeof ( Vertex::position );
                }

                if ( mVertexFlags & Mesh::NORMAL_BIT )
                {
                    memcpy ( vertices[j].normal, aMeshBuffer.vertexbuffer().data() + offset, sizeof ( Vertex::normal ) );
                    offset += sizeof ( Vertex::normal );
                }

                if ( mVertexFlags & Mesh::TANGENT_BIT )
                {
                    memcpy ( vertices[j].tangent, aMeshBuffer.vertexbuffer().data() + offset, sizeof ( Vertex::tangent ) );
                    offset += sizeof ( Vertex::tangent );
                }

                if ( mVertexFlags & Mesh::BITANGENT_BIT )
                {
                    memcpy ( vertices[j].bitangent, aMeshBuffer.vertexbuffer().data() + offset, sizeof ( Vertex::bitangent ) );
                    offset += sizeof ( Vertex::bitangent );
                }

                if ( mVertexFlags & Mesh::UV_BIT )
                {
                    memcpy ( vertices[j].uv, aMeshBuffer.vertexbuffer().data() + offset, sizeof ( Vertex::uv ) );
                    offset += sizeof ( Vertex::uv );
                }

                if ( mVertexFlags & Mesh::WEIGHT_BIT )
                {
                    memcpy ( vertices[j].weight_indices, aMeshBuffer.vertexbuffer().data() + offset, sizeof ( Vertex::weight_indices ) );
                    offset += sizeof ( Vertex::weight_indices );
                    memcpy ( vertices[j].weight_influences, aMeshBuffer.vertexbuffer().data() + offset, sizeof ( Vertex::weight_influences ) );
                    offset += sizeof ( Vertex::weight_influences );
                }
            }
        }
        if ( mIndexCount )
        {
            void* data = buffer.data() + vertex_buffer_size;
            if ( aMeshBuffer.indexsize() != 1 )
            {
                memcpy ( data, aMeshBuffer.indexbuffer().data(), aMeshBuffer.indexbuffer().size() );
            }
            else
            {
                /**@note upcast to 16 bit indices.*/
                for ( size_t j = 0; j < aMeshBuffer.indexbuffer().size(); ++j )
                {
                    reinterpret_cast<uint16_t*> ( data ) [j] = aMeshBuffer.indexbuffer() [j];
                }
            }
        }
        mBuffer.WriteMemory ( 0, buffer.size(), buffer.data() );
    }

    void VulkanMesh::Unload ()
    {
    }

    uint32_t VulkanMesh::GetIndexCount() const
    {
        return mIndexCount;
    }

    uint32_t VulkanMesh::GetVertexCount() const
    {
        return mVertexCount;
    }

    const AABB& VulkanMesh::GetAABB() const
    {
        return mAABB;
    }

    uint32_t VulkanMesh::GetIndexSize () const
    {
        return mIndexSize;
    }
}
