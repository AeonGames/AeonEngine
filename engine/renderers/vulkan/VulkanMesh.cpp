/*
Copyright (C) 2017-2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.h"
#include "aeongames/Utilities.h"
#include "aeongames/CRC.h"
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const VulkanRenderer& aVulkanRenderer, const Mesh& aMesh ) :
        mVulkanRenderer {aVulkanRenderer}, mMesh{&aMesh}, mMeshBuffer { mVulkanRenderer }
    {
        // Vulkan Specific code
        const VkDeviceSize vertex_buffer_size = ( sizeof ( Vertex ) * aMesh.GetVertexCount() );
        // We need to expand 1 byte indices to 2 since they're not supported on Vulkan
        const VkDeviceSize index_buffer_size = aMesh.GetIndexBuffer().size() * ( ( aMesh.GetIndexSize() == 1 ) ? 2 : 1 );
        VkDeviceSize buffer_size = vertex_buffer_size + index_buffer_size;
        VkBufferUsageFlags buffer_usage = ( ( aMesh.GetVertexCount() ) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0 ) | ( ( aMesh.GetIndexCount() ) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0 );
        mMeshBuffer.Initialize ( buffer_size, buffer_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        std::vector<uint8_t> buffer ( buffer_size );
        if ( aMesh.GetVertexCount() )
        {
            auto* vertices = reinterpret_cast<Vertex*> ( buffer.data() );
            memset ( vertices, 0, vertex_buffer_size );
            uintptr_t offset = 0;
            for ( uint32_t j = 0; j < aMesh.GetVertexCount(); ++j )
            {
                if ( aMesh.GetVertexFlags() & Mesh::POSITION_BIT )
                {
                    memcpy ( vertices[j].position, aMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::position ) );
                    offset += sizeof ( Vertex::position );
                }

                if ( aMesh.GetVertexFlags() & Mesh::NORMAL_BIT )
                {
                    memcpy ( vertices[j].normal, aMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::normal ) );
                    offset += sizeof ( Vertex::normal );
                }

                if ( aMesh.GetVertexFlags() & Mesh::TANGENT_BIT )
                {
                    memcpy ( vertices[j].tangent, aMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::tangent ) );
                    offset += sizeof ( Vertex::tangent );
                }

                if ( aMesh.GetVertexFlags() & Mesh::BITANGENT_BIT )
                {
                    memcpy ( vertices[j].bitangent, aMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::bitangent ) );
                    offset += sizeof ( Vertex::bitangent );
                }

                if ( aMesh.GetVertexFlags() & Mesh::UV_BIT )
                {
                    memcpy ( vertices[j].uv, aMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::uv ) );
                    offset += sizeof ( Vertex::uv );
                }

                if ( aMesh.GetVertexFlags() & Mesh::WEIGHT_IDX_BIT )
                {
                    memcpy ( vertices[j].weight_indices, aMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::weight_indices ) );
                    offset += sizeof ( Vertex::weight_indices );
                }

                if ( aMesh.GetVertexFlags() & Mesh::WEIGHT_BIT )
                {
                    memcpy ( vertices[j].weight_influences, aMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::weight_influences ) );
                    offset += sizeof ( Vertex::weight_influences );
                }

                if ( aMesh.GetVertexFlags() & Mesh::COLOR_BIT )
                {
                    memcpy ( vertices[j].color, aMesh.GetVertexBuffer().data() + offset, sizeof ( Vertex::color ) );
                    offset += sizeof ( Vertex::color );
                }
            }
        }
        if ( aMesh.GetIndexCount() )
        {
            void* data = buffer.data() + vertex_buffer_size;
            if ( aMesh.GetIndexSize() != 1 )
            {
                memcpy ( data, aMesh.GetIndexBuffer().data(), aMesh.GetIndexBuffer().size() );
            }
            else
            {
                /**@note upcast to 16 bit indices.*/
                for ( size_t j = 0; j < aMesh.GetIndexCount(); ++j )
                {
                    ( reinterpret_cast<uint16_t*> ( data ) [j] ) = aMesh.GetIndexBuffer() [j];
                }
            }
        }
        mMeshBuffer.WriteMemory ( 0, buffer.size(), buffer.data() );
    }

    VulkanMesh::~VulkanMesh()
    {
        mMeshBuffer.Finalize();
    }

    VulkanMesh::VulkanMesh ( const VulkanMesh& aVulkanMesh ) :
        mVulkanRenderer{aVulkanMesh.mVulkanRenderer},
        mMesh{aVulkanMesh.mMesh}, mMeshBuffer{aVulkanMesh.mMeshBuffer} {}

    VulkanMesh::VulkanMesh ( VulkanMesh&& aVulkanMesh ) :
        mVulkanRenderer{aVulkanMesh.mVulkanRenderer},
        mMesh{aVulkanMesh.mMesh}, mMeshBuffer{std::move ( aVulkanMesh.mMeshBuffer ) } {}

    VulkanMesh& VulkanMesh::operator= ( const VulkanMesh& aVulkanMesh )
    {
        assert ( &mVulkanRenderer == &aVulkanMesh.mVulkanRenderer );
        mMesh = aVulkanMesh.mMesh;
        mMeshBuffer = aVulkanMesh.mMeshBuffer;
        return *this;
    }

    VulkanMesh& VulkanMesh::operator= ( VulkanMesh&& aVulkanMesh )
    {
        assert ( &mVulkanRenderer == &aVulkanMesh.mVulkanRenderer );
        std::swap ( mMesh, aVulkanMesh.mMesh );
        std::swap ( mMeshBuffer, aVulkanMesh.mMeshBuffer );
        return *this;
    }

    static VkIndexType GetIndexType ( const Mesh* aMesh )
    {
        switch ( aMesh->GetIndexSize() )
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

    void VulkanMesh::Bind() const
    {
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers ( mVulkanRenderer.GetCommandBuffer(), 0, 1, &mMeshBuffer.GetBuffer(), &offset );
        if ( mMesh->GetIndexCount() )
        {
            vkCmdBindIndexBuffer ( mVulkanRenderer.GetCommandBuffer(),
                                   mMeshBuffer.GetBuffer(), ( sizeof ( Vertex ) * mMesh->GetVertexCount() ),
                                   GetIndexType ( mMesh ) );
        }
    }
}
