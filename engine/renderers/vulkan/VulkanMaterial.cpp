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
#include <algorithm>
#include <cstring>
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
#include "aeongames/GpuMaterial.hpp"
#include "VulkanMaterial.hpp"
#include "VulkanTexture.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanUtilities.hpp"

namespace AeonGames
{
    static VkFilter ToVulkanFilter ( Material::SamplerFilter aFilter )
    {
        return aFilter == Material::SamplerFilter::NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    }

    static VkSamplerMipmapMode ToVulkanMipmapMode ( Material::SamplerMipmapMode aMode )
    {
        return aMode == Material::SamplerMipmapMode::NEAREST ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    static VkSamplerAddressMode ToVulkanAddressMode ( Material::SamplerAddressMode aMode )
    {
        switch ( aMode )
        {
        case Material::SamplerAddressMode::MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case Material::SamplerAddressMode::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case Material::SamplerAddressMode::CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case Material::SamplerAddressMode::REPEAT:
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    static VkCompareOp ToVulkanCompareOp ( Material::SamplerCompareOp aOperation )
    {
        static constexpr std::array<VkCompareOp, 8> operations
        {
            VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_EQUAL,
            VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER, VK_COMPARE_OP_NOT_EQUAL,
            VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_ALWAYS
        };
        const auto index = static_cast<size_t> ( aOperation );
        return index < operations.size() ? operations[index] : VK_COMPARE_OP_NEVER;
    }

    static VkBorderColor ToVulkanBorderColor ( Material::SamplerBorderColor aColor )
    {
        switch ( aColor )
        {
        case Material::SamplerBorderColor::OPAQUE_BLACK:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case Material::SamplerBorderColor::OPAQUE_WHITE:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        case Material::SamplerBorderColor::TRANSPARENT_BLACK:
        default:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        }
    }

    static VkSampler CreateMaterialSampler ( const VulkanRenderer& aRenderer, const Material::SamplerState& aState )
    {
        VkSamplerCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_info.magFilter = ToVulkanFilter ( aState.mag_filter );
        create_info.minFilter = ToVulkanFilter ( aState.min_filter );
        create_info.mipmapMode = ToVulkanMipmapMode ( aState.mipmap_mode );
        create_info.addressModeU = ToVulkanAddressMode ( aState.address_mode_u );
        create_info.addressModeV = ToVulkanAddressMode ( aState.address_mode_v );
        create_info.addressModeW = ToVulkanAddressMode ( aState.address_mode_w );
        create_info.mipLodBias = aState.mip_lod_bias;
        create_info.anisotropyEnable = aState.anisotropy_enable ? VK_TRUE : VK_FALSE;
        create_info.maxAnisotropy = aState.anisotropy_enable ? aState.max_anisotropy : 1.0f;
        create_info.compareEnable = aState.compare_enable ? VK_TRUE : VK_FALSE;
        create_info.compareOp = ToVulkanCompareOp ( aState.compare_op );
        create_info.minLod = aState.min_lod;
        create_info.maxLod = aState.max_lod;
        create_info.borderColor = ToVulkanBorderColor ( aState.border_color );
        VkSampler sampler = VK_NULL_HANDLE;
        if ( VkResult result = vkCreateSampler ( aRenderer.GetDevice(), &create_info, nullptr, &sampler ) )
        {
            std::ostringstream stream;
            stream << "Create material sampler failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
        return sampler;
    }

    VulkanMaterial::VulkanMaterial ( VulkanRenderer&  aVulkanRenderer, const Material& aMaterial ) :
        mVulkanRenderer { aVulkanRenderer },
        mMaterial { &aMaterial },
        mUniformBuffer { mVulkanRenderer }
    {
        mBindlessSamplerSlots.fill ( UINT32_MAX );
        if ( mVkDescriptorPool != VK_NULL_HANDLE || mUniformDescriptorSet != VK_NULL_HANDLE || mSamplerDescriptorSet != VK_NULL_HANDLE )
        {
            std::cout << LogLevel::Error << "VulkanMaterial: Already initialized." << std::endl;
            throw std::runtime_error ( "VulkanMaterial: Already initialized." );
        }

        for ( size_t slot = 0; slot < mSamplers.size(); ++slot )
        {
            Material::SamplerState state{};
            const uint32_t slot_crc = crc32i ( kMaterialSamplerSlots[slot].name, std::strlen ( kMaterialSamplerSlots[slot].name ) );
            for ( const auto& sampler : mMaterial->GetSamplers() )
            {
                if ( std::get<0> ( sampler ) == slot_crc )
                {
                    state = std::get<2> ( sampler );
                    break;
                }
            }
            mSamplers[slot] = CreateMaterialSampler ( mVulkanRenderer, state );
        }

        std::vector<VkDescriptorPoolSize> descriptor_pool_sizes{};
        descriptor_pool_sizes.reserve ( 2 );

        if ( mMaterial->GetUniformBuffer().size() > 0 )
        {
            descriptor_pool_sizes.push_back ( {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1} );
        }
        // Every material presents the full canonical material-sampler set
        // (kMaterialSamplerSlots) so its descriptor set layout always matches a
        // material-sampling pipeline, regardless of which samplers the material
        // actually declares. Slots the material omits are padded below with the
        // slot's fallback texture (white diffuse / flat normal).
        const uint32_t sampler_binding_count = static_cast<uint32_t> ( kMaterialSamplerSlots.size() );
        descriptor_pool_sizes.push_back ( {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sampler_binding_count } );

        if ( !descriptor_pool_sizes.empty() )
        {
            mVkDescriptorPool = CreateDescriptorPool ( mVulkanRenderer.GetDevice(), descriptor_pool_sizes );
        }

        std::vector<VkWriteDescriptorSet> write_descriptor_sets{};
        write_descriptor_sets.reserve ( sampler_binding_count + ( ( mMaterial->GetUniformBuffer().size() ? 1 : 0 ) ) );

        // Must outlive the single vkUpdateDescriptorSets call at the end of this
        // constructor: the VkWriteDescriptorSet below stores a pointer to this
        // struct, so a block-scoped local would dangle once the uniform-buffer
        // block closes. The sampler block's stack reuse then zeroes its range,
        // tripping VUID-VkDescriptorBufferInfo-range-00341 on strict drivers
        // (undefined behavior that happened to survive on the Windows build).
        VkDescriptorBufferInfo descriptor_buffer_info{};

        if ( mMaterial->GetUniformBuffer().size() > 0 )
        {
            VkDescriptorSetLayoutBinding descriptor_set_layout_binding
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr
            };
            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = 1,
                .pBindings = &descriptor_set_layout_binding
            };

            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
            descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
            descriptor_set_allocate_info.descriptorSetCount = 1;
            descriptor_set_allocate_info.pSetLayouts = &mVulkanRenderer.GetDescriptorSetLayout ( descriptor_set_layout_create_info );
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
            // Populate the function-scope struct (declared above) so the pointer
            // stored in pBufferInfo stays valid until vkUpdateDescriptorSets.
            descriptor_buffer_info = VkDescriptorBufferInfo
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

        {
            std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings{};
            descriptor_set_layout_bindings.reserve ( sampler_binding_count );
            for ( uint32_t i = 0; i < sampler_binding_count; ++i )
            {
                descriptor_set_layout_bindings.push_back (
                    VkDescriptorSetLayoutBinding
                {
                    .binding = i,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                } );
            }
            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = static_cast<uint32_t> ( descriptor_set_layout_bindings.size() ),
                .pBindings = descriptor_set_layout_bindings.data()
            };

            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
            descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
            descriptor_set_allocate_info.descriptorSetCount = 1;
            descriptor_set_allocate_info.pSetLayouts = &mVulkanRenderer.GetDescriptorSetLayout ( descriptor_set_layout_create_info );
            if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, &mSamplerDescriptorSet ) )
            {
                std::ostringstream stream;
                stream << "Allocate Descriptor Set failed: ( " << GetVulkanResultString ( result ) << " )";
                std::cout << LogLevel::Error << stream.str() << std::endl;
                throw std::runtime_error ( stream.str().c_str() );
            }

            for ( uint32_t i = 0; i < sampler_binding_count; ++i )
            {
                // Bind canonical slot i to the material's matching sampler (by
                // name crc), or the slot's fallback texture when the material
                // does not declare it.
                const char* slot_name = kMaterialSamplerSlots[i].name;
                const uint32_t slot_crc = crc32i ( slot_name, std::strlen ( slot_name ) );
                const VkDescriptorImageInfo* source_image_info = mVulkanRenderer.GetMaterialSamplerFallbackDescriptorImageInfo ( i );
                const Texture* bindless_texture = nullptr;
                Material::SamplerState bindless_state{};
                for ( const auto& sampler : mMaterial->GetSamplers() )
                {
                    if ( std::get<0> ( sampler ) == slot_crc )
                    {
                        bindless_texture = std::get<1> ( sampler ).Get<Texture>();
                        bindless_state = std::get<2> ( sampler );
                        source_image_info = mVulkanRenderer.GetTextureDescriptorImageInfo ( *bindless_texture );
                        break;
                    }
                }
                if ( bindless_texture == nullptr )
                {
                    ResourceId fallback{ "Texture"_crc32, kMaterialSamplerSlots[i].fallback_path };
                    bindless_texture = fallback.Get<Texture>();
                    bindless_state = {};
                }
                mDescriptorImageInfos[i] = *source_image_info;
                mDescriptorImageInfos[i].sampler = mSamplers[i];
                mBindlessSamplerSlots[i] = mVulkanRenderer.AcquireBindlessSamplerSlot ( *bindless_texture, bindless_state );
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
                write_descriptor_set.pImageInfo = &mDescriptorImageInfos[i];
                write_descriptor_set.pTexelBufferView = nullptr;
            }
        }
        if ( !write_descriptor_sets.empty() )
        {
            vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
        }

        // Assemble this material's bindless record: resolve each canonical
        // sampler slot to a global texture-array slot (falling back exactly like
        // the sampler set above) and copy the factor block, then register it in
        // the renderer's global material storage buffer. The record's factor
        // region shares the std140 Material block layout, so the material's
        // uniform buffer copies in verbatim. (Additive: the descriptor sets
        // above still drive rendering until the shaders switch to bindless.)
        GpuMaterial record{};
        for ( uint32_t i = 0; i < sampler_binding_count; ++i )
        {
            const uint32_t bindless_slot = mBindlessSamplerSlots[i];
            record.texture_refs[i][0] = bindless_slot;
            record.texture_refs[i][1] = 0;
        }
        if ( !mMaterial->GetUniformBuffer().empty() )
        {
            // The factor block begins right after the texture references; copy
            // through a byte pointer (GpuMaterial holds Vector4 members that are
            // not trivially copyable, so a typed memcpy target is rejected).
            const size_t factor_offset = sizeof ( record.texture_refs );
            const size_t factor_bytes = sizeof ( GpuMaterial ) - factor_offset;
            std::memcpy ( reinterpret_cast<uint8_t*> ( &record ) + factor_offset,
                          mMaterial->GetUniformBuffer().data(),
                          std::min ( mMaterial->GetUniformBuffer().size(), factor_bytes ) );
        }
        mBindlessMaterialIndex = mVulkanRenderer.RegisterBindlessMaterial ( record );
    }

