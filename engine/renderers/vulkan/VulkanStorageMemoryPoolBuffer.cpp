/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#include <iostream>
#include <stdexcept>
#include <utility>
#include "aeongames/LogLevel.hpp"
#include "VulkanStorageMemoryPoolBuffer.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanUtilities.hpp"

namespace AeonGames
{
    VulkanStorageMemoryPoolBuffer::VulkanStorageMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer, size_t aStackSize ) :
        mVulkanRenderer { aVulkanRenderer },
        mStorageBuffer { mVulkanRenderer,
                         ( ( aStackSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment - 1 ) ) + 1,
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT }
    {
        InitializeDescriptorPool();
        InitializeDescriptorSet();
    }

    VulkanStorageMemoryPoolBuffer::VulkanStorageMemoryPoolBuffer ( VulkanStorageMemoryPoolBuffer&& aVulkanStorageMemoryPoolBuffer ) :
        mVulkanRenderer { aVulkanStorageMemoryPoolBuffer.mVulkanRenderer },
        mStorageBuffer { std::move ( aVulkanStorageMemoryPoolBuffer.mStorageBuffer ) }
    {
        std::swap ( mOffset, aVulkanStorageMemoryPoolBuffer.mOffset );
        std::swap ( mVkDescriptorPool, aVulkanStorageMemoryPoolBuffer.mVkDescriptorPool );
        std::swap ( mVkDescriptorSet, aVulkanStorageMemoryPoolBuffer.mVkDescriptorSet );
    }

    VulkanStorageMemoryPoolBuffer::VulkanStorageMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer { aVulkanRenderer }, mStorageBuffer{aVulkanRenderer} {}

    void VulkanStorageMemoryPoolBuffer::Initialize ( size_t aStackSize )
    {
        mStorageBuffer.Initialize ( ( ( aStackSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment - 1 ) ) + 1,
                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
        InitializeDescriptorPool();
        InitializeDescriptorSet();
    }

    VulkanStorageMemoryPoolBuffer::~VulkanStorageMemoryPoolBuffer()
    {
        Finalize();
    }

    void VulkanStorageMemoryPoolBuffer::Finalize()
    {
        FinalizeDescriptorPool();
        mStorageBuffer.Finalize();
    }

    void VulkanStorageMemoryPoolBuffer::InitializeDescriptorPool()
    {
        mVkDescriptorPool = CreateDescriptorPool ( mVulkanRenderer.GetDevice(), {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1}} );
    }

    void VulkanStorageMemoryPoolBuffer::InitializeDescriptorSet()
    {
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
        descriptor_set_layout_binding.binding = 0;
        descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        descriptor_set_layout_binding.descriptorCount = 1;
        descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
        descriptor_set_layout_binding.pImmutableSamplers = nullptr;
        descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
        mVkDescriptorSet = CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( descriptor_set_layout_create_info ) );

        VkDescriptorBufferInfo descriptor_buffer_info = { mStorageBuffer.GetBuffer(), 0, mStorageBuffer.GetSize() };
        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstSet = mVkDescriptorSet;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
        write_descriptor_set.pImageInfo = nullptr;
        write_descriptor_set.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), 1, &write_descriptor_set, 0, nullptr );
    }

    void VulkanStorageMemoryPoolBuffer::FinalizeDescriptorPool()
    {
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, nullptr );
            mVkDescriptorPool = VK_NULL_HANDLE;
        }
    }

    BufferAccessor VulkanStorageMemoryPoolBuffer::Allocate ( size_t aSize )
    {
        size_t offset = mOffset;
        mOffset += ( ( aSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment - 1 ) ) + 1;
        if ( mOffset > mStorageBuffer.GetSize() )
        {
            mOffset = offset;
            std::cout << LogLevel::Error << "Storage Memory Pool Buffer cannot fulfill allocation request." << std::endl;
            throw std::runtime_error ( "Storage Memory Pool Buffer cannot fulfill allocation request." );
        }
        return BufferAccessor{this, offset, aSize};
    }

    void VulkanStorageMemoryPoolBuffer::Reset()
    {
        mOffset = 0;
    }

    const VkDescriptorSet& VulkanStorageMemoryPoolBuffer::GetDescriptorSet() const
    {
        return mVkDescriptorSet;
    }
    const Buffer& VulkanStorageMemoryPoolBuffer::GetBuffer() const
    {
        return mStorageBuffer;
    }
}
