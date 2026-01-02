/*
Copyright (C) 2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Vector2.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Vector4.hpp"
#include "aeongames/LogLevel.hpp"
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

    VulkanMemoryPoolBuffer::VulkanMemoryPoolBuffer ( VulkanMemoryPoolBuffer&& aVulkanMemoryPoolBuffer ) :
        mVulkanRenderer { aVulkanMemoryPoolBuffer.mVulkanRenderer },
        mUniformBuffer { std::move ( aVulkanMemoryPoolBuffer.mUniformBuffer ) }
    {
        std::swap ( mOffset, aVulkanMemoryPoolBuffer.mOffset );
        std::swap ( mVkDescriptorPool, aVulkanMemoryPoolBuffer.mVkDescriptorPool );
        std::swap ( mVkDescriptorSet, aVulkanMemoryPoolBuffer.mVkDescriptorSet );
    }

    VulkanMemoryPoolBuffer::VulkanMemoryPoolBuffer ( const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer { aVulkanRenderer }, mUniformBuffer{aVulkanRenderer} {}

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
        FinalizeDescriptorPool();
        mUniformBuffer.Finalize();
    }

    void VulkanMemoryPoolBuffer::InitializeDescriptorPool()
    {
        mVkDescriptorPool = CreateDescriptorPool ( mVulkanRenderer.GetDevice(), {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}} );
    }

    void VulkanMemoryPoolBuffer::InitializeDescriptorSet()
    {
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = 1;
        VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
        descriptor_set_layout_binding.binding = 0;
        descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptor_set_layout_binding.descriptorCount = 1;
        // Only vertex shaders will access these for now, add a flag for other stages if needed
        descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        descriptor_set_layout_binding.pImmutableSamplers = nullptr;
        descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
        mVkDescriptorSet = CreateDescriptorSet ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, mVulkanRenderer.GetDescriptorSetLayout ( descriptor_set_layout_create_info ) );

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

    BufferAccessor VulkanMemoryPoolBuffer::Allocate ( size_t aSize )
    {
        size_t offset = mOffset;
        mOffset += ( (  aSize - 1 ) | ( mVulkanRenderer.GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment - 1 ) ) + 1;
        if ( mOffset > mUniformBuffer.GetSize() )
        {
            mOffset = offset;
            std::cout << LogLevel::Error << "Memory Pool Buffer cannot fulfill allocation request." << std::endl;
            throw std::runtime_error ( "Memory Pool Buffer cannot fulfill allocation request." );
        }
        return BufferAccessor{this, offset, aSize};
    }

    void VulkanMemoryPoolBuffer::Reset()
    {
        mOffset = 0;
    }

    const VkDescriptorSet& VulkanMemoryPoolBuffer::GetDescriptorSet() const
    {
        return mVkDescriptorSet;
    }
    const Buffer& VulkanMemoryPoolBuffer::GetBuffer() const
    {
        return mUniformBuffer;
    }
}
