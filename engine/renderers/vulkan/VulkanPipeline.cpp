/*
Copyright (C) 2017-2018 Rodrigo Jose Hernandez Cordoba

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
#include <cassert>
#include <utility>
#include <vector>
#include <sstream>
#include <fstream>
#include <vulkan/vulkan.h>
#include "aeongames/ResourceCache.h"
#include "aeongames/Pipeline.h"
#include "aeongames/Material.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanMaterial.h"
#include "VulkanUtilities.h"
#include "VulkanBuffer.h"
#include "SPIR-V/CompilerLinker.h"

namespace AeonGames
{
    VulkanPipeline::VulkanPipeline ( const Pipeline& aPipeline, const std::shared_ptr<const VulkanRenderer>& aVulkanRenderer ) :
        mPipeline ( aPipeline  ),
        mVulkanRenderer ( aVulkanRenderer )
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

    VulkanPipeline::~VulkanPipeline()
    {
        Finalize();
    }

    void VulkanPipeline::Use ( const VulkanMaterial& aMaterial ) const
    {
#if 0
        std::array<VkDescriptorSet, 1> descriptor_sets { { mVkDescriptorSet }};
        vkCmdBindPipeline ( mVulkanRenderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mVkPipeline );
#if 0
        vkCmdPushConstants ( mVulkanRenderer->GetCommandBuffer(),
                             mVkPipelineLayout,
                             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                             0, sizeof ( VulkanRenderer::Transforms ), &mVulkanRenderer->GetTransforms() );
#endif
#if 0
        /** @todo vkCmdUpdateBuffer is not supposed to be called inside a render pass... but it works here.*/
        vkCmdUpdateBuffer ( mVulkanRenderer->GetCommandBuffer(), mVkPropertiesPropertyBuffer, 0,
                            material->GetPropertyData().size(), material->GetPropertyData().data() );
#endif
        /** @todo Add a Stack buffer for skeletons and properly set the offset. */
        uint32_t offset = 0;
        vkCmdBindDescriptorSets ( mVulkanRenderer->GetCommandBuffer(),
                                  VK_PIPELINE_BIND_POINT_GRAPHICS, mVkPipelineLayout, 0, ( descriptor_sets[1] == VK_NULL_HANDLE ) ? 1 : 2, descriptor_sets.data(), 1, &offset );
#endif
    }

