/*
Copyright (C) 2017-2021 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufUtils.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "material.pb.h"
#include "pipeline.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/Material.h"
#include "aeongames/Vector2.h"
#include "aeongames/Vector3.h"
#include "aeongames/Vector4.h"
#include "VulkanMaterial.h"
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMaterial::VulkanMaterial ( const VulkanRenderer&  aVulkanRenderer, uint32_t aPath ) :
        Material{},
        mVulkanRenderer { aVulkanRenderer },
        mUniformBuffer { mVulkanRenderer }
    {
        if ( aPath )
        {
            Resource::Load ( aPath );
        }
    }

    VulkanMaterial::VulkanMaterial ( const VulkanRenderer&  aVulkanRenderer, std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers ) :
        Material{},
        mVulkanRenderer { aVulkanRenderer },
        mUniformBuffer { mVulkanRenderer }
    {
        VulkanMaterial::Load ( aUniforms, aSamplers );
    }

    VulkanMaterial::~VulkanMaterial()
    {
        Unload();
    }

    VulkanMaterial::VulkanMaterial ( const VulkanMaterial& aMaterial ) :
        Material{aMaterial},
        mVulkanRenderer{aMaterial.mVulkanRenderer},
        mUniformBuffer{aMaterial.mUniformBuffer}
    {
        InitializeDescriptorPool();
        InitializeDescriptorSets();
    }

    VulkanMaterial& VulkanMaterial::operator= ( const VulkanMaterial& aMaterial )
    {
        if ( &mVulkanRenderer != &aMaterial.mVulkanRenderer )
        {
            throw std::runtime_error ( "Assigning materials from different renderer instances." );
        }
        this->Material::operator= ( aMaterial );
        mUniformBuffer = aMaterial.mUniformBuffer;
        InitializeDescriptorPool();
        InitializeDescriptorSets();
        return *this;
    }

    std::unique_ptr<Material> VulkanMaterial::Clone() const
    {
        return std::make_unique<VulkanMaterial> ( *this );
    }

    void VulkanMaterial::Load ( std::initializer_list<UniformKeyValue> aUniforms, std::initializer_list<SamplerKeyValue> aSamplers )
    {
        size_t size = LoadVariables ( aUniforms );
        if ( size )
        {
            mUniformBuffer.Initialize (
                static_cast<VkDeviceSize> ( size ),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
            for ( auto& i : aUniforms )
            {
                Set ( i );
            }
        }
        LoadSamplers ( aSamplers );
        InitializeDescriptorPool();
        InitializeDescriptorSets();
    }

    void VulkanMaterial::Load ( const MaterialMsg& aMaterialMsg )
    {
        size_t size = LoadVariables ( aMaterialMsg );
        if ( size )
        {
            mUniformBuffer.Initialize (
                static_cast<VkDeviceSize> ( size ),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
            for ( auto& i : aMaterialMsg.property() )
            {
                Set ( PropertyToKeyValue ( i ) );
            }
        }
        LoadSamplers ( aMaterialMsg );
        InitializeDescriptorPool();
        InitializeDescriptorSets();
    }

    void VulkanMaterial::Unload()
    {
        FinalizeDescriptorSets();
        FinalizeDescriptorPool();
        mUniformBuffer.Finalize();
    }

    void VulkanMaterial::Set ( size_t aIndex, const UniformValue& aValue )
    {
        mUniformBuffer.WriteMemory ( mVariables.at ( aIndex ).GetOffset(), GetUniformValueSize ( aValue ), GetUniformValuePointer ( aValue ) );
    }

    void VulkanMaterial::Set ( const UniformKeyValue& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aValue] ( const UniformVariable & variable )
        {
            return variable.GetName() == std::get<std::string> ( aValue );
        } );
        if ( i != mVariables.end() )
        {
            size_t value_size = GetUniformValueSize ( std::get<UniformValue> ( aValue ) );
            // Do some bounds checking
            auto j = i + 1;
            if ( ( ( j != mVariables.end() ) ? ( j->GetOffset() - i->GetOffset() ) : ( mUniformBuffer.GetSize() - i->GetOffset() ) ) < value_size )
            {
                throw std::runtime_error ( "Value type size exceeds original type size." );
            }
            mUniformBuffer.WriteMemory ( i->GetOffset(), value_size, GetUniformValuePointer ( std::get<UniformValue> ( aValue ) ) );
        }
    }

    void VulkanMaterial::SetSampler ( const std::string& aName, const ResourceId& aValue )
    {
        auto i = std::find_if ( mSamplers.begin(), mSamplers.end(),
                                [&aName] ( const std::tuple<std::string, ResourceId>& aTuple )
        {
            return std::get<0> ( aTuple ) == aName;
        } );
        if ( i != mSamplers.end() )
        {
            std::get<1> ( *i ) = aValue;
        }
    }

    ResourceId VulkanMaterial::GetSampler ( const std::string& aName )
    {
        auto i = std::find_if ( mSamplers.begin(), mSamplers.end(),
                                [&aName] ( const std::tuple<std::string, ResourceId>& aTuple )
        {
            return std::get<0> ( aTuple ) == aName;
        } );
        if ( i != mSamplers.end() )
        {
            return std::get<1> ( *i );
        }
        return ResourceId{"Texture"_crc32, 0};
    }

    const VkDescriptorSet& VulkanMaterial::GetUniformDescriptorSet() const
    {
        return mUniformDescriptorSet;
    }

    const VkDescriptorSet& VulkanMaterial::GetSamplerDescriptorSet() const
    {
        return mSamplerDescriptorSet;
    }

    void VulkanMaterial::InitializeDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 2> descriptor_pool_sizes{};
        uint32_t descriptor_pool_size_count = 0;

        if ( mUniformBuffer.GetSize() )
        {
            descriptor_pool_sizes[descriptor_pool_size_count].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_pool_sizes[descriptor_pool_size_count++].descriptorCount = 1;
        }
        if ( mSamplers.size() )
        {
            descriptor_pool_sizes[descriptor_pool_size_count].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_pool_sizes[descriptor_pool_size_count++].descriptorCount = static_cast<uint32_t> ( mSamplers.size() );
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
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
    }

    void VulkanMaterial::InitializeDescriptorSets()
    {
        if ( !mUniformBuffer.GetSize() && !mSamplers.size() )
        {
            return;
        }
        if ( mUniformBuffer.GetSize() )
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
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        if ( mSamplers.size() )
        {
            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
            descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
            descriptor_set_allocate_info.descriptorSetCount = 1;
            descriptor_set_allocate_info.pSetLayouts = &mVulkanRenderer.GetSamplerDescriptorSetLayout ( mSamplers.size() );
            if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &mSamplerDescriptorSet ) )
            {
                std::ostringstream stream;
                stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }

        std::array<VkDescriptorBufferInfo, 1> descriptor_buffer_infos =
        {
            {
                VkDescriptorBufferInfo{mUniformBuffer.GetBuffer(), 0, mUniformBuffer.GetSize() }
            }
        };

        std::vector<VkWriteDescriptorSet> write_descriptor_sets{};
        write_descriptor_sets.reserve ( mUniformBuffer.GetSize() ? 1 : 0 + mSamplers.size() );

        if ( mUniformBuffer.GetSize() )
        {
            write_descriptor_sets.emplace_back();
            auto& write_descriptor_set = write_descriptor_sets.back();
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.pNext = nullptr;
            write_descriptor_set.dstSet = mUniformDescriptorSet;
            write_descriptor_set.dstBinding = 0;
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptor_set.descriptorCount = 1;
            write_descriptor_set.pBufferInfo = &descriptor_buffer_infos[0];
            write_descriptor_set.pImageInfo = nullptr;
            write_descriptor_set.pTexelBufferView = nullptr;
        }
        for ( uint32_t i = 0; i < mSamplers.size(); ++i )
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
            write_descriptor_set.pImageInfo = &reinterpret_cast<const VulkanTexture*> ( std::get<1> ( mSamplers[i] ).Get<Texture>() )->GetDescriptorImageInfo();
            write_descriptor_set.pTexelBufferView = nullptr;
        }
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
    }

    void VulkanMaterial::FinalizeDescriptorPool()
    {
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, nullptr );
            mVkDescriptorPool = VK_NULL_HANDLE;
        }
    }

    void VulkanMaterial::FinalizeDescriptorSets()
    {
        if ( mUniformDescriptorSet )
        {
            vkFreeDescriptorSets ( mVulkanRenderer.GetDevice(),
                                   mVkDescriptorPool,
                                   1,
                                   &mUniformDescriptorSet );
        }
        if ( mSamplerDescriptorSet )
        {
            vkFreeDescriptorSets ( mVulkanRenderer.GetDevice(),
                                   mVkDescriptorPool,
                                   1,
                                   &mSamplerDescriptorSet );
        }
    }

    const std::vector<std::tuple<std::string, ResourceId>>& VulkanMaterial::GetSamplers() const
    {
        return mSamplers;
    }
}
