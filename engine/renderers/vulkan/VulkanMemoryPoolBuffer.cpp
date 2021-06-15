/*
Copyright (C) 2019,2021 Rodrigo Jose Hernandez Cordoba

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
#include <ostream>
#include <regex>
#include <array>
#include <utility>
#include <cassert>
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/Material.h"
#include "aeongames/Vector2.h"
#include "aeongames/Vector3.h"
#include "aeongames/Vector4.h"
#include "VulkanMemoryPoolBuffer.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMemoryPoolBuffer::VulkanMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer, size_t aStackSize ) :
        mVulkanRenderer { aVulkanRenderer },
        mUniformBuffer { mVulkanRenderer,
                         ( ( aStackSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment - 1 ) ) + 1, // Adjust for alignment
                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT }
    {
        InitializeDescriptorPool();
        InitializeDescriptorSet();
    }

    VulkanMemoryPoolBuffer::VulkanMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer { aVulkanRenderer } {}

    void VulkanMemoryPoolBuffer::Initialize ( size_t aStackSize )
    {
        mUniformBuffer.Initialize ( ( ( aStackSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment - 1 ) ) + 1, // Adjust for alignment
                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
        InitializeDescriptorPool();
        InitializeDescriptorSet();
    }

    VulkanMemoryPoolBuffer::~VulkanMemoryPoolBuffer()
    {
        Finalize();
    }

    void VulkanMemoryPoolBuffer::Finalize()
    {
        FinalizeDescriptorSet();
        FinalizeDescriptorPool();
        mUniformBuffer.Finalize();
    }

    void VulkanMemoryPoolBuffer::InitializeDescriptorPool()
    {
        VkDescriptorPoolSize descriptor_pool_size{};
        descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptor_pool_size.descriptorCount = 1;
        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.pNext = nullptr;
        descriptor_pool_create_info.flags = 0;
        descriptor_pool_create_info.maxSets = 1;
        descriptor_pool_create_info.poolSizeCount = 1;
        descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;
        if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer.GetDevice(), &descriptor_pool_create_info, nullptr, &mVkDescriptorPool ) )
        {
            std::ostringstream stream;
            stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanMemoryPoolBuffer::InitializeDescriptorSet()
    {
        VkDescriptorSetLayout descriptorset_layout{VK_NULL_HANDLE};
        descriptorset_layout = mVulkanRenderer.GetUniformBufferDynamicDescriptorSetLayout();
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        descriptor_set_allocate_info.pSetLayouts = &descriptorset_layout;
        if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &mVkDescriptorSet ) )
        {
            std::ostringstream stream;
            stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        VkDescriptorBufferInfo descriptor_buffer_info = { mUniformBuffer.GetBuffer(), 0, mUniformBuffer.GetSize() };
        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstSet = mVkDescriptorSet;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        write_descriptor_set.pImageInfo = nullptr;
        write_descriptor_set.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &write_descriptor_set, 0, nullptr );
    }

    void VulkanMemoryPoolBuffer::FinalizeDescriptorPool()
    {
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, nullptr );
            mVkDescriptorPool = VK_NULL_HANDLE;
        }
    }

    void VulkanMemoryPoolBuffer::FinalizeDescriptorSet()
    {
        if ( mVkDescriptorSet != VK_NULL_HANDLE )
        {
            vkFreeDescriptorSets ( mVulkanRenderer.GetDevice(),
                                   mVkDescriptorPool,
                                   1,
                                   &mVkDescriptorSet );
            mVkDescriptorSet = VK_NULL_HANDLE;
        }
    }

    BufferAccessor VulkanMemoryPoolBuffer::Allocate ( size_t aSize )
    {
        size_t offset = mOffset;
        mOffset += ( (  aSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment - 1 ) ) + 1;
        if ( mOffset > mUniformBuffer.GetSize() )
        {
            mOffset = offset;
            throw std::runtime_error ( "Memory Pool Buffer cannot fulfill allocation request." );
        }
        return BufferAccessor{&mUniformBuffer, offset, aSize};
    }

    void VulkanMemoryPoolBuffer::Reset()
    {
        mOffset = 0;
    }

    const VkDescriptorSet& VulkanMemoryPoolBuffer::GetDescriptorSet() const
    {
        return mVkDescriptorSet;
    }
}
