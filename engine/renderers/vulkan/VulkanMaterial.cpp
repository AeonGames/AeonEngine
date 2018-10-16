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
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/Material.h"
#include "aeongames/Vector2.h"
#include "aeongames/Vector3.h"
#include "aeongames/Vector4.h"
#include "VulkanMaterial.h"
#include "VulkanImage.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"

namespace AeonGames
{
    VulkanMaterial::VulkanMaterial ( const VulkanRenderer&  aVulkanRenderer ) :
        mVulkanRenderer { aVulkanRenderer },
        mUniformBuffer { mVulkanRenderer }
    {
    }

    VulkanMaterial::~VulkanMaterial()
    {
        Unload();
    }

    void VulkanMaterial::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }

    void VulkanMaterial::Load ( const uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            Unload();
            throw;
        }
    }

    void VulkanMaterial::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static MaterialBuffer material_buffer;
        LoadProtoBufObject ( material_buffer, aBuffer, aBufferSize, "AEONMTL" );
        Load ( material_buffer );
        material_buffer.Clear();
    }

    void VulkanMaterial::Load ( const MaterialBuffer& aMaterialBuffer )
    {
        size_t size = 0;
        size_t offset = 0;
        mVariables.reserve ( aMaterialBuffer.property().size() );
        for ( auto& i : aMaterialBuffer.property() )
        {
            switch ( i.value_case() )
            {
            case PropertyBuffer::ValueCase::kScalarFloat:
                offset += ( offset % sizeof ( float ) ) ? sizeof ( float ) - ( offset % sizeof ( float ) ) : 0;
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                size += sizeof ( float );
                break;
            case PropertyBuffer::ValueCase::kScalarUint:
                offset += ( offset % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( offset % sizeof ( uint32_t ) ) : 0;
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                size += sizeof ( uint32_t );
                break;
            case PropertyBuffer::ValueCase::kScalarInt:
                offset += ( offset % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( offset % sizeof ( int32_t ) ) : 0;
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to uint
                size += sizeof ( int32_t );
                break;
            case PropertyBuffer::ValueCase::kVector2:
                offset += ( offset % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( offset % ( sizeof ( float ) * 2 ) ) : 0;
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                size += sizeof ( float ) * 2;
                break;
            case PropertyBuffer::ValueCase::kVector3:
                offset += ( offset % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( offset % ( sizeof ( float ) * 4 ) ) : 0;
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 3;
                break;
            case PropertyBuffer::ValueCase::kVector4:
                offset += ( offset % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( offset % ( sizeof ( float ) * 4 ) ) : 0;
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                size += sizeof ( float ) * 4;
                break;
            default:
                break;
            }
            mVariables.emplace_back ( i.uniform_name(), i.value_case(), offset );
        }

        size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // align the final value to 4 floats
        if ( size )
        {
            mUniformBuffer.Initialize (
                static_cast<VkDeviceSize> ( size ),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

            uint8_t* pointer = reinterpret_cast<uint8_t*> ( mUniformBuffer.Map ( 0, size ) );
            for ( auto& i : mVariables )
            {
                auto j = std::find_if ( aMaterialBuffer.property().begin(), aMaterialBuffer.property().end(),
                                        [&i] ( const PropertyBuffer & property )
                {
                    return property.uniform_name() == i.GetName();
                } );
                if ( j != aMaterialBuffer.property().end() )
                {
                    switch ( i.GetType() )
                    {
                    case PropertyBuffer::ValueCase::kScalarFloat:
                        ( *reinterpret_cast<float*> ( pointer + i.GetOffset() ) ) = j->scalar_float();
                        break;
                    case PropertyBuffer::ValueCase::kScalarUint:
                        ( *reinterpret_cast<uint32_t*> ( pointer + i.GetOffset() ) ) = j->scalar_uint();
                        break;
                    case PropertyBuffer::ValueCase::kScalarInt:
                        ( *reinterpret_cast<int32_t*> ( pointer + i.GetOffset() ) ) = j->scalar_int();
                        break;
                    case PropertyBuffer::ValueCase::kVector2:
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [0] = j->vector2().x();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [1] = j->vector2().y();
                        break;
                    case PropertyBuffer::ValueCase::kVector3:
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [0] = j->vector3().x();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [1] = j->vector3().y();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [2] = j->vector3().z();
                        break;
                    case PropertyBuffer::ValueCase::kVector4:
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [0] = j->vector4().x();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [1] = j->vector4().y();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [2] = j->vector4().z();
                        reinterpret_cast<float*> ( pointer + i.GetOffset() ) [3] = j->vector4().w();
                        break;
                    default:
                        break;
                    }
                }
            }
            mUniformBuffer.Unmap();
        }
        InitializeDescriptorSetLayout();
        InitializeDescriptorPool();
        InitializeDescriptorSet();
    }

    void VulkanMaterial::Unload()
    {
        FinalizeDescriptorSet();
        FinalizeDescriptorPool();
        FinalizeDescriptorSetLayout();
        mUniformBuffer.Finalize();
    }

    void VulkanMaterial::SetUint ( const std::string& aName, uint32_t aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kScalarUint ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( uint32_t ), &aValue );
        }
    }

    void VulkanMaterial::SetSint ( const std::string& aName, int32_t aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kScalarInt ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( int32_t ), &aValue );
        }
    }

    void VulkanMaterial::SetFloat ( const std::string& aName, float aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kScalarFloat ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ), &aValue );
        }
    }

    void VulkanMaterial::SetFloatVec2 ( const std::string& aName, const Vector2& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kVector2 ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 2, aValue.GetVector() );
        }
    }

    void VulkanMaterial::SetFloatVec3 ( const std::string& aName, const Vector3& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kVector3 ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 3, aValue.GetVector3() );
        }
    }

    void VulkanMaterial::SetFloatVec4 ( const std::string& aName, const Vector4& aValue )
    {
        auto i = std::find_if ( mVariables.begin(), mVariables.end(),
                                [&aName] ( const UniformVariable & variable )
        {
            return variable.GetName() == aName;
        } );
        if ( ( i != mVariables.end() ) && ( i->GetType() == PropertyBuffer::ValueCase::kVector4 ) )
        {
            mUniformBuffer.WriteMemory ( i->GetOffset(), sizeof ( float ) * 4, aValue.GetVector4() );
        }
    }

    void VulkanMaterial::SetSampler ( const std::string& aName, const std::string& aValue )
    {
        ///@todo Reimplement samplers
    }
    uint32_t VulkanMaterial::GetUint ( const std::string& aName )
    {
        return 0;
    }
    int32_t VulkanMaterial::GetSint ( const std::string& aName )
    {
        return 0;
    }
    float VulkanMaterial::GetFloat ( const std::string& aName )
    {
        return 0.0f;
    }
    Vector2 VulkanMaterial::GetFloatVec2 ( const std::string& aName )
    {
        return Vector2{};
    }
    Vector3 VulkanMaterial::GetFloatVec3 ( const std::string& aName )
    {
        return Vector3{};
    }
    Vector4 VulkanMaterial::GetFloatVec4 ( const std::string& aName )
    {
        return Vector4{};
    }
    std::string VulkanMaterial::GetSampler ( const std::string& aName )
    {
        return std::string{};
    }

