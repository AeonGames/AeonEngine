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
#include <cassert>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/CRC.hpp"
#include <vulkan/vulkan.h>
#include "VulkanPipeline.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"
#include "SPIR-V/CompilerLinker.h"
#include <spirv_reflect.h>

namespace AeonGames
{
    static const std::unordered_map<SpvReflectFormat, VkFormat> SpvReflectToVulkanFormat
    {
        { SPV_REFLECT_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED },
        { SPV_REFLECT_FORMAT_R16_UINT, VK_FORMAT_R16_UINT },
        { SPV_REFLECT_FORMAT_R16_SINT, VK_FORMAT_R16_SINT },
        { SPV_REFLECT_FORMAT_R16_SFLOAT, VK_FORMAT_R16_SFLOAT },
        { SPV_REFLECT_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_UINT },
        { SPV_REFLECT_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SINT },
        { SPV_REFLECT_FORMAT_R16G16_SFLOAT, VK_FORMAT_R16G16_SFLOAT },
        { SPV_REFLECT_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16_UINT },
        { SPV_REFLECT_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16_SINT },
        { SPV_REFLECT_FORMAT_R16G16B16_SFLOAT, VK_FORMAT_R16G16B16_SFLOAT },
        { SPV_REFLECT_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_UINT },
        { SPV_REFLECT_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT },
        { SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT },
        { SPV_REFLECT_FORMAT_R32_UINT, VK_FORMAT_R32_UINT },
        { SPV_REFLECT_FORMAT_R32_SINT, VK_FORMAT_R32_SINT },
        { SPV_REFLECT_FORMAT_R32_SFLOAT, VK_FORMAT_R32_SFLOAT },
        { SPV_REFLECT_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_UINT },
        { SPV_REFLECT_FORMAT_R32G32_SINT, VK_FORMAT_R32G32_SINT },
        { SPV_REFLECT_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32_SFLOAT },
        { SPV_REFLECT_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_UINT },
        { SPV_REFLECT_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32_SINT },
        { SPV_REFLECT_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT },
        { SPV_REFLECT_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_UINT },
        { SPV_REFLECT_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SINT },
        { SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT },
        { SPV_REFLECT_FORMAT_R64_UINT, VK_FORMAT_R64_UINT },
        { SPV_REFLECT_FORMAT_R64_SINT, VK_FORMAT_R64_SINT },
        { SPV_REFLECT_FORMAT_R64_SFLOAT, VK_FORMAT_R64_SFLOAT },
        { SPV_REFLECT_FORMAT_R64G64_UINT, VK_FORMAT_R64G64_UINT },
        { SPV_REFLECT_FORMAT_R64G64_SINT, VK_FORMAT_R64G64_SINT },
        { SPV_REFLECT_FORMAT_R64G64_SFLOAT, VK_FORMAT_R64G64_SFLOAT },
        { SPV_REFLECT_FORMAT_R64G64B64_UINT, VK_FORMAT_R64G64B64_UINT },
        { SPV_REFLECT_FORMAT_R64G64B64_SINT, VK_FORMAT_R64G64B64_SINT },
        { SPV_REFLECT_FORMAT_R64G64B64_SFLOAT, VK_FORMAT_R64G64B64_SFLOAT },
        { SPV_REFLECT_FORMAT_R64G64B64A64_UINT, VK_FORMAT_R64G64B64A64_UINT },
        { SPV_REFLECT_FORMAT_R64G64B64A64_SINT, VK_FORMAT_R64G64B64A64_SINT },
        { SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT, VK_FORMAT_R64G64B64A64_SFLOAT }
    };