#if 0
    void VulkanPipeline::InitializeSkeletonUniform()
    {
        if ( mPipeline.GetAttributes() & ( Pipeline::VertexWeightIndicesBit | Pipeline::VertexWeightsBit ) )
        {
            mSkeletonBuffer.Initialize ( 256 * 16 * sizeof ( float ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
            auto* joint_array = static_cast<float*> ( mSkeletonBuffer.Map ( 0, VK_WHOLE_SIZE ) );
            const float identity[16] =
            {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            };
            for ( size_t i = 0; i < 256; ++i )
            {
                memcpy ( ( joint_array + ( i * 16 ) ), identity, sizeof ( float ) * 16 );
            }
            mSkeletonBuffer.Unmap();
        }
    }

    void VulkanPipeline::FinalizeSkeletonUniform()
    {
    }
#endif
#if 0
    void VulkanPipeline::InitializeDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
        descriptor_set_layout_bindings.reserve ( 2 + mPipeline.GetDefaultMaterial().GetSamplerCount() );
        uint32_t binding = 0;
        // Properties binding
        if ( mPropertiesBuffer.GetSize() )
        {
            descriptor_set_layout_bindings.emplace_back();
            descriptor_set_layout_bindings.back().binding = binding++;
            descriptor_set_layout_bindings.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            /* We will bind just 1 UBO, descriptor count is the number of array elements, and we just use a single struct. */
            descriptor_set_layout_bindings.back().descriptorCount = 1;
            descriptor_set_layout_bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
            descriptor_set_layout_bindings.back().pImmutableSamplers = nullptr;
        }
        // Sampler bindings
        for ( auto& i : mPipeline.GetDefaultMaterial().GetProperties() )
        {
            if ( i.GetType() == Material::PropertyType::SAMPLER_2D )
            {
                descriptor_set_layout_bindings.emplace_back();
                auto& descriptor_set_layout_binding = descriptor_set_layout_bindings.back();
                descriptor_set_layout_binding.binding = binding++;
                // Descriptor Count is the count of elements in an array.
                descriptor_set_layout_binding.descriptorCount = 1;
                descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_set_layout_binding.pImmutableSamplers = nullptr;
                descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            }
        }
        // Skeleton binding
        if ( mSkeletonBuffer.GetSize() )
        {
            descriptor_set_layout_bindings.emplace_back();
            descriptor_set_layout_bindings.back().binding = binding++;
            descriptor_set_layout_bindings.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            descriptor_set_layout_bindings.back().descriptorCount = 1;
            descriptor_set_layout_bindings.back().stageFlags = VK_SHADER_STAGE_ALL;
            descriptor_set_layout_bindings.back().pImmutableSamplers = nullptr;
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

    void VulkanPipeline::FinalizeDescriptorSetLayout()
    {
        if ( mVkPropertiesDescriptorSetLayout != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorSetLayout ( mVulkanRenderer->GetDevice(), mVkPropertiesDescriptorSetLayout, nullptr );
            mVkPropertiesDescriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    void VulkanPipeline::InitializeDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 3> descriptor_pool_sizes{};
        descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_pool_sizes[0].descriptorCount = ( mPipeline.GetDefaultMaterial().GetPropertyBlock().size() == 0 ) ? 1 : 2;
        descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptor_pool_sizes[1].descriptorCount = 1;
        descriptor_pool_sizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_pool_sizes[2].descriptorCount = static_cast<uint32_t> ( mPipeline.GetDefaultMaterial().GetSamplerCount() );

        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.pNext = nullptr;
        descriptor_pool_create_info.flags = 0;
        descriptor_pool_create_info.maxSets = 1;
        descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t> ( descriptor_pool_sizes.size() ) - ( descriptor_pool_sizes[2].descriptorCount ) ? 0 : 1;
        descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
        if ( VkResult result = vkCreateDescriptorPool ( mVulkanRenderer->GetDevice(), &descriptor_pool_create_info, nullptr, &mVkDescriptorPool ) )
        {
            std::ostringstream stream;
            stream << "vkCreateDescriptorPool failed. error code: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanPipeline::FinalizeDescriptorPool()
    {
        if ( mVkDescriptorPool != VK_NULL_HANDLE )
        {
            vkDestroyDescriptorPool ( mVulkanRenderer->GetDevice(), mVkDescriptorPool, nullptr );
            mVkDescriptorPool = VK_NULL_HANDLE;
        }
    }

    void VulkanPipeline::InitializeDescriptorSet()
    {
        std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts{ { mVkPropertiesDescriptorSetLayout } };
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
        // We'll be moving this somewhere else.
        std::array<VkDescriptorBufferInfo, 3> descriptor_buffer_infos =
        {
            {
                VkDescriptorBufferInfo{mMatrices.GetBuffer(), 0, sizeof ( float ) * 32},
                VkDescriptorBufferInfo{mProperties.GetBuffer(), 0, mPipeline.GetDefaultMaterial().GetPropertyBlock().size() },
                VkDescriptorBufferInfo{mSkeleton.GetBuffer(),   0, 256 * 16 * sizeof ( float ) }
            }
        };

        std::vector<VkWriteDescriptorSet> write_descriptor_sets{};
        write_descriptor_sets.reserve ( 3 + mPipeline.GetDefaultMaterial().GetSamplerCount() );
        // Matrices
        write_descriptor_sets.emplace_back();
        write_descriptor_sets.back().sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_sets.back().pNext = nullptr;
        write_descriptor_sets.back().dstSet = mVkDescriptorSet;
        write_descriptor_sets.back().dstBinding = 0;
        write_descriptor_sets.back().dstArrayElement = 0;
        write_descriptor_sets.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_sets.back().descriptorCount = 1;
        write_descriptor_sets.back().pBufferInfo = &descriptor_buffer_infos[0];
        write_descriptor_sets.back().pImageInfo = nullptr;
        write_descriptor_sets.back().pTexelBufferView = nullptr;
        if ( mPipeline.GetDefaultMaterial().GetPropertyBlock().size() )
        {
            write_descriptor_sets.emplace_back();
            write_descriptor_sets.back().sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_sets.back().pNext = nullptr;
            write_descriptor_sets.back().dstSet = mVkDescriptorSet;
            write_descriptor_sets.back().dstBinding = 1;
            write_descriptor_sets.back().dstArrayElement = 0;
            write_descriptor_sets.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptor_sets.back().descriptorCount = 1;
            write_descriptor_sets.back().pBufferInfo = &descriptor_buffer_infos[1];
            write_descriptor_sets.back().pImageInfo = nullptr;
            write_descriptor_sets.back().pTexelBufferView = nullptr;
        }
        if ( mSkeleton.GetSize() )
        {
            write_descriptor_sets.emplace_back();
            write_descriptor_sets.back().sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_sets.back().pNext = nullptr;
            write_descriptor_sets.back().dstSet = mVkDescriptorSet;
            write_descriptor_sets.back().dstBinding = 2;
            write_descriptor_sets.back().dstArrayElement = 0;
            write_descriptor_sets.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            write_descriptor_sets.back().descriptorCount = 1;
            write_descriptor_sets.back().pBufferInfo = &descriptor_buffer_infos[2];
            write_descriptor_sets.back().pImageInfo = nullptr;
            write_descriptor_sets.back().pTexelBufferView = nullptr;
        }
        for ( uint32_t i = 0; i < mPipeline.GetDefaultMaterial().GetSamplerCount(); ++i )
        {
            write_descriptor_sets.emplace_back();
            auto& write_descriptor_set = write_descriptor_sets.back();
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.pNext = nullptr;
            write_descriptor_set.dstSet = mVkDescriptorSet;
            write_descriptor_set.dstBinding = i; // this will probably break because of how the shader code is build.
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptor_set.descriptorCount = 1;
            assert ( 0 && "TODO: Set pImageInfo" );
            write_descriptor_set.pImageInfo = nullptr;//&mTextures[i]->GetDescriptorImageInfo();
        }
        vkUpdateDescriptorSets ( mVulkanRenderer->GetDevice(), static_cast<uint32_t> ( write_descriptor_sets.size() ), write_descriptor_sets.data(), 0, nullptr );
    }

    void VulkanPipeline::FinalizeDescriptorSet()
    {
    }
#endif

    void VulkanPipeline::Initialize()
    {
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        CompilerLinker compiler_linker;
        compiler_linker.AddShaderSource ( EShLanguage::EShLangVertex, mPipeline.GetVertexShaderSource().c_str() );
        compiler_linker.AddShaderSource ( EShLanguage::EShLangFragment, mPipeline.GetFragmentShaderSource().c_str() );
        if ( CompilerLinker::FailCode result = compiler_linker.CompileAndLink() )
        {
            std::ostringstream stream;
            stream << mPipeline.GetVertexShaderSource() << std::endl;
            stream << mPipeline.GetFragmentShaderSource() << std::endl;
            stream << ( ( result == CompilerLinker::EFailCompile ) ? "Compilation" : "Linking" ) <<
                   " Error:" << std::endl << compiler_linker.GetLog();
            throw std::runtime_error ( stream.str().c_str() );
        }
        {
            VkShaderModuleCreateInfo shader_module_create_info{};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = compiler_linker.GetSpirV ( EShLanguage::EShLangVertex ).size() * sizeof ( uint32_t );
            shader_module_create_info.pCode = compiler_linker.GetSpirV ( EShLanguage::EShLangVertex ).data();

            if ( VkResult result = vkCreateShaderModule ( mVulkanRenderer->GetDevice(), &shader_module_create_info, nullptr, &mVkShaderModules[ffs ( VK_SHADER_STAGE_VERTEX_BIT )] ) )
            {
                std::ostringstream stream;
                stream << "Shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        {
            VkShaderModuleCreateInfo shader_module_create_info{};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = compiler_linker.GetSpirV ( EShLanguage::EShLangFragment ).size() * sizeof ( uint32_t );
            shader_module_create_info.pCode = compiler_linker.GetSpirV ( EShLanguage::EShLangFragment ).data();
            if ( VkResult result = vkCreateShaderModule ( mVulkanRenderer->GetDevice(), &shader_module_create_info, nullptr, &mVkShaderModules[ffs ( VK_SHADER_STAGE_FRAGMENT_BIT )] ) )
            {
                std::ostringstream stream;
                stream << "Shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }

        std::array<VkPipelineShaderStageCreateInfo, 2> pipeline_shader_stage_create_infos{ {} };
        pipeline_shader_stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_shader_stage_create_infos[0].pNext = nullptr;
        pipeline_shader_stage_create_infos[0].flags = 0;
        pipeline_shader_stage_create_infos[0].module = mVkShaderModules[ffs ( VK_SHADER_STAGE_VERTEX_BIT )];
        pipeline_shader_stage_create_infos[0].pName = "main";
        pipeline_shader_stage_create_infos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stage_create_infos[0].pSpecializationInfo = nullptr;

        pipeline_shader_stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_shader_stage_create_infos[1].pNext = nullptr;
        pipeline_shader_stage_create_infos[1].flags = 0;
        pipeline_shader_stage_create_infos[1].module = mVkShaderModules[ffs ( VK_SHADER_STAGE_FRAGMENT_BIT )];
        pipeline_shader_stage_create_infos[1].pName = "main";
        pipeline_shader_stage_create_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_shader_stage_create_infos[1].pSpecializationInfo = nullptr;

        std::array<VkVertexInputBindingDescription, 1> vertex_input_binding_descriptions { {} };
        vertex_input_binding_descriptions[0].binding = 0;
        vertex_input_binding_descriptions[0].stride = sizeof ( Vertex );
        vertex_input_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        uint32_t attributes = mPipeline.GetAttributes();
        std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions ( popcount ( attributes ) );
        for ( auto& i : vertex_input_attribute_descriptions )
        {
            uint32_t attribute_bit = ( 1 << ffs ( attributes ) );
            i.location = mPipeline.GetLocation ( static_cast<Pipeline::AttributeBits> ( attribute_bit ) );
            i.binding = 0;
            i.format = GetVulkanFormat ( mPipeline.GetFormat ( static_cast<Pipeline::AttributeBits> ( attribute_bit ) ) );
            i.offset = mPipeline.GetOffset ( static_cast<Pipeline::AttributeBits> ( attribute_bit ) );
            attributes ^= attribute_bit;
        }

        VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info {};
        pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipeline_vertex_input_state_create_info.pNext = nullptr;
        pipeline_vertex_input_state_create_info.flags = 0;
        pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t> ( vertex_input_binding_descriptions.size() );
        pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = vertex_input_binding_descriptions.data();
        pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t> ( vertex_input_attribute_descriptions.size() );
        pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();

        VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
        pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipeline_input_assembly_state_create_info.pNext = nullptr;
        pipeline_input_assembly_state_create_info.flags = 0;

        switch ( mPipeline.GetTopology() )
        {
        case Pipeline::Topology::POINT_LIST:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case Pipeline::Topology::LINE_STRIP:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case Pipeline::Topology::LINE_LIST:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case Pipeline::Topology::TRIANGLE_STRIP:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case Pipeline::Topology::TRIANGLE_FAN:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
            break;
        case Pipeline::Topology::TRIANGLE_LIST:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case Pipeline::Topology::LINE_LIST_WITH_ADJACENCY:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
            break;
        case Pipeline::Topology::LINE_STRIP_WITH_ADJACENCY:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
            break;
        case Pipeline::Topology::TRIANGLE_LIST_WITH_ADJACENCY:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
            break;
        case Pipeline::Topology::TRIANGLE_STRIP_WITH_ADJACENCY:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
            break;
        case Pipeline::Topology::PATCH_LIST:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
        default:
            pipeline_input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        }

        pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info {};
        pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipeline_viewport_state_create_info.pNext = nullptr;
        pipeline_viewport_state_create_info.flags = 0;
        pipeline_viewport_state_create_info.viewportCount = 1;
        pipeline_viewport_state_create_info.pViewports = nullptr;
        pipeline_viewport_state_create_info.scissorCount = 1;
        pipeline_viewport_state_create_info.pScissors = nullptr;

        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{};
        pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipeline_rasterization_state_create_info.pNext = nullptr;
        pipeline_rasterization_state_create_info.flags = 0;
        pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pipeline_rasterization_state_create_info.depthBiasEnable = VK_TRUE;
        pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
        pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
        pipeline_rasterization_state_create_info.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{};
        pipeline_multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipeline_multisample_state_create_info.pNext = nullptr;
        pipeline_multisample_state_create_info.flags = 0;
        pipeline_multisample_state_create_info.sampleShadingEnable = VK_TRUE;
        pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipeline_multisample_state_create_info.minSampleShading = 0.0f;
        pipeline_multisample_state_create_info.pSampleMask = nullptr;
        pipeline_multisample_state_create_info.alphaToCoverageEnable = VK_TRUE;
        pipeline_multisample_state_create_info.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{};
        pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipeline_depth_stencil_state_create_info.pNext = nullptr;
        pipeline_depth_stencil_state_create_info.flags = 0;
        pipeline_depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
        pipeline_depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
        pipeline_depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
        pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
        pipeline_depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
        pipeline_depth_stencil_state_create_info.front = {};
        pipeline_depth_stencil_state_create_info.back = {};
        pipeline_depth_stencil_state_create_info.minDepthBounds = 0.0f;
        pipeline_depth_stencil_state_create_info.maxDepthBounds = 1.0f;

        std::array<VkPipelineColorBlendAttachmentState, 1> pipeline_color_blend_attachment_states{ {} };
        pipeline_color_blend_attachment_states[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pipeline_color_blend_attachment_states[0].blendEnable = VK_TRUE;
        pipeline_color_blend_attachment_states[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipeline_color_blend_attachment_states[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipeline_color_blend_attachment_states[0].colorBlendOp = VK_BLEND_OP_ADD;
        pipeline_color_blend_attachment_states[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pipeline_color_blend_attachment_states[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipeline_color_blend_attachment_states[0].alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{};
        pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipeline_color_blend_state_create_info.pNext = nullptr;
        pipeline_color_blend_state_create_info.flags = 0;
        pipeline_color_blend_state_create_info.logicOpEnable = VK_FALSE;
        pipeline_color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
        pipeline_color_blend_state_create_info.attachmentCount = static_cast<uint32_t> ( pipeline_color_blend_attachment_states.size() );
        pipeline_color_blend_state_create_info.pAttachments = pipeline_color_blend_attachment_states.data();
        memset ( pipeline_color_blend_state_create_info.blendConstants, 0, sizeof ( VkPipelineColorBlendStateCreateInfo::blendConstants ) );

        std::array<VkDynamicState, 2> dynamic_states
        {
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            }
        };
        VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
        pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipeline_dynamic_state_create_info.pNext = nullptr;
        pipeline_dynamic_state_create_info.flags = 0;
        pipeline_dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t> ( dynamic_states.size() );
        pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states.data();
#if 0
        std::array<VkPushConstantRange, 1> push_constant_ranges {};
        push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; ///@todo determine ALL stage flags based on usage.
        push_constant_ranges[0].offset = 0;
        /** @todo We'll have to reduce our matrices to just two here,
            the validation layers don't like size to be more than VkPhysicalLimits::maxPushConstantsSize
            which at maximum must be 128 bytes to be safe. */
        push_constant_ranges[0].size = sizeof ( float ) * 16; // the push constant will contain just the Model Matrix
#endif
        /** @todo Gather Layouts from Renderer and Material*/
        std::array<VkDescriptorSetLayout, 1> descriptor_set_layouts { { /*mVkPropertiesDescriptorSetLayout*/ } };
        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pNext = nullptr;
        pipeline_layout_create_info.setLayoutCount = descriptor_set_layouts.size();
        pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
#if 0
        pipeline_layout_create_info.pushConstantRangeCount = static_cast<uint32_t> ( push_constant_ranges.size() );
        pipeline_layout_create_info.pPushConstantRanges = push_constant_ranges.data();
#endif
        if ( VkResult result = vkCreatePipelineLayout ( mVulkanRenderer->GetDevice(), &pipeline_layout_create_info, nullptr, &mVkPipelineLayout ) )
        {
            std::ostringstream stream;
            stream << "Pipeline Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info {};
        graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphics_pipeline_create_info.pNext = nullptr;
        graphics_pipeline_create_info.flags = 0;
        graphics_pipeline_create_info.stageCount = static_cast<uint32_t> ( pipeline_shader_stage_create_infos.size() );
        graphics_pipeline_create_info.pStages = pipeline_shader_stage_create_infos.data();
        graphics_pipeline_create_info.pVertexInputState = &pipeline_vertex_input_state_create_info;
        graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
        graphics_pipeline_create_info.pTessellationState = nullptr;
        graphics_pipeline_create_info.pViewportState = &pipeline_viewport_state_create_info;
        graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
        graphics_pipeline_create_info.pMultisampleState = &pipeline_multisample_state_create_info;
        graphics_pipeline_create_info.pDepthStencilState = &pipeline_depth_stencil_state_create_info;
        graphics_pipeline_create_info.pColorBlendState = &pipeline_color_blend_state_create_info;
        graphics_pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
        graphics_pipeline_create_info.layout = mVkPipelineLayout;
        graphics_pipeline_create_info.renderPass = mVulkanRenderer->GetRenderPass();
        graphics_pipeline_create_info.subpass = 0;
        graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        graphics_pipeline_create_info.basePipelineIndex = 0;
        if ( VkResult result = vkCreateGraphicsPipelines ( mVulkanRenderer->GetDevice(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &mVkPipeline ) )
        {
            std::ostringstream stream;
            stream << "Pipeline creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }
    }

    void VulkanPipeline::Finalize()
    {
        if ( mVkPipeline != VK_NULL_HANDLE )
        {
            vkDestroyPipeline ( mVulkanRenderer->GetDevice(), mVkPipeline, nullptr );
            mVkPipeline = VK_NULL_HANDLE;
        }
        if ( mVkPipelineLayout != VK_NULL_HANDLE )
        {
            vkDestroyPipelineLayout ( mVulkanRenderer->GetDevice(), mVkPipelineLayout, nullptr );
            mVkPipelineLayout = VK_NULL_HANDLE;
        }
        for ( auto& i : mVkShaderModules )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyShaderModule ( mVulkanRenderer->GetDevice(), i, nullptr );
                i = VK_NULL_HANDLE;
            }
        }
    }
}