#if 0
    void VulkanMaterial::Update ( const uint8_t* aValue, size_t aOffset, size_t aSize )
    {
        mUniformBuffer.WriteMemory ( aOffset, ( aSize ) ? aSize : GetPropertyBlock().size() - aOffset, aValue );
    }
#endif

    const VkDescriptorSetLayout& VulkanMaterial::GetPropertiesDescriptorSetLayout() const
    {
        return mVkPropertiesDescriptorSetLayout;
    }

    const VkDescriptorSet VulkanMaterial::GetPropertiesDescriptorSet() const
    {
        return mVkPropertiesDescriptorSet;
    }

    void VulkanMaterial::InitializeDescriptorSetLayout()
    {
        if ( !mUniformBuffer.GetSize() )
        {
            // We don' need a layout.
            return;
        }
        std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
        descriptor_set_layout_bindings.reserve ( 1 /*+ GetSamplerCount()*/ );
        uint32_t binding = 0;

        if ( mUniformBuffer.GetSize() )
        {
            descriptor_set_layout_bindings.emplace_back();
            descriptor_set_layout_bindings.back().binding = binding++;
            descriptor_set_layout_bindings.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            /* We will bind just 1 UBO, descriptor count is the number of array elements, and we just use a single struct. */
            descriptor_set_layout_bindings.back().descriptorCount = 1;
            descriptor_set_layout_bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
            descriptor_set_layout_bindings.back().pImmutableSamplers = nullptr;
        }
#if 0
        for ( auto& i : GetProperties() )
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
#endif
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
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
        if ( mUniformBuffer.GetSize() )
        {
            descriptor_pool_sizes.emplace_back();
            descriptor_pool_sizes.back().type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_pool_sizes.back().descriptorCount = 1;
        }
#if 0
        if ( GetSamplerCount() )
        {
            descriptor_pool_sizes.emplace_back();
            descriptor_pool_sizes.back().type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_pool_sizes.back().descriptorCount = static_cast<uint32_t> ( GetSamplerCount() );
        }
#endif
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
                VkDescriptorBufferInfo{mUniformBuffer.GetBuffer(), 0, mUniformBuffer.GetSize() }
            }
        };

        std::vector<VkWriteDescriptorSet> write_descriptor_sets{};
        write_descriptor_sets.reserve ( 1 /*+ GetSamplerCount()*/ );

        if ( mUniformBuffer.GetSize() )
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

#if 0
        uint32_t index {0};
        for ( auto& i : GetProperties() )
        {
            if ( i.GetType() == Material::PropertyType::SAMPLER_2D )
            {
                write_descriptor_sets.emplace_back();
                auto& write_descriptor_set = write_descriptor_sets.back();
                write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor_set.pNext = nullptr;
                write_descriptor_set.dstSet = mVkPropertiesDescriptorSet;
                ///@todo find a better way to calculate sampler binding, perhaps move samplers to their own descriptor set.
                write_descriptor_set.dstBinding = static_cast<uint32_t> ( ( write_descriptor_sets.size() - 1 ) + index++ );
                write_descriptor_set.dstArrayElement = 0;
                write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write_descriptor_set.descriptorCount = 1;
                write_descriptor_set.pBufferInfo = nullptr;
                write_descriptor_set.pImageInfo = &reinterpret_cast<const VulkanImage*> ( i.GetImage() )->GetDescriptorImageInfo();
            }
        }
#endif
        vkUpdateDescriptorSets ( mVulkanRenderer.GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
    }

    void VulkanMaterial::FinalizeDescriptorSet()
    {
    }
}
