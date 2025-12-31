/*
Copyright (C) 2017-2019,2021,2023,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/LogLevel.hpp"
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMesh::VulkanMesh ( const VulkanRenderer& aVulkanRenderer, const Mesh& aMesh ) :
        mVulkanRenderer {aVulkanRenderer}, mMesh{&aMesh}, mMeshBuffer { mVulkanRenderer }
    {
        const VkDeviceSize buffer_size{ aMesh.GetVertexBuffer().size() + ( aMesh.GetIndexBuffer().size() * ( aMesh.GetIndexSize() == 1 ? 2 : 1 ) ) };
        const VkBufferUsageFlags buffer_usage {static_cast<VkBufferUsageFlags> ( ( aMesh.GetVertexCount() ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0 ) | ( aMesh.GetIndexCount() ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0 ) ) };

        mMeshBuffer.Initialize ( buffer_size, buffer_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        if ( aMesh.GetVertexCount() )
        {
            mMeshBuffer.WriteMemory ( 0, aMesh.GetVertexBuffer().size(), aMesh.GetVertexBuffer().data() );
        }
        if ( aMesh.GetIndexCount() )
        {
            if ( aMesh.GetIndexSize() != 1 )
            {
                mMeshBuffer.WriteMemory ( aMesh.GetVertexBuffer().size(), aMesh.GetIndexBuffer().size(), aMesh.GetIndexBuffer().data() );
            }
            else
            {
                std::vector<uint16_t> buffer{};
                buffer.resize ( aMesh.GetIndexCount() );
                for ( size_t i = 0; i < aMesh.GetIndexCount(); ++i )
                {
                    buffer[i] = aMesh.GetIndexBuffer() [i];
                }
                mMeshBuffer.WriteMemory ( aMesh.GetVertexBuffer().size(), buffer.size() * sizeof ( uint16_t ), buffer.data() );
            }
        }
    }

    VulkanMesh::~VulkanMesh()
    {
        mMeshBuffer.Finalize();
    }

    VulkanMesh::VulkanMesh ( VulkanMesh&& aVulkanMesh ) :
        mVulkanRenderer{aVulkanMesh.mVulkanRenderer},
        mMesh{aVulkanMesh.mMesh}, mMeshBuffer{std::move ( aVulkanMesh.mMeshBuffer ) } {}

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
        std::cout << LogLevel::Error << "Invalid Index Size." << std::endl;
        throw std::runtime_error ( "Invalid Index Size." );
    }

    void VulkanMesh::Bind ( VkCommandBuffer aVkCommandBuffer ) const
    {
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers ( aVkCommandBuffer, 0, 1, &mMeshBuffer.GetBuffer(), &offset );
        if ( mMesh->GetIndexCount() )
        {
            vkCmdBindIndexBuffer ( aVkCommandBuffer,
                                   mMeshBuffer.GetBuffer(), mMesh->GetVertexBuffer().size(),
                                   GetIndexType ( mMesh ) );
        }
    }
}
