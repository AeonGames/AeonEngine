/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
    VulkanMaterial::VulkanMaterial ( const std::shared_ptr<const Material> aMaterial, const std::shared_ptr<const VulkanRenderer> aVulkanRenderer ) :
        mVulkanRenderer ( aVulkanRenderer ),
        mMaterial ( aMaterial )
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

    const std::vector<uint8_t>& VulkanMaterial::GetUniformData() const
    {
        return mUniformData;
    }

    const std::vector<std::shared_ptr<VulkanTexture>>& VulkanMaterial::GetTextures() const
    {
        return mTextures;
    }

    const VkDescriptorSet& VulkanMaterial::GetDescriptorSet() const
    {
        return mVkDescriptorSet;
    }

    const VkDescriptorSetLayout & VulkanMaterial::GetDescriptorSetLayout() const
    {
        return mVkDescriptorSetLayout;
    }

    void VulkanMaterial::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        if ( !mMaterial )
        {
            throw std::runtime_error ( "Material cannot be null." );
        }
        mUniformData.resize ( mMaterial->GetUniformBlockSize() );
        uint32_t offset = 0;
        for ( auto& i : mMaterial->GetUniformMetaData() )
        {
            uint32_t advance = 0;
            switch ( i.GetType() )
            {
            case Uniform::FLOAT_VEC4:
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 3 ) = i.GetW();
            /* Intentional Pass-Thru */
            case Uniform::FLOAT_VEC3:
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 2 ) = i.GetZ();
                advance += sizeof ( float ) * 2; /* Both VEC3 and VEC4 have a 4 float stride due to std140 padding. */
            /* Intentional Pass-Thru */
            case Uniform::FLOAT_VEC2:
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 1 ) = i.GetY();
                advance += sizeof ( float );
            /* Intentional Pass-Thru */
            case Uniform::FLOAT:
                * ( reinterpret_cast<float*> ( mUniformData.data() + offset ) + 0 ) = i.GetX();
                advance += sizeof ( float );
                break;
            case Uniform::SAMPLER_2D:
                mTextures.emplace_back ( Get<VulkanTexture> ( i.GetImage().get(), i.GetImage(), mVulkanRenderer ) );
                break;
            default:
                break;
            }
            offset += advance;
        }
        InitializeDescriptorSetLayout();
        InitializeDescriptorPool();
        InitializeDescriptorSet();
    }
    void VulkanMaterial::Finalize()
    {
        FinalizeDescriptorSet();
        FinalizeDescriptorPool();
        FinalizeDescriptorSetLayout();
    }
    void VulkanMaterial::InitializeDescriptorSetLayout()
    {
        if ( !mMaterial->GetUniformMetaData().size() )
        {
            return;
        }
        std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
        /*  Reserve enough slots as if all uniforms where shaders, better safe than sorry.*/
        descriptor_set_layout_bindings.reserve ( mMaterial->GetUniformMetaData().size() );

        for ( auto& i : mMaterial->GetUniformMetaData() )
        {
            if ( i.GetType() == Uniform::Type::SAMPLER_2D )
            {
                descriptor_set_layout_bindings.emplace_back();
                auto& descriptor_set_layout_binding = descriptor_set_layout_bindings.back();
                descriptor_set_layout_binding.binding = static_cast<uint32_t> ( descriptor_set_layout_bindings.size() - 1 );
                // Descriptor Count is the count of elements in an array.
                descriptor_set_layout_binding.descriptorCount = 1;
                descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_set_layout_binding.pImmutableSamplers = nullptr;
                descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            }
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.pNext = nullptr;
        descriptor_set_layout_create_info.flags = 0;
        descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t> ( descriptor_set_layout_bindings.size() );
        descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings.data();
        if ( VkResult result = vkCreateDescriptorSetLayout ( mVulkanRenderer->GetDevice(), &descriptor_set_layout_create_info, nullptr, &mVkDescriptorSetLayout ) )
        {
            std::ostringstream stream;
            stream << "DescriptorSet Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanMaterial::FinalizeDescriptorSetLayout()
    {
        if ( mVkDescriptorSetLayout != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorSetLayout ( mVulkanRenderer->GetDevice(), mVkDescriptorSetLayout, nullptr );
            mVkDescriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    void VulkanMaterial::InitializeDescriptorPool()
    {
        if ( !mMaterial->GetUniformMetaData().size() )
        {
            return;
        }
        uint32_t sampler_descriptor_count = 0;
        for ( auto&i : mMaterial->GetUniformMetaData() )
        {
            if ( i.GetType() == Uniform::Type::SAMPLER_2D )
            {
                ++sampler_descriptor_count;
            }
        }
        if ( !sampler_descriptor_count )
        {
            return;
        }
        std::vector<VkDescriptorPoolSize> descriptor_pool_sizes{};
        descriptor_pool_sizes.reserve ( sampler_descriptor_count );

        descriptor_pool_sizes.emplace_back();
        auto& descriptor_pool_size = descriptor_pool_sizes.back();
        descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_pool_size.descriptorCount = sampler_descriptor_count;

        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.pNext = nullptr;
        descriptor_pool_create_info.flags = 0;
        descriptor_pool_create_info.maxSets = 1;
        descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t> ( descriptor_pool_sizes.size() );
        descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();

        if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer->GetDevice(), &descriptor_pool_create_info, nullptr, &mVkDescriptorPool ) )
        {
            std::ostringstream stream;
            stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanMaterial::FinalizeDescriptorPool()
    {
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer->GetDevice(), mVkDescriptorPool, nullptr );
            mVkDescriptorPool = VK_NULL_HANDLE;
        }
    }

    void VulkanMaterial::InitializeDescriptorSet()
    {
        if ( mVkDescriptorPool == VK_NULL_HANDLE )
        {
            return;
        }
        std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts{ { mVkDescriptorSetLayout } };
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = static_cast<uint32_t> ( descriptor_set_layouts.size() );
        descriptor_set_allocate_info.pSetLayouts = descriptor_set_layouts.data();
        if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer->GetDevice(), &descriptor_set_allocate_info, &mVkDescriptorSet ) )
        {
            std::ostringstream stream;
            stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        std::vector<VkWriteDescriptorSet> write_descriptor_sets;
        write_descriptor_sets.reserve ( mMaterial->GetUniformMetaData().size() );

        for ( uint32_t i = 0; i < mTextures.size(); ++i )
        {
            write_descriptor_sets.emplace_back();
            auto& write_descriptor_set = write_descriptor_sets.back();
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.pNext = nullptr;
            write_descriptor_set.dstSet = mVkDescriptorSet;
            write_descriptor_set.dstBinding = static_cast<uint32_t> ( i );
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptor_set.descriptorCount = 1;
            write_descriptor_set.pImageInfo = &mTextures[i]->GetDescriptorImageInfo();
        }
        vkUpdateDescriptorSets ( mVulkanRenderer->GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
    }

    void VulkanMaterial::FinalizeDescriptorSet()
    {
    }
}
