/*
Copyright (C) 2017-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/ProtoBufUtils.hpp"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "material.pb.h"
#include "pipeline.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/AeonEngine.hpp"
#include "aeongames/CRC.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/Vector2.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Vector4.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/LogLevel.hpp"
#include "VulkanMaterial.h"
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{

    void VulkanMaterial::Initialize ( const VulkanPipeline& aVulkanPipeline )
    {
        if ( mVkDescriptorPool != VK_NULL_HANDLE || mUniformDescriptorSet != VK_NULL_HANDLE || mSamplerDescriptorSet != VK_NULL_HANDLE )
        {
            std::cout << LogLevel::Error << "VulkanMaterial: Already initialized." << std::endl;
            throw std::runtime_error ( "VulkanMaterial: Already initialized." );
        }

        std::array<VkDescriptorPoolSize, 2> descriptor_pool_sizes{};
        uint32_t descriptor_pool_size_count{0};
        VkDescriptorSetLayout material_descriptor_set_layout {aVulkanPipeline.GetDescriptorSetLayout ( Mesh::BindingLocations::MATERIAL ) };
        if ( material_descriptor_set_layout != VK_NULL_HANDLE )
        {
            descriptor_pool_sizes[descriptor_pool_size_count].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_pool_sizes[descriptor_pool_size_count++].descriptorCount = 1;
        }

        VkDescriptorSetLayout sampler_descriptor_set_layout  {aVulkanPipeline.GetDescriptorSetLayout ( Mesh::BindingLocations::SAMPLERS ) };
        if ( sampler_descriptor_set_layout != VK_NULL_HANDLE )
        {
            descriptor_pool_sizes[descriptor_pool_size_count].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_pool_sizes[descriptor_pool_size_count++].descriptorCount = static_cast<uint32_t> ( mMaterial->GetSamplers().size() );
        }

        std::vector<VkWriteDescriptorSet> write_descriptor_sets{};
        write_descriptor_sets.reserve ( mMaterial->GetSamplers().size() + 1 );

        if ( descriptor_pool_size_count )
        {
            VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
            descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptor_pool_create_info.pNext = nullptr;
            descriptor_pool_create_info.flags = 0;
            descriptor_pool_create_info.maxSets = descriptor_pool_size_count;
            descriptor_pool_create_info.poolSizeCount = descriptor_pool_size_count;
            descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
            if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer.GetDevice(), &descriptor_pool_create_info, nullptr, &mVkDescriptorPool ) )
            {
                std::ostringstream stream;
                stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }
        }

        if ( material_descriptor_set_layout != VK_NULL_HANDLE )
        {
            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
            descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
            descriptor_set_allocate_info.descriptorSetCount = 1;
            descriptor_set_allocate_info.pSetLayouts = &material_descriptor_set_layout;
            if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &mUniformDescriptorSet ) )
            {
                std::ostringstream stream;
                stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }
            mUniformBuffer.Initialize (
                mMaterial->GetUniformBuffer().size(),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                mMaterial->GetUniformBuffer().data() );
            VkDescriptorBufferInfo descriptor_buffer_info
            {
                mUniformBuffer.GetBuffer(),
                              0,
                              mMaterial->GetUniformBuffer().size()
            };
            write_descriptor_sets.emplace_back();
            auto& write_descriptor_set = write_descriptor_sets.back();
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.pNext = nullptr;
            write_descriptor_set.dstSet = mUniformDescriptorSet;
            write_descriptor_set.dstBinding = 0;
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptor_set.descriptorCount = 1;
            write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
            write_descriptor_set.pImageInfo = nullptr;
            write_descriptor_set.pTexelBufferView = nullptr;
        }

        if ( sampler_descriptor_set_layout != VK_NULL_HANDLE )
        {
            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
            descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
            descriptor_set_allocate_info.descriptorSetCount = 1;
            descriptor_set_allocate_info.pSetLayouts = &sampler_descriptor_set_layout;
            if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &mSamplerDescriptorSet ) )
            {
                std::ostringstream stream;
                stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }

            for ( uint32_t i = 0; i < mMaterial->GetSamplers().size(); ++i )
            {
                write_descriptor_sets.emplace_back();
                auto& write_descriptor_set = write_descriptor_sets.back();
                write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor_set.pNext = nullptr;
                // Note that the descriptor set does not change, we are setting multiple bindings on a single descriptor set.
                write_descriptor_set.dstSet = mSamplerDescriptorSet;
                write_descriptor_set.dstBinding = i;
                write_descriptor_set.dstArrayElement = 0;
                write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write_descriptor_set.descriptorCount = 1;
                write_descriptor_set.pBufferInfo = nullptr;
                write_descriptor_set.pImageInfo = mVulkanRenderer.GetTextureDescriptorImageInfo ( *std::get<1> ( mMaterial->GetSamplers() [i] ).Get<Texture>() );
                write_descriptor_set.pTexelBufferView = nullptr;
            }
        }
        if ( !write_descriptor_sets.empty() )
        {
            vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
        }
    }

    VulkanMaterial::VulkanMaterial ( VulkanRenderer&  aVulkanRenderer, const Pipeline& aPipeline, const Material& aMaterial ) :
        mVulkanRenderer { aVulkanRenderer },
        mMaterial { &aMaterial },
        mUniformBuffer { mVulkanRenderer }
    {
        const VulkanPipeline* vulkan_pipeline = mVulkanRenderer.GetVulkanPipeline ( aPipeline );
        if ( vulkan_pipeline == nullptr )
        {
            std::cout << LogLevel::Error << "VulkanMaterial: Pipeline not loaded in VulkanRenderer." << std::endl;
            throw std::runtime_error ( "VulkanMaterial: Pipeline not loaded in VulkanRenderer." );
        }
        Initialize ( *vulkan_pipeline );
    }

    void VulkanMaterial::Finalize ()
    {
        // Finalize Descriptor Pool
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, nullptr );
        }
        mUniformBuffer.Finalize();
    }

    VulkanMaterial::VulkanMaterial ( VulkanRenderer&  aVulkanRenderer, const Material& aMaterial ) :
        mVulkanRenderer { aVulkanRenderer },
        mMaterial { &aMaterial },
        mUniformBuffer { mVulkanRenderer }
    {
#if 0
        // Initialize DescriptorPool
        {
            std::array<VkDescriptorPoolSize, 2> descriptor_pool_sizes{};
            uint32_t descriptor_pool_size_count = 0;

            if ( aMaterial.GetUniformBuffer().size() )
            {
                descriptor_pool_sizes[descriptor_pool_size_count].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptor_pool_sizes[descriptor_pool_size_count++].descriptorCount = 1;
            }
            if ( aMaterial.GetSamplers().size() )
            {
                descriptor_pool_sizes[descriptor_pool_size_count].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_pool_sizes[descriptor_pool_size_count++].descriptorCount = static_cast<uint32_t> ( aMaterial.GetSamplers().size() );
            }
            if ( descriptor_pool_size_count )
            {
                VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
                descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                descriptor_pool_create_info.pNext = nullptr;
                descriptor_pool_create_info.flags = 0;
                descriptor_pool_create_info.maxSets = descriptor_pool_size_count;
                descriptor_pool_create_info.poolSizeCount = descriptor_pool_size_count;
                descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
                if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer.GetDevice(), &descriptor_pool_create_info, nullptr, &mVkDescriptorPool ) )
                {
                    std::ostringstream stream;
                    stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
                    std::cout << LogLevel::Error << stream.str() << std::endl;
                    throw std::runtime_error ( stream.str().c_str() );
                }
            }
        }
        ///@kwizatz Haderach Commented out while new pipeline code is set up.
        // Initialize Descriptor Sets
        {
            if ( aMaterial.GetUniformBuffer().size() )
            {
                VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
                descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
                descriptor_set_allocate_info.descriptorSetCount = 1;
                descriptor_set_allocate_info.pSetLayouts = &mVulkanRenderer.GetUniformBufferDescriptorSetLayout();
                if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &mUniformDescriptorSet ) )
                {
                    std::ostringstream stream;
                    stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
                    std::cout << LogLevel::Error << stream.str() << std::endl;
                    throw std::runtime_error ( stream.str().c_str() );
                }
            }
            if ( aMaterial.GetSamplers().size() )
            {
                VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
                descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
                descriptor_set_allocate_info.descriptorSetCount = 1;
                descriptor_set_allocate_info.pSetLayouts = &mVulkanRenderer.GetSamplerDescriptorSetLayout ( aMaterial.GetSamplers().size() );
                if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &mSamplerDescriptorSet ) )
                {
                    std::ostringstream stream;
                    stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
                    std::cout << LogLevel::Error << stream.str() << std::endl;
                    throw std::runtime_error ( stream.str().c_str() );
                }
            }

            /*-----------------------------------------------------------------*/
            std::vector<VkWriteDescriptorSet> write_descriptor_sets{};
            write_descriptor_sets.reserve ( ( aMaterial.GetUniformBuffer().size() ? 1 : 0 ) + aMaterial.GetSamplers().size() );

            if ( aMaterial.GetUniformBuffer().size() )
            {
                mUniformBuffer.Initialize (
                    aMaterial.GetUniformBuffer().size(),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    aMaterial.GetUniformBuffer().data() );
                VkDescriptorBufferInfo descriptor_buffer_info
                {
                    mUniformBuffer.GetBuffer(),
                                  0,
                                  aMaterial.GetUniformBuffer().size()
                };
                write_descriptor_sets.emplace_back();
                auto& write_descriptor_set = write_descriptor_sets.back();
                write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor_set.pNext = nullptr;
                write_descriptor_set.dstSet = mUniformDescriptorSet;
                write_descriptor_set.dstBinding = 0;
                write_descriptor_set.dstArrayElement = 0;
                write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write_descriptor_set.descriptorCount = 1;
                write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
                write_descriptor_set.pImageInfo = nullptr;
                write_descriptor_set.pTexelBufferView = nullptr;
            }
            for ( uint32_t i = 0; i < aMaterial.GetSamplers().size(); ++i )
            {
                write_descriptor_sets.emplace_back();
                auto& write_descriptor_set = write_descriptor_sets.back();
                write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor_set.pNext = nullptr;
                // Note that the descriptor set does not change, we are setting multiple bindings on a single descriptor set.
                write_descriptor_set.dstSet = mSamplerDescriptorSet;
                write_descriptor_set.dstBinding = i;
                write_descriptor_set.dstArrayElement = 0;
                write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write_descriptor_set.descriptorCount = 1;
                write_descriptor_set.pBufferInfo = nullptr;
                write_descriptor_set.pImageInfo = mVulkanRenderer.GetTextureDescriptorImageInfo ( *std::get<1> ( aMaterial.GetSamplers() [i] ).Get<Texture>() );
                write_descriptor_set.pTexelBufferView = nullptr;
            }
            vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
        }