    static const std::unordered_map<VkFormat, uint32_t> VkFormatToVulkanSize
    {
        { VK_FORMAT_UNDEFINED, 0 },
        { VK_FORMAT_R8G8B8A8_UINT, static_cast<uint32_t> ( sizeof ( uint8_t ) * 4 ) },
        { VK_FORMAT_R8G8B8A8_UNORM, static_cast<uint32_t> ( sizeof ( uint8_t ) * 4 ) },
        { VK_FORMAT_R16_UINT, static_cast<uint32_t> ( sizeof ( uint16_t ) ) },
        { VK_FORMAT_R16_SINT, static_cast<uint32_t> ( sizeof ( int16_t ) ) },
        { VK_FORMAT_R16_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) ) },
        { VK_FORMAT_R16G16_UINT, static_cast<uint32_t> ( sizeof ( uint16_t ) * 2 ) },
        { VK_FORMAT_R16G16_SINT, static_cast<uint32_t> ( sizeof ( int16_t ) * 2 ) },
        { VK_FORMAT_R16G16_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 2 ) },
        { VK_FORMAT_R16G16B16_UINT, static_cast<uint32_t> ( sizeof ( uint16_t ) * 3 ) },
        { VK_FORMAT_R16G16B16_SINT, static_cast<uint32_t> ( sizeof ( int16_t ) * 3 ) },
        { VK_FORMAT_R16G16B16_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 3 ) },
        { VK_FORMAT_R16G16B16A16_UINT, static_cast<uint32_t> ( sizeof ( uint16_t ) * 4 ) },
        { VK_FORMAT_R16G16B16A16_SINT, static_cast<uint32_t> ( sizeof ( int16_t ) * 4 ) },
        { VK_FORMAT_R16G16B16A16_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 4 ) },
        { VK_FORMAT_R32_UINT, static_cast<uint32_t> ( sizeof ( uint32_t ) ) },
        { VK_FORMAT_R32_SINT, static_cast<uint32_t> ( sizeof ( int32_t ) ) },
        { VK_FORMAT_R32_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) ) },
        { VK_FORMAT_R32G32_UINT, static_cast<uint32_t> ( sizeof ( uint32_t ) * 2 ) },
        { VK_FORMAT_R32G32_SINT, static_cast<uint32_t> ( sizeof ( int32_t ) * 2 ) },
        { VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 2 ) },
        { VK_FORMAT_R32G32B32_UINT, static_cast<uint32_t> ( sizeof ( uint32_t ) * 3 ) },
        { VK_FORMAT_R32G32B32_SINT, static_cast<uint32_t> ( sizeof ( int32_t ) * 3 ) },
        { VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 3 ) },
        { VK_FORMAT_R32G32B32A32_UINT, static_cast<uint32_t> ( sizeof ( uint32_t ) * 4 ) },
        { VK_FORMAT_R32G32B32A32_SINT, static_cast<uint32_t> ( sizeof ( int32_t ) * 4 ) },
        { VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 4 ) },
        { VK_FORMAT_R64_UINT, static_cast<uint32_t> ( sizeof ( uint64_t ) ) },
        { VK_FORMAT_R64_SINT, static_cast<uint32_t> ( sizeof ( int64_t ) ) },
        { VK_FORMAT_R64_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) ) },
        { VK_FORMAT_R64G64_UINT, static_cast<uint32_t> ( sizeof ( uint64_t ) * 2 ) },
        { VK_FORMAT_R64G64_SINT, static_cast<uint32_t> ( sizeof ( int64_t ) * 2 ) },
        { VK_FORMAT_R64G64_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 2 ) },
        { VK_FORMAT_R64G64B64_UINT, static_cast<uint32_t> ( sizeof ( uint64_t ) * 3 ) },
        { VK_FORMAT_R64G64B64_SINT, static_cast<uint32_t> ( sizeof ( int64_t ) * 3 ) },
        { VK_FORMAT_R64G64B64_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 3 ) },
        { VK_FORMAT_R64G64B64A64_UINT, static_cast<uint32_t> ( sizeof ( uint64_t ) * 4 ) },
        { VK_FORMAT_R64G64B64A64_SINT, static_cast<uint32_t> ( sizeof ( int64_t ) * 4 ) },
        { VK_FORMAT_R64G64B64A64_SFLOAT, static_cast<uint32_t> ( sizeof ( float ) * 4 ) }
    };

    static const std::unordered_map<SpvReflectDescriptorType, VkDescriptorType> SpvReflectToVulkanDescriptorType
    {
        { SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER },
        { SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
        { SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
        { SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE },
        { SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER },
        { SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER },
        { SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
        { SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
        { SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC },
        { SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC },
        { SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT },
        { SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR }
    };

    static const std::unordered_map<uint32_t, VkPrimitiveTopology> TopologyClassToVulkanTopology
    {
        { Pipeline::TOPOLOGY_CLASS_TRIANGLE, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
        { Pipeline::TOPOLOGY_CLASS_LINE, VK_PRIMITIVE_TOPOLOGY_LINE_LIST },
        { Pipeline::TOPOLOGY_CLASS_POINT, VK_PRIMITIVE_TOPOLOGY_POINT_LIST },
        { Pipeline::TOPOLOGY_CLASS_PATCH, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST }
    };

    bool operator!= ( const VkDescriptorSetLayoutBinding& a, const SpvReflectDescriptorBinding& b )
    {
        return ! ( a.binding == b.binding &&
                   a.descriptorType  == SpvReflectToVulkanDescriptorType.at ( b.descriptor_type ) &&
                   a.descriptorCount == b.count &&
                   a.pImmutableSamplers == nullptr );
    }

    VulkanPipeline::VulkanPipeline ( VulkanPipeline&& aVulkanPipeline ) :
        mVulkanRenderer{aVulkanPipeline.mVulkanRenderer}
    {
        std::swap ( mPipeline, aVulkanPipeline.mPipeline );
        std::swap ( mVkPipelineLayout, aVulkanPipeline.mVkPipelineLayout );
        std::swap ( mVkPipeline, aVulkanPipeline.mVkPipeline );
        std::swap ( mVertexStride, aVulkanPipeline.mVertexStride );
        std::swap ( mPushConstantModelMatrix, aVulkanPipeline.mPushConstantModelMatrix );
        mVertexAttributes.swap ( aVulkanPipeline.mVertexAttributes );
        mDescriptorSets.swap ( aVulkanPipeline.mDescriptorSets );
    }

    std::array<EShLanguage, ShaderType::COUNT> ShaderTypeToEShLanguage
    {
        EShLanguage::EShLangVertex,
        EShLanguage::EShLangFragment,
        EShLanguage::EShLangCompute,
        EShLanguage::EShLangTessControl,
        EShLanguage::EShLangTessEvaluation,
        EShLanguage::EShLangGeometry
    };

    std::array<VkShaderStageFlagBits, ShaderType::COUNT> ShaderTypeToShaderStageFlagBit
    {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_SHADER_STAGE_COMPUTE_BIT,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT
    };

    VulkanPipeline::VulkanPipeline ( const VulkanRenderer&  aVulkanRenderer, const Pipeline& aPipeline ) :
        mVulkanRenderer { aVulkanRenderer }, mPipeline{&aPipeline}
    {
        std::array < VkShaderModule, ShaderType::COUNT > shader_modules{ { VK_NULL_HANDLE } };
        CompilerLinker compiler_linker{static_cast<CompilerLinker::TOptions> (
                                           CompilerLinker::TOptions::EOptionSpv |
                                           CompilerLinker::TOptions::EOptionVulkanRules |
                                           CompilerLinker::TOptions::EOptionLinkProgram
                                           //| CompilerLinker::TOptions::EOptionDumpReflection
                                       ) };

        std::array<std::string_view, ShaderType::COUNT> shader_codes =
        {
            aPipeline.GetShaderCode ( VERT ),
            aPipeline.GetShaderCode ( FRAG ),
            aPipeline.GetShaderCode ( COMP ),
            aPipeline.GetShaderCode ( TESC ),
            aPipeline.GetShaderCode ( TESE ),
            aPipeline.GetShaderCode ( GEOM )
        };

        for ( uint32_t i = 0; i < ShaderType::COUNT; ++i )
        {
            if ( shader_codes.at ( i ).empty() )
            {
                continue;
            }
            compiler_linker.AddShaderSource ( ShaderTypeToEShLanguage.at ( i ), shader_codes.at ( i ).data() );
        }

        if ( CompilerLinker::FailCode result = compiler_linker.CompileAndLink() )
        {
            std::ostringstream stream;
            stream << ( ( result == CompilerLinker::EFailCompile ) ? "Compilation" : "Linking" ) <<
                                                                     " Error:" << std::endl << compiler_linker.GetLog();
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        uint32_t pipeline_shader_stage_create_info_count{0};
        std::array<VkPipelineShaderStageCreateInfo, ShaderType::COUNT> pipeline_shader_stage_create_infos{ {} };
        for ( uint32_t i = 0; i < ShaderType::COUNT; ++i )
        {
            const std::vector<uint32_t>& spirv{compiler_linker.GetSpirV ( ShaderTypeToEShLanguage.at ( i ) ) };
            if ( spirv.size() == 0 )
            {
                continue;
            }
            VkShaderModuleCreateInfo shader_module_create_info{};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = spirv.size() * sizeof ( uint32_t );
            shader_module_create_info.pCode = spirv.data();

            if ( VkResult result = vkCreateShaderModule ( mVulkanRenderer.GetDevice(), &shader_module_create_info, nullptr, &shader_modules[ i ] ) )
            {
                std::ostringstream stream;
                stream << "Shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
                std::cout << LogLevel::Error << stream.str();
                throw std::runtime_error ( stream.str().c_str() );
            }

            //--------Reflection----------//
            SpvReflectShaderModule module {};
            SpvReflectResult result = spvReflectCreateShaderModule (
                                          shader_module_create_info.codeSize,
                                          shader_module_create_info.pCode,
                                          &module );
            if ( result != SPV_REFLECT_RESULT_SUCCESS )
            {
                std::ostringstream stream;
                stream << "SPIR-V Reflect module creation failed: ( " << static_cast<int> ( result ) << " )";
                std::cout << LogLevel::Error << stream.str();
                throw std::runtime_error ( stream.str().c_str() );
            }

            if ( i == VERT )
            {
                // Only vertex shader needs attribute reflection
                ReflectAttributes ( module );
            }
            ReflectDescriptorSets ( module, static_cast<ShaderType> ( i ) );
            ReflectPushConstants ( module, static_cast<ShaderType> ( i ) );
            spvReflectDestroyShaderModule ( &module );
            //--------Reflection-END----------//
            //----------------Shader Stages------------------//
            pipeline_shader_stage_create_infos[pipeline_shader_stage_create_info_count].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            pipeline_shader_stage_create_infos[pipeline_shader_stage_create_info_count].pNext = nullptr;
            pipeline_shader_stage_create_infos[pipeline_shader_stage_create_info_count].flags = 0;
            pipeline_shader_stage_create_infos[pipeline_shader_stage_create_info_count].module = shader_modules[i];
            pipeline_shader_stage_create_infos[pipeline_shader_stage_create_info_count].pName = "main";
            pipeline_shader_stage_create_infos[pipeline_shader_stage_create_info_count].stage = ShaderTypeToShaderStageFlagBit.at ( static_cast<ShaderType> ( i ) );
            pipeline_shader_stage_create_infos[pipeline_shader_stage_create_info_count++].pSpecializationInfo = nullptr;
        }

        //----------------Vertex Input------------------//
        std::array<VkVertexInputBindingDescription, 1> vertex_input_binding_descriptions { {} };
        vertex_input_binding_descriptions[0].binding = 0;
        vertex_input_binding_descriptions[0].stride = mVertexStride;
        vertex_input_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info {};
        pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipeline_vertex_input_state_create_info.pNext = nullptr;
        pipeline_vertex_input_state_create_info.flags = 0;
        pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t> ( vertex_input_binding_descriptions.size() );
        pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = vertex_input_binding_descriptions.data();
        pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t> ( mVertexAttributes.size() );
        pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = mVertexAttributes.data();

        //----------------Input Assembly------------------//
        VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
        pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipeline_input_assembly_state_create_info.pNext = nullptr;
        pipeline_input_assembly_state_create_info.flags = 0;
        pipeline_input_assembly_state_create_info.topology = TopologyClassToVulkanTopology.at ( mPipeline->GetTopologyClass() );
#ifdef VK_USE_PLATFORM_METAL_EXT
        pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_TRUE;
#else
        pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;
#endif
        //----------------Viewport State------------------//
        VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info {};
        pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipeline_viewport_state_create_info.pNext = nullptr;
        pipeline_viewport_state_create_info.flags = 0;
        pipeline_viewport_state_create_info.viewportCount = 1;
        pipeline_viewport_state_create_info.pViewports = nullptr;
        pipeline_viewport_state_create_info.scissorCount = 1;
        pipeline_viewport_state_create_info.pScissors = nullptr;

        //----------------Rasterization State------------------//
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

        //----------------Multisample State------------------//
        VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{};
        pipeline_multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipeline_multisample_state_create_info.pNext = nullptr;
        pipeline_multisample_state_create_info.flags = 0;
        pipeline_multisample_state_create_info.sampleShadingEnable = VK_FALSE;
        pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipeline_multisample_state_create_info.minSampleShading = 0.0f;
        pipeline_multisample_state_create_info.pSampleMask = nullptr;
        pipeline_multisample_state_create_info.alphaToCoverageEnable = VK_TRUE;
        pipeline_multisample_state_create_info.alphaToOneEnable = VK_FALSE;

        //----------------Depth Stencil State------------------//
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

        //----------------Color Blend State------------------//
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

        //----------------Dynamic State------------------//
        std::array<VkDynamicState, 3> dynamic_states
        {
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
                VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
            }
        };
        VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
        pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipeline_dynamic_state_create_info.pNext = nullptr;
        pipeline_dynamic_state_create_info.flags = 0;
        pipeline_dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t> ( dynamic_states.size() );
        pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states.data();

        std::array<VkDescriptorSetLayout, 8> descriptor_set_layouts{};
        if ( mDescriptorSets.size() > descriptor_set_layouts.size() )
        {
            std::cout << LogLevel::Error << "More descriptor sets than available slots" << std::endl;
            throw std::runtime_error ( "More descriptor sets than available slots" );
        }
        for ( const auto& i : mDescriptorSets )
        {
            if ( i.set >= mDescriptorSets.size() )
            {
                std::cout << LogLevel::Error << "Set index out of bounds" << std::endl;
                throw std::runtime_error ( "Set index out of bounds" );
            }
            descriptor_set_layouts[i.set] = mVulkanRenderer.GetDescriptorSetLayout ( i.descriptor_set_layout_create_info );
        }

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pNext = nullptr;
        pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t> ( mDescriptorSets.size() );
        pipeline_layout_create_info.pSetLayouts = mDescriptorSets.size() ? descriptor_set_layouts.data() : nullptr;
        pipeline_layout_create_info.pushConstantRangeCount = mPushConstantModelMatrix.offset == 0 && mPushConstantModelMatrix.size == 0 ? 0 : 1;
        pipeline_layout_create_info.pPushConstantRanges = pipeline_layout_create_info.pushConstantRangeCount ? &mPushConstantModelMatrix : nullptr;
        if ( VkResult result = vkCreatePipelineLayout ( mVulkanRenderer.GetDevice(), &pipeline_layout_create_info, nullptr, &mVkPipelineLayout ) )
        {
            std::ostringstream stream;
            stream << "Pipeline Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }

        //----------------Shader Stages------------------//
        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info {};
        graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphics_pipeline_create_info.pNext = nullptr;
        graphics_pipeline_create_info.flags = 0;
        graphics_pipeline_create_info.stageCount = pipeline_shader_stage_create_info_count;
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
        graphics_pipeline_create_info.renderPass = mVulkanRenderer.GetRenderPass();
        graphics_pipeline_create_info.subpass = 0;
        graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        graphics_pipeline_create_info.basePipelineIndex = 0;

        if ( VkResult result = vkCreateGraphicsPipelines ( mVulkanRenderer.GetDevice(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &mVkPipeline ) )
        {
            std::ostringstream stream;
            stream << "Pipeline creation failed: ( " << GetVulkanResultString ( result ) << " )";
            std::cout << LogLevel::Error << stream.str() << std::endl;
            throw std::runtime_error ( stream.str().c_str() );
        }
        for ( auto& i : shader_modules )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyShaderModule ( mVulkanRenderer.GetDevice(), i, nullptr );
                i = VK_NULL_HANDLE;
            }
        }
    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline ( mVulkanRenderer.GetDevice(), mVkPipeline, nullptr );
        vkDestroyPipelineLayout ( mVulkanRenderer.GetDevice(), mVkPipelineLayout, nullptr );
    }

    const VkPipelineLayout VulkanPipeline::GetPipelineLayout() const
    {
        return mVkPipelineLayout;
    }

    const VkPipeline VulkanPipeline::GetVkPipeline() const
    {
        return mVkPipeline;
    }

    const Pipeline* VulkanPipeline::GetPipeline() const
    {
        return mPipeline;
    }

    const VkDescriptorSetLayout VulkanPipeline::GetDescriptorSetLayout ( uint32_t name ) const
    {
        auto it = std::lower_bound ( mDescriptorSets.begin(), mDescriptorSets.end(), name,
                                     [] ( const VulkanDescriptorSetInfo & a, const uint32_t b )
        {
            return a.hash < b;
        } );
        if ( it != mDescriptorSets.end() && it->hash == name )
        {
            return mVulkanRenderer.GetDescriptorSetLayout ( it->descriptor_set_layout_create_info );
        }
        return VK_NULL_HANDLE;
    }

    void VulkanPipeline::ReflectAttributes ( SpvReflectShaderModule& module )
    {
        uint32_t var_count{0};
        SpvReflectResult result{spvReflectEnumerateInputVariables ( &module, &var_count, nullptr ) };
        if ( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            std::ostringstream stream;
            stream << "SPIR-V Reflect input variable enumeration failed: ( " << static_cast<int> ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }
        std::vector<SpvReflectInterfaceVariable*> input_vars ( var_count );
        result = spvReflectEnumerateInputVariables ( &module, &var_count, input_vars.data() );
        if ( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            std::ostringstream stream;
            stream << "SPIR-V Reflect input variable enumeration failed: ( " << static_cast<int> ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }
        mVertexAttributes.reserve ( mVertexAttributes.capacity() + input_vars.size() );
        for ( const auto& i : input_vars )
        {
            if ( i->built_in < 0 )
            {
                const uint32_t name_crc{crc32i ( i->name, strlen ( i->name ) ) };
                // Override format for weight attributes - they use 8-bit data even though shader declares 32-bit
                VkFormat format;
                if ( name_crc == Mesh::WEIGHT_INDEX )
                {
                    format = VK_FORMAT_R8G8B8A8_UINT; // uint8_t indices
                }
                else if ( name_crc == Mesh::WEIGHT_VALUE )
                {
                    format = VK_FORMAT_R8G8B8A8_UNORM; // normalized uint8_t weights (0-255 -> 0.0-1.0)
                }
                else
                {
                    format = SpvReflectToVulkanFormat.at ( i->format );
                }

                auto it = std::lower_bound ( mVertexAttributes.begin(), mVertexAttributes.end(), i->location,
                                             [] ( const VkVertexInputAttributeDescription & a, const uint32_t b )
                {
                    return a.location < b;
                } );
                mVertexAttributes.insert ( it,
                {
                    i->location,
                    0,
                    format,
                    0
                } );
            }
        }
        for ( auto& attr : mVertexAttributes )
        {
            // Calculate offsets and stride
            attr.offset = mVertexStride;
            mVertexStride += VkFormatToVulkanSize.at ( attr.format );
        }
    }

    void VulkanPipeline::ReflectDescriptorSets ( SpvReflectShaderModule& aModule, ShaderType aType )
    {
        // Get descriptor set count
        uint32_t descriptor_set_count = 0;
        SpvReflectResult result = spvReflectEnumerateDescriptorSets ( &aModule, &descriptor_set_count, nullptr );
        if ( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            std::ostringstream stream;
            stream << "SPIR-V Reflect descriptor set enumeration failed: ( " << static_cast<int> ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        if ( descriptor_set_count > 0 )
        {
            std::vector<SpvReflectDescriptorSet*> descriptor_sets ( descriptor_set_count );
            result = spvReflectEnumerateDescriptorSets ( &aModule, &descriptor_set_count, descriptor_sets.data() );
            if ( result != SPV_REFLECT_RESULT_SUCCESS )
            {
                std::ostringstream stream;
                stream << "SPIR-V Reflect descriptor set enumeration failed: ( " << static_cast<int> ( result ) << " )";
                std::cout << LogLevel::Error << stream.str();
                throw std::runtime_error ( stream.str().c_str() );
            }
            for ( const auto& descriptor_set : descriptor_sets )
            {

                const char* type_name{ ( descriptor_set->binding_count == 1 ) && descriptor_set->bindings[0]->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ?
                                       descriptor_set->bindings[0]->type_description->type_name : "Samplers"};
                uint32_t hash = crc32i ( type_name,
                                         strlen ( type_name ) );

                auto it = std::lower_bound ( mDescriptorSets.begin(), mDescriptorSets.end(), hash,
                                             [] ( const VulkanDescriptorSetInfo & a, const uint32_t b )
                {
                    return a.hash < b;
                } );

                ///@Kwizatz Add more checks to avoid binding conflicts across different shaders
                if ( it != mDescriptorSets.end() && it->hash == hash && it->set == descriptor_set->set )
                {
                    for ( auto& binding : it->descriptor_set_layout_bindings )
                    {
                        binding.stageFlags |= ShaderTypeToShaderStageFlagBit.at ( aType );
                    }
                    continue;
                }

                it = mDescriptorSets.insert ( it,
                                              VulkanDescriptorSetInfo
                {
                    hash,
                    descriptor_set->set,
                    VkDescriptorSetLayoutCreateInfo
                    {
                        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = 0
                    },
                    {}
                } );
                it->descriptor_set_layout_bindings.reserve ( descriptor_set->binding_count );
                std::cout << LogLevel::Debug << "Descriptor set " << descriptor_set->set << std::endl;

                for ( uint32_t i = 0; i < descriptor_set->binding_count; ++i )
                {
                    const SpvReflectDescriptorBinding& descriptor_set_binding = *descriptor_set->bindings[i];
                    auto layout_binding = std::lower_bound ( it->descriptor_set_layout_bindings.begin(), it->descriptor_set_layout_bindings.end(), descriptor_set_binding.binding,
                                          [] ( const VkDescriptorSetLayoutBinding & a, const uint32_t b )
                    {
                        return a.binding < b;
                    } );

                    if ( layout_binding == it->descriptor_set_layout_bindings.end() || layout_binding->binding != descriptor_set_binding.binding )
                    {
                        layout_binding = it->descriptor_set_layout_bindings.insert ( layout_binding, VkDescriptorSetLayoutBinding{} );
                        layout_binding->binding = descriptor_set_binding.binding;
                        layout_binding->descriptorType = ( hash == Mesh::BindingLocations::SKELETON ) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : SpvReflectToVulkanDescriptorType.at ( descriptor_set_binding.descriptor_type );
                        layout_binding->descriptorCount = descriptor_set_binding.count;
                        layout_binding->stageFlags = ShaderTypeToShaderStageFlagBit.at ( aType );
                        layout_binding->pImmutableSamplers = nullptr;
                        std::cout << LogLevel::Debug << "Set " << descriptor_set_binding.set << " New Binding " << descriptor_set_binding.binding  << " Type Name " << type_name << " Shader Type " << ShaderTypeToString.at ( aType ) << std::endl;
                    }
                }
                it->descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t> ( it->descriptor_set_layout_bindings.size() );
                it->descriptor_set_layout_create_info.pBindings = it->descriptor_set_layout_bindings.data();
            }
        }
    }

    uint32_t VulkanPipeline::GetDescriptorSetIndex ( uint32_t hash ) const
    {
        auto it = std::lower_bound ( mDescriptorSets.begin(), mDescriptorSets.end(), hash,
                                     [] ( const VulkanDescriptorSetInfo & a, const uint32_t b )
        {
            return a.hash < b;
        } );
        if ( it != mDescriptorSets.end() && it->hash == hash )
        {
            return it->set;
        }
        return std::numeric_limits<uint32_t>::max();
    }

    const VkPushConstantRange& VulkanPipeline::GetPushConstantModelMatrix() const
    {
        return mPushConstantModelMatrix;
    }

    void VulkanPipeline::ReflectPushConstants ( SpvReflectShaderModule& aModule, ShaderType aType )
    {
        // Get push constant block count
        uint32_t push_constant_count{0};
        SpvReflectResult result{spvReflectEnumeratePushConstantBlocks ( &aModule, &push_constant_count, nullptr ) };
        if ( result != SPV_REFLECT_RESULT_SUCCESS )
        {
            std::ostringstream stream;
            stream << "SPIR-V Reflect push constant enumeration failed: ( " << static_cast<int> ( result ) << " )";
            std::cout << LogLevel::Error << stream.str();
            throw std::runtime_error ( stream.str().c_str() );
        }

        if ( push_constant_count > 0 )
        {
            std::vector<SpvReflectBlockVariable*> push_constant_blocks ( push_constant_count );
            result = spvReflectEnumeratePushConstantBlocks ( &aModule, &push_constant_count, push_constant_blocks.data() );
            if ( result != SPV_REFLECT_RESULT_SUCCESS )
            {
                std::ostringstream stream;
                stream << "SPIR-V Reflect push constant enumeration failed: ( " << static_cast<int> ( result ) << " )";
                std::cout << LogLevel::Error << stream.str();
                throw std::runtime_error ( stream.str().c_str() );
            }
            for ( const auto& push_constant_block : push_constant_blocks )
            {
                /* We'll keep it simple and just look for the model matrix push constant */
                if ( push_constant_block->member_count == 1 && push_constant_block->members[0].name != nullptr &&
                     strcmp ( push_constant_block->members[0].name, "ModelMatrix" ) == 0 )
                {
                    mPushConstantModelMatrix.stageFlags |= ShaderTypeToShaderStageFlagBit.at ( aType );
                    mPushConstantModelMatrix.offset = push_constant_block->offset;
                    mPushConstantModelMatrix.size = push_constant_block->size;
                    std::cout << LogLevel::Debug << "Model Matrix Push Constant Offset: " << mPushConstantModelMatrix.offset
                              << " Size: " << mPushConstantModelMatrix.size
                              << " Shader Type: " << ShaderTypeToString.at ( aType ) << std::endl;
                }
            }
        }
    }
}