    void VulkanMaterial::Finalize ()
    {
        if ( mMaterial == nullptr )
        {
            return;
        }
        // Release the global material-buffer slot, then the descriptor pool.
        mVulkanRenderer.UnregisterBindlessMaterial ( mBindlessMaterialIndex );
        for ( size_t slot = 0; slot < mBindlessSamplerSlots.size(); ++slot )
        {
            const uint32_t slot_crc = crc32i ( kMaterialSamplerSlots[slot].name, std::strlen ( kMaterialSamplerSlots[slot].name ) );
            const Texture* texture = nullptr;
            Material::SamplerState state{};
            if ( mMaterial != nullptr )
            {
                for ( const auto& sampler : mMaterial->GetSamplers() )
                {
                    if ( std::get<0> ( sampler ) == slot_crc )
                    {
                        texture = std::get<1> ( sampler ).Get<Texture>();
                        state = std::get<2> ( sampler );
                        break;
                    }
                }
            }
            if ( texture == nullptr )
            {
                ResourceId fallback{ "Texture"_crc32, kMaterialSamplerSlots[slot].fallback_path };
                texture = fallback.Get<Texture>();
            }
            mVulkanRenderer.ReleaseBindlessSamplerSlot ( *texture, state );
        }
        // Finalize Descriptor Pool
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer.GetDevice(), mVkDescriptorPool, nullptr );
        }
        mUniformBuffer.Finalize();
        for ( VkSampler& sampler : mSamplers )
        {
            if ( sampler != VK_NULL_HANDLE )
            {
                vkDestroySampler ( mVulkanRenderer.GetDevice(), sampler, nullptr );
                sampler = VK_NULL_HANDLE;
            }
        }
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
        mSamplers = aVulkanMaterial.mSamplers;
        mDescriptorImageInfos = aVulkanMaterial.mDescriptorImageInfos;
        mBindlessSamplerSlots = aVulkanMaterial.mBindlessSamplerSlots;
        std::swap ( mVkDescriptorPool, aVulkanMaterial.mVkDescriptorPool );
        std::swap ( mUniformDescriptorSet, aVulkanMaterial.mUniformDescriptorSet );
        std::swap ( mSamplerDescriptorSet, aVulkanMaterial.mSamplerDescriptorSet );
        std::swap ( mBindlessMaterialIndex, aVulkanMaterial.mBindlessMaterialIndex );
        aVulkanMaterial.mSamplers.fill ( VK_NULL_HANDLE );
        aVulkanMaterial.mBindlessSamplerSlots.fill ( UINT32_MAX );
        aVulkanMaterial.mMaterial = nullptr;
    }

    void VulkanMaterial::Bind ( VkCommandBuffer aVkCommandBuffer, const VulkanPipeline& aPipeline  ) const
    {
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
        // Push the material storage buffer's device address so the fragment
        // shader reaches the records as a buffer_reference (BDA). The address is
        // constant across the frame; the per-instance material index is delivered
        // separately through the InstanceMaterials buffer. Inert for pipelines
        // without the material-buffer push constant.
        if ( const VkPushConstantRange& material_buffer = aPipeline.GetPushConstantMaterialBuffer(); material_buffer.size != 0 )
        {
            VkDeviceAddress material_buffer_address = mVulkanRenderer.GetMaterialStorageBufferDeviceAddress();
            vkCmdPushConstants ( aVkCommandBuffer, aPipeline.GetPipelineLayout(),
                                 material_buffer.stageFlags, material_buffer.offset, material_buffer.size,
                                 &material_buffer_address );
        }
    }

    uint32_t VulkanMaterial::GetBindlessMaterialIndex() const
    {
        return mBindlessMaterialIndex;
    }
}