#endif
    }

    VulkanMaterial::~VulkanMaterial()
    {
        Finalize ();
    }

    VulkanMaterial::VulkanMaterial ( VulkanMaterial&& aVulkanMaterial ) :
        mVulkanRenderer{aVulkanMaterial.mVulkanRenderer},
        mMaterial{aVulkanMaterial.mMaterial},
        mUniformBuffer{std::move ( aVulkanMaterial.mUniformBuffer ) }
    {
        std::swap ( mVkDescriptorPool, aVulkanMaterial.mVkDescriptorPool );
        std::swap ( mUniformDescriptorSet, aVulkanMaterial.mUniformDescriptorSet );
        std::swap ( mSamplerDescriptorSet, aVulkanMaterial.mSamplerDescriptorSet );
    }

    void VulkanMaterial::Bind ( VkCommandBuffer aVkCommandBuffer, const VulkanPipeline& aPipeline  ) const
    {
        if ( mVkDescriptorPool == VK_NULL_HANDLE && mUniformDescriptorSet == VK_NULL_HANDLE && mSamplerDescriptorSet == VK_NULL_HANDLE )
        {
            /* I dont like that currently this is the only place where initialization happens.
                Initialization should rather be only at construction time, so this is
                here only temporarily while I get a render back on screen. */
            std::cout << LogLevel::Debug << "VulkanMaterial: Late Descriptor Pool and Set Initialization." << std::endl;
            const_cast<VulkanMaterial*> ( this )->Initialize ( aPipeline );
        }

        if ( uint32_t material_set_index = aPipeline.GetDescriptorSetIndex ( Mesh::BindingLocations::MATERIAL ); material_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( aVkCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      aPipeline.GetPipelineLayout(),
                                      material_set_index,
                                      1,
                                      &mUniformDescriptorSet, 0, nullptr );
        }
        if ( uint32_t sampler_set_index = aPipeline.GetDescriptorSetIndex ( Mesh::BindingLocations::SAMPLERS ); sampler_set_index != std::numeric_limits<uint32_t>::max() )
        {
            vkCmdBindDescriptorSets ( aVkCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      aPipeline.GetPipelineLayout(),
                                      sampler_set_index,
                                      1,
                                      &mSamplerDescriptorSet, 0, nullptr );
        }
    }
}
