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
    }
    void VulkanMaterial::Finalize()
    {
    }

    void VulkanMaterial::InitializeDescriptorSetLayout()
    {
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

}
