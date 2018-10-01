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
#include <ostream>
#include <regex>
#include <array>
#include <utility>
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/Material.h"
#include "aeongames/ResourceCache.h"
#include "VulkanMaterial.h"
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMaterial::VulkanMaterial ( const Material& aMaterial, const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer ( aVulkanRenderer ),
        mMaterial ( aMaterial ), mPropertiesBuffer ( mVulkanRenderer )
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

    VulkanMaterial::~VulkanMaterial()
    {
        Finalize();
    }

    void VulkanMaterial::Update ( const uint8_t* aValue, size_t aOffset, size_t aSize )
    {
        mPropertiesBuffer.WriteMemory ( aOffset, ( aSize ) ? aSize : mMaterial.GetPropertyBlock().size() - aOffset, aValue );
    }

    const VkDescriptorSetLayout& VulkanMaterial::GetPropertiesDescriptorSetLayout() const
    {
        return mVkPropertiesDescriptorSetLayout;
    }

    const VkDescriptorSet VulkanMaterial::GetPropertiesDescriptorSet() const
    {
        return mVkPropertiesDescriptorSet;
    }

    const std::vector<std::shared_ptr<VulkanTexture>>& VulkanMaterial::GetTextures() const
    {
        return mTextures;
    }

    void VulkanMaterial::Initialize()
    {
        InitializeDescriptorSetLayout();
        InitializeDescriptorPool();
        InitializePropertiesUniform();
        InitializeDescriptorSet();
        for ( auto& i : mMaterial.GetProperties() )
        {
            if ( i.GetType() == Material::SAMPLER_2D )
            {
                mTextures.emplace_back ( std::make_shared<VulkanTexture> ( *i.GetImage(), mVulkanRenderer ) );
            }
        }
    }

    void VulkanMaterial::Finalize()
    {
        FinalizeDescriptorSet();
        FinalizePropertiesUniform();
        FinalizeDescriptorPool();
        FinalizeDescriptorSetLayout();
    }

    void VulkanMaterial::InitializeDescriptorSetLayout()
    {
        if ( !mMaterial.GetProperties().size() )
        {
            // We don' need a layout.
            return;
        }
        std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
        descriptor_set_layout_bindings.reserve ( 1 + mMaterial.GetSamplerCount() );
        uint32_t binding = 0;

        if ( mMaterial.GetPropertyBlock().size() )
        {
            descriptor_set_layout_bindings.emplace_back();
            descriptor_set_layout_bindings.back().binding = binding++;
            descriptor_set_layout_bindings.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            /* We will bind just 1 UBO, descriptor count is the number of array elements, and we just use a single struct. */
            descriptor_set_layout_bindings.back().descriptorCount = 1;
            descriptor_set_layout_bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
            descriptor_set_layout_bindings.back().pImmutableSamplers = nullptr;
        }

        for ( auto& i : mMaterial.GetProperties() )
        {
            if ( i.GetType() == Material::PropertyType::SAMPLER_2D )
            {
                descriptor_set_layout_bindings.emplace_back();
                auto& descriptor_set_layout_binding = descriptor_set_layout_bindings.back();
                descriptor_set_layout_binding.binding = binding++;
                descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                // Descriptor Count is the count of elements in an array.
                descriptor_set_layout_binding.descriptorCount = 1;
                descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                descriptor_set_layout_binding.pImmutableSamplers = nullptr;
            }
        }
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.pNext = nullptr;
        descriptor_set_layout_create_info.flags = 0;
        descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t> ( descriptor_set_layout_bindings.size() );
        descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings.data();
        if ( VkResult result = vkCreateDescriptorSetLayout ( mVulkanRenderer.GetDevice(), &descriptor_set_layout_create_info, nullptr, &mVkPropertiesDescriptorSetLayout ) )
        {
            std::ostringstream stream;
            stream << "DescriptorSet Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanMaterial::FinalizeDescriptorSetLayout()
    {
        if ( mVkPropertiesDescriptorSetLayout != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorSetLayout ( mVulkanRenderer.GetDevice(), mVkPropertiesDescriptorSetLayout, nullptr );
            mVkPropertiesDescriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    void VulkanMaterial::InitializeDescriptorPool()
    {
        std::vector<VkDescriptorPoolSize> descriptor_pool_sizes{};
        descriptor_pool_sizes.reserve ( 2 );
        if ( mMaterial.GetPropertyBlock().size() )
        {
            descriptor_pool_sizes.emplace_back();
            descriptor_pool_sizes.back().type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_pool_sizes.back().descriptorCount = 1;
        }
        if ( mMaterial.GetSamplerCount() )
        {
            descriptor_pool_sizes.emplace_back();
            descriptor_pool_sizes.back().type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_pool_sizes.back().descriptorCount = static_cast<uint32_t> ( mMaterial.GetSamplerCount() );
        }
        if ( descriptor_pool_sizes.size() )
        {
            VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
            descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptor_pool_create_info.pNext = nullptr;
            descriptor_pool_create_info.flags = 0;
            descriptor_pool_create_info.maxSets = 1;
            descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t> ( descriptor_pool_sizes.size() );
            descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
            if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer.GetDevice(), &descriptor_pool_create_info, nullptr, &mVkPropertiesDescriptorPool ) )
            {
                std::ostringstream stream;
                stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
    }

    void VulkanMaterial::FinalizeDescriptorPool()
    {
        if ( mVkPropertiesDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mVkPropertiesDescriptorPool, nullptr );
            mVkPropertiesDescriptorPool = VK_NULL_HANDLE;
        }
    }

    void VulkanMaterial::InitializePropertiesUniform()
    {
        if ( mMaterial.GetPropertyBlock().size() )
        {
            mPropertiesBuffer.Initialize (
                mMaterial.GetPropertyBlock().size(),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                static_cast<const void*> ( mMaterial.GetPropertyBlock().data() ) );
        }
    }

    void VulkanMaterial::FinalizePropertiesUniform()
    {
    }

    void VulkanMaterial::InitializeDescriptorSet()
    {
        std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts{ { mVkPropertiesDescriptorSetLayout } };
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = mVkPropertiesDescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = static_cast<uint32_t> ( descriptor_set_layouts.size() );
        descriptor_set_allocate_info.pSetLayouts = descriptor_set_layouts.data();
        if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &mVkPropertiesDescriptorSet ) )
        {
            std::ostringstream stream;
            stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        std::array<VkDescriptorBufferInfo, 1> descriptor_buffer_infos =
        {
            {
                VkDescriptorBufferInfo{mPropertiesBuffer.GetBuffer(), 0, mMaterial.GetPropertyBlock().size() }
            }
        };

        std::vector<VkWriteDescriptorSet> write_descriptor_sets{};
        write_descriptor_sets.reserve ( 1 + mMaterial.GetSamplerCount() );

        if ( mMaterial.GetPropertyBlock().size() )
        {
            write_descriptor_sets.emplace_back();
            auto& write_descriptor_set = write_descriptor_sets.back();
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.pNext = nullptr;
            write_descriptor_set.dstSet = mVkPropertiesDescriptorSet;
            write_descriptor_set.dstBinding = 0;
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptor_set.descriptorCount = 1;
            write_descriptor_set.pBufferInfo = &descriptor_buffer_infos[0];
            write_descriptor_set.pImageInfo = nullptr;
            write_descriptor_set.pTexelBufferView = nullptr;
        }

        for ( uint32_t i = 0; i < mMaterial.GetSamplerCount(); ++i )
        {
            write_descriptor_sets.emplace_back();
            auto& write_descriptor_set = write_descriptor_sets.back();
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.pNext = nullptr;
            write_descriptor_set.dstSet = mVkPropertiesDescriptorSet;
            ///@todo find a better way to calculate sampler binding, perhaps move samplers to their own descriptor set.
            write_descriptor_set.dstBinding = static_cast<uint32_t> ( i + ( write_descriptor_sets.size() - 1 ) );
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptor_set.descriptorCount = 1;
            write_descriptor_set.pBufferInfo = nullptr;
            write_descriptor_set.pImageInfo = &mTextures[i]->GetDescriptorImageInfo();
        }
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
    }

    void VulkanMaterial::FinalizeDescriptorSet()
    {
    }
}
