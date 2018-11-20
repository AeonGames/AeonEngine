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
#include <ostream>
#include <regex>
#include <array>
#include <utility>
#include <cassert>
#include "aeongames/ProtoBufClasses.h"
#include "aeongames/ProtoBufUtils.h"
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
    VulkanMaterial::VulkanMaterial ( const VulkanRenderer&  aVulkanRenderer, uint32_t aPath ) :
        mVulkanRenderer { aVulkanRenderer },
        mUniformBuffer { mVulkanRenderer }
    {
        if ( aPath )
        {
            Load ( aPath );
        }
    }

    VulkanMaterial::~VulkanMaterial()
    {
        Unload();
    }

    VulkanMaterial::VulkanMaterial ( const VulkanMaterial& aMaterial ) :
        mVulkanRenderer{aMaterial.mVulkanRenderer},
        mUniformBuffer{aMaterial.mUniformBuffer},
        mVariables{aMaterial.mVariables}
    {
        InitializeDescriptorSetLayout();
        InitializeDescriptorPool();
        InitializeDescriptorSet();
    }

    VulkanMaterial& VulkanMaterial::operator= ( const VulkanMaterial& aMaterial )
    {
        if ( &mVulkanRenderer != &aMaterial.mVulkanRenderer )
        {
            throw std::runtime_error ( "Assigning materials from different renderer instances." );
        }
        mVariables = aMaterial.mVariables;
        mUniformBuffer = aMaterial.mUniformBuffer;
        InitializeDescriptorSetLayout();
        InitializeDescriptorPool();
        InitializeDescriptorSet();
        return *this;
    }

    std::unique_ptr<Material> VulkanMaterial::Clone() const
    {
        return std::make_unique<VulkanMaterial> ( *this );
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
        mVariables.reserve ( aMaterialBuffer.property().size() );
        for ( auto& i : aMaterialBuffer.property() )
        {
            switch ( i.value_case() )
            {
            case PropertyBuffer::ValueCase::kScalarFloat:
                size += ( size % sizeof ( float ) ) ? sizeof ( float ) - ( size % sizeof ( float ) ) : 0; // Align to float
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( float );
                break;
            case PropertyBuffer::ValueCase::kScalarUint:
                size += ( size % sizeof ( uint32_t ) ) ? sizeof ( uint32_t ) - ( size % sizeof ( uint32_t ) ) : 0; // Align to uint
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( uint32_t );
                break;
            case PropertyBuffer::ValueCase::kScalarInt:
                size += ( size % sizeof ( int32_t ) ) ? sizeof ( int32_t ) - ( size % sizeof ( int32_t ) ) : 0; // Align to int
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( int32_t );
                break;
            case PropertyBuffer::ValueCase::kVector2:
                size += ( size % ( sizeof ( float ) * 2 ) ) ? ( sizeof ( float ) * 2 ) - ( size % ( sizeof ( float ) * 2 ) ) : 0; // Align to 2 floats
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( float ) * 2;
                break;
            case PropertyBuffer::ValueCase::kVector3:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( float ) * 3;
                break;
            case PropertyBuffer::ValueCase::kVector4:
                size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // Align to 4 floats
                mVariables.emplace_back ( i.name(), i.value_case(), size );
                size += sizeof ( float ) * 4;
                break;
            default:
                break;
            }
        }

        size += ( size % ( sizeof ( float ) * 4 ) ) ? ( sizeof ( float ) * 4 ) - ( size % ( sizeof ( float ) * 4 ) ) : 0; // align the final value to 4 floats
        if ( size )
        {
            mUniformBuffer.Initialize (
                static_cast<VkDeviceSize> ( size ),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

            auto* pointer = reinterpret_cast<uint8_t*> ( mUniformBuffer.Map ( 0, size ) );
            for ( auto& i : mVariables )
            {
                auto j = std::find_if ( aMaterialBuffer.property().begin(), aMaterialBuffer.property().end(),
                                        [&i] ( const PropertyBuffer & property )
                {
                    return property.name() == i.GetName();
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
        mSamplers.reserve ( aMaterialBuffer.sampler().size() );
        for ( auto& i : aMaterialBuffer.sampler() )
        {
            std::get<1> ( mSamplers.emplace_back ( i.name(), ResourceId{"Image"_crc32, GetReferenceBufferId ( i.image() ) } ) ).Store();
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
        return ResourceId{"Image"_crc32, 0};
    }

    const std::vector<VkDescriptorSetLayout>& VulkanMaterial::GetDescriptorSetLayouts() const
    {
        return mVkDescriptorSetLayouts;
    }

    const std::vector<VkDescriptorSet>& VulkanMaterial::GetDescriptorSets() const
    {
        return mVkUniformDescriptorSets;
    }

    void VulkanMaterial::InitializeDescriptorSetLayout()
    {
        if ( !mUniformBuffer.GetSize() && !mSamplers.size() )
        {
            // We don' need any layouts.
            return;
        }

        size_t layout_index = 0;
        mVkDescriptorSetLayouts.resize ( ( mUniformBuffer.GetSize() ? 1 : 0 ) + ( mSamplers.size() ? 1 : 0 ), VK_NULL_HANDLE );

        if ( mUniformBuffer.GetSize() )
        {
            VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
            descriptor_set_layout_binding.binding = 0;
            descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            /* We will bind just 1 UBO, descriptor count is the number of array elements, and we just use a single struct. */
            descriptor_set_layout_binding.descriptorCount = 1;
            descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_ALL;
            descriptor_set_layout_binding.pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
            descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_set_layout_create_info.pNext = nullptr;
            descriptor_set_layout_create_info.flags = 0;
            descriptor_set_layout_create_info.bindingCount = 1;
            descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;
            if ( VkResult result = vkCreateDescriptorSetLayout ( mVulkanRenderer.GetDevice(), &descriptor_set_layout_create_info, nullptr, &mVkDescriptorSetLayouts[layout_index++] ) )
            {
                std::ostringstream stream;
                stream << "DescriptorSet Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }

        if ( mSamplers.size() )
        {
            std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
            descriptor_set_layout_bindings.resize ( mSamplers.size() );
            for ( uint32_t i = 0; i < mSamplers.size(); ++i )
            {
                descriptor_set_layout_bindings[i].binding = i;
                descriptor_set_layout_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                // Descriptor Count is the count of elements in an array.
                descriptor_set_layout_bindings[i].descriptorCount = 1;
                descriptor_set_layout_bindings[i].stageFlags = VK_SHADER_STAGE_ALL;
                descriptor_set_layout_bindings[i].pImmutableSamplers = nullptr;
            }

            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
            descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_set_layout_create_info.pNext = nullptr;
            descriptor_set_layout_create_info.flags = 0;
            descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t> ( descriptor_set_layout_bindings.size() );
            descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings.data();
            if ( VkResult result = vkCreateDescriptorSetLayout ( mVulkanRenderer.GetDevice(), &descriptor_set_layout_create_info, nullptr, &mVkDescriptorSetLayouts[layout_index++] ) )
            {
                std::ostringstream stream;
                stream << "DescriptorSet Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
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

    void VulkanMaterial::InitializeDescriptorSet()
    {
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = mVkDescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = static_cast<uint32_t> ( mVkDescriptorSetLayouts.size() );
        descriptor_set_allocate_info.pSetLayouts = mVkDescriptorSetLayouts.data();
        mVkUniformDescriptorSets.resize ( mVkDescriptorSetLayouts.size(), VK_NULL_HANDLE );
        if ( VkResult result = vkAllocateDescriptorSets ( mVulkanRenderer.GetDevice(), &descriptor_set_allocate_info, mVkUniformDescriptorSets.data() ) )
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
        write_descriptor_sets.reserve ( mUniformBuffer.GetSize() ? 1 : 0 + mSamplers.size() );

        size_t descriptor_set_index = 0;

        if ( mUniformBuffer.GetSize() )
        {
            write_descriptor_sets.emplace_back();
            auto& write_descriptor_set = write_descriptor_sets.back();
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.pNext = nullptr;
            write_descriptor_set.dstSet = mVkUniformDescriptorSets[descriptor_set_index++];
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
            write_descriptor_set.dstSet = mVkUniformDescriptorSets[descriptor_set_index];
            write_descriptor_set.dstBinding = i;
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptor_set.descriptorCount = 1;
            write_descriptor_set.pBufferInfo = nullptr;
            write_descriptor_set.pImageInfo = &reinterpret_cast<const VulkanImage*> ( std::get<1> ( mSamplers[i] ).Get<Image>() )->GetDescriptorImageInfo();
        }
        descriptor_set_index += 1;
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

    void VulkanMaterial::FinalizeDescriptorSetLayout()
    {
        for ( auto& i : mVkDescriptorSetLayouts )
        {
            vkDestroyDescriptorSetLayout ( mVulkanRenderer.GetDevice(), i, nullptr );
            i = VK_NULL_HANDLE;
        }
        mVkDescriptorSetLayouts.clear();
    }

    void VulkanMaterial::FinalizeDescriptorSet()
    {
        if ( mVkUniformDescriptorSets.size() )
        {
            vkFreeDescriptorSets ( mVulkanRenderer.GetDevice(),
                                   mVkDescriptorPool,
                                   static_cast<uint32_t> ( mVkUniformDescriptorSets.size() ),
                                   mVkUniformDescriptorSets.data() );
            mVkUniformDescriptorSets.clear();
        }
    }

    const std::vector<std::tuple<std::string, ResourceId>>& VulkanMaterial::GetSamplers() const
    {
        return mSamplers;
    }
}
