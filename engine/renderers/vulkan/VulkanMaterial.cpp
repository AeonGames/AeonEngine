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
    VulkanMaterial::VulkanMaterial ( const Material& aMaterial, const std::shared_ptr<const VulkanRenderer>&  aVulkanRenderer ) :
        mVulkanRenderer ( aVulkanRenderer ),
        mMaterial ( aMaterial ), mPropertiesBuffer ( *mVulkanRenderer )
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
        = default;

    void VulkanMaterial::Update ( const uint8_t* aValue, size_t aOffset, size_t aSize )
    {
    }

    const VkDescriptorSetLayout& VulkanMaterial::GetPropertiesDescriptorSetLayout() const
    {
        return mVkPropertiesDescriptorSetLayout;
    }

    const std::vector<std::shared_ptr<VulkanTexture>>& VulkanMaterial::GetTextures() const
    {
        return mTextures;
    }

    void VulkanMaterial::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        InitializeDescriptorSetLayout();
        InitializeDescriptorPool();
        InitializePropertiesUniform();
#if 0
        for ( auto& i : mMaterial.GetProperties() )
        {
            switch ( i.GetType() )
            {
            case Material::SAMPLER_2D:
                ///@todo to be reworked.
                //mTextures.emplace_back ( Get<VulkanTexture> ( i.GetImage().get(), i.GetImage(), mVulkanRenderer ) );
                break;
            default:
                break;
            }
        }
#endif
    }
    void VulkanMaterial::Finalize()
    {
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
        for ( auto& i : mMaterial.GetProperties() )
        {
            if ( i.GetType() != Material::PropertyType::SAMPLER_2D )
            {
                descriptor_set_layout_bindings.emplace_back();
                descriptor_set_layout_bindings.back().binding = binding++;
                descriptor_set_layout_bindings.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                /* We will bind just 1 UBO, descriptor count is the number of array elements, and we just use a single struct. */
                descriptor_set_layout_bindings.back().descriptorCount = 1;
                descriptor_set_layout_bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
                descriptor_set_layout_bindings.back().pImmutableSamplers = nullptr;
            }
            else
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
        if ( VkResult result = vkCreateDescriptorSetLayout ( mVulkanRenderer->GetDevice(), &descriptor_set_layout_create_info, nullptr, &mVkPropertiesDescriptorSetLayout ) )
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
            vkDestroyDescriptorSetLayout ( mVulkanRenderer->GetDevice(), mVkPropertiesDescriptorSetLayout, nullptr );
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
            if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer->GetDevice(), &descriptor_pool_create_info, nullptr, &mVkPropertiesDescriptorPool ) )
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
            vkDestroyDescriptorPool ( mVulkanRenderer->GetDevice(), mVkPropertiesDescriptorPool, nullptr );
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
}
