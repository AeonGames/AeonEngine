/*
Copyright (C) 2017-2021,2025 Rodrigo Jose Hernandez Cordoba

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

    static const std::unordered_map<Topology, VkPrimitiveTopology> TopologyMap
    {
        {POINT_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST},
        {LINE_STRIP, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP},
        {LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
        {TRIANGLE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP},
        {TRIANGLE_FAN, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN},
        {TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
        {LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY},
        {LINE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY},
        {TRIANGLE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY},
        {TRIANGLE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY},
        {PATCH_LIST, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST}
    };

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

#if 0
    static std::string GetSamplersCode ( const Pipeline& aPipeline, uint32_t aSetNumber )
    {
        std::string samplers ( "//----SAMPLERS-START----\n" );
        {
            uint32_t sampler_binding = 0;
            for ( auto& i : aPipeline.GetSamplerDescriptors() )
            {
                samplers += "layout(set = " + std::to_string ( aSetNumber ) + ", binding = " + std::to_string ( sampler_binding ) + ", location =" + std::to_string ( sampler_binding ) + ") uniform sampler2D " + i + ";\n";
                ++sampler_binding;
            }
        }
        samplers.append ( "//----SAMPLERS-END----\n" );
        return samplers;
    }

    static std::string GetVertexShaderCode ( const Pipeline& aPipeline )
    {
        std::string vertex_shader{ "#version 450\n" };
        vertex_shader.append ( aPipeline.GetAttributes () );

        std::string transforms (
            "layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };\n"
            "layout(set = " + std::to_string ( MATRICES ) + ", binding = 0, std140) uniform Matrices{\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );

        vertex_shader.append ( transforms );

        // Properties
        vertex_shader.append (
            "layout(set = " + std::to_string ( MATERIAL ) +
            ", binding = 0,std140) uniform Material{\n" +
            aPipeline.GetProperties( ) + "};\n" );

        vertex_shader.append ( GetSamplersCode ( aPipeline, SAMPLERS ) );

        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            // Skeleton
            std::string skeleton (
                "layout(set = " + std::to_string ( SKELETON ) + ", binding = 0, std140) uniform Skeleton{\n"
                "mat4 skeleton[256];\n"
                "};\n"
            );
            vertex_shader.append ( skeleton );
        }

        vertex_shader.append ( aPipeline.GetVertexShaderCode() );
        return vertex_shader;
    }

    static std::string GetFragmentShaderCode ( const Pipeline& aPipeline )
    {
        std::string fragment_shader{"#version 450\n"};
        std::string transforms (
            "layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };\n"
            "layout(set = " + std::to_string ( MATRICES ) + ", binding = 0, std140) uniform Matrices{\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );
        fragment_shader.append ( transforms );

        fragment_shader.append ( "layout(set = " + std::to_string ( MATERIAL ) +
                                 ", binding = 0,std140) uniform Material{\n" +
                                 aPipeline.GetProperties() + "};\n" );
        fragment_shader.append ( GetSamplersCode ( aPipeline, SAMPLERS ) );

        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            // Skeleton
            std::string skeleton (
                "layout(set = " + std::to_string ( SKELETON ) + ", binding = 0, std140) uniform Skeleton{\n"
                "mat4 skeleton[256];\n"
                "};\n"
            );
            fragment_shader.append ( skeleton );
        }

        fragment_shader.append ( aPipeline.GetFragmentShaderCode() );
        return fragment_shader;
    }

    static uint32_t GetLocation ( AttributeBits aAttributeBit )
    {
        return ffs ( aAttributeBit );
    }

    static VkFormat GetFormat ( AttributeBits aAttributeBit )
    {
        return ( aAttributeBit & VertexUVBit ) ? VK_FORMAT_R32G32_SFLOAT :
               ( aAttributeBit & VertexWeightIdxBit ) ? VK_FORMAT_R8G8B8A8_UINT :
               ( aAttributeBit & VertexWeightBit ) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R32G32B32_SFLOAT;
    }

    static uint32_t GetSize ( AttributeBits aAttributeBit )
    {
        switch ( GetFormat ( aAttributeBit ) )
        {
        case VK_FORMAT_R32G32_SFLOAT:
            return sizeof ( float ) * 2;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return sizeof ( float ) * 3;
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_UNORM:
            return sizeof ( uint8_t ) * 4;
        default:
            return 0;
        }
        return 0;
    }

    static uint32_t GetOffset ( AttributeBits aAttributeBit )
    {
        uint32_t offset = 0;
        for ( uint32_t i = 1; i != aAttributeBit; i = i << 1 )
        {
            offset += GetSize ( static_cast<AttributeBits> ( i ) );
        }
        return offset;
    }
#endif

    VulkanPipeline::VulkanPipeline ( VulkanPipeline&& aVulkanPipeline ) :
        mVulkanRenderer{aVulkanPipeline.mVulkanRenderer}
    {
        std::swap ( mPipeline, aVulkanPipeline.mPipeline );
        std::swap ( mVkPipelineLayout, aVulkanPipeline.mVkPipelineLayout );
        std::swap ( mVkPipeline, aVulkanPipeline.mVkPipeline );
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
            ReflectUniforms ( module, static_cast<ShaderType> ( i ) );
            spvReflectDestroyShaderModule ( &module );
            //--------Reflection----------//
        }

        //----------------Vertex Input------------------//
        std::array<VkVertexInputBindingDescription, 1> vertex_input_binding_descriptions { {} };
        vertex_input_binding_descriptions[0].binding = 0;
        vertex_input_binding_descriptions[0].stride = sizeof ( mDefaultStride );
        vertex_input_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions ( mAttributes.size() );
        for ( size_t i = 0; i < vertex_input_attribute_descriptions.size(); ++i )
        {
            vertex_input_attribute_descriptions[i].location = mAttributes[i].location;
            vertex_input_attribute_descriptions[i].binding = 0;
            vertex_input_attribute_descriptions[i].format = mAttributes[i].format;
        }
        // Sort by location
        std::sort ( vertex_input_attribute_descriptions.begin(), vertex_input_attribute_descriptions.end(),
                    [] ( const VkVertexInputAttributeDescription & a, const VkVertexInputAttributeDescription & b )
        {
            return a.location < b.location;
        } );
        // Calculate offsets
        uint32_t offset = 0;
        for ( size_t i = 0; i < vertex_input_attribute_descriptions.size(); ++i )
        {
            vertex_input_attribute_descriptions[i].offset = offset;
            offset += VkFormatToVulkanSize.at ( mAttributes[i].format );
        }

        VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info {};
        pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipeline_vertex_input_state_create_info.pNext = nullptr;
        pipeline_vertex_input_state_create_info.flags = 0;
        pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t> ( vertex_input_binding_descriptions.size() );
        pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = vertex_input_binding_descriptions.data();
        pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t> ( vertex_input_attribute_descriptions.size() );
        pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();

        //----------------Input Assembly------------------//
        VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
        pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipeline_input_assembly_state_create_info.pNext = nullptr;
        pipeline_input_assembly_state_create_info.flags = 0;
        pipeline_input_assembly_state_create_info.topology = TopologyMap.at ( LINE_LIST );
        pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

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

        //----------------Push Constant Range------------------//
        std::array<VkPushConstantRange, 1> push_constant_ranges {};
        push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; ///@todo determine ALL stage flags based on usage.
        push_constant_ranges[0].offset = 0;
        push_constant_ranges[0].size = sizeof ( float ) * 16; // the push constant will contain just the Model Matrix

        uint32_t descriptor_set_layout_count{0};
        std::array<VkDescriptorSetLayout, 4> descriptor_set_layouts;

#if 0
        // Matrix Descriptor Set Layout
        if ( GetUniformBlock ( "Matrices"_crc32 ) )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = mVulkanRenderer.GetUniformBufferDescriptorSetLayout();
        }
        // Material Descriptor Set Layout
        if ( GetUniformBlock ( "Material"_crc32 ) )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = mVulkanRenderer.GetUniformBufferDescriptorSetLayout();
        }
        // Sampler Descriptor Set Layout
        if ( mSamplerLocations.size() )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = mVulkanRenderer.GetSamplerDescriptorSetLayout ( mSamplerLocations.size() );
        }
        //---------------Dynamic Uniform Buffer for Skeletons------------------//
        if ( aPipeline.GetAttributeBitmap() & ( VertexWeightIdxBit | VertexWeightBit ) )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = mVulkanRenderer.GetUniformBufferDynamicDescriptorSetLayout();
        }
#endif

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pNext = nullptr;
        pipeline_layout_create_info.setLayoutCount = descriptor_set_layout_count;
        pipeline_layout_create_info.pSetLayouts = descriptor_set_layout_count ? descriptor_set_layouts.data() : nullptr;
        pipeline_layout_create_info.pushConstantRangeCount = static_cast<uint32_t> ( push_constant_ranges.size() );
        pipeline_layout_create_info.pPushConstantRanges = push_constant_ranges.data();
        if ( VkResult result = vkCreatePipelineLayout ( mVulkanRenderer.GetDevice(), &pipeline_layout_create_info, nullptr, &mVkPipelineLayout ) )
        {
            std::ostringstream stream;
            stream << "Pipeline Layout creation failed: ( " << GetVulkanResultString ( result ) << " )";
            throw std::runtime_error ( stream.str().c_str() );
        }

        ///@Kwizatz Haderach Note: This needs to take into account multiple shader stages.
        std::array<VkPipelineShaderStageCreateInfo, 2> pipeline_shader_stage_create_infos{ {} };
        pipeline_shader_stage_create_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_shader_stage_create_infos[0].pNext = nullptr;
        pipeline_shader_stage_create_infos[0].flags = 0;
        pipeline_shader_stage_create_infos[0].module = shader_modules[VERT];
        pipeline_shader_stage_create_infos[0].pName = "main";
        pipeline_shader_stage_create_infos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stage_create_infos[0].pSpecializationInfo = nullptr;

        pipeline_shader_stage_create_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_shader_stage_create_infos[1].pNext = nullptr;
        pipeline_shader_stage_create_infos[1].flags = 0;
        pipeline_shader_stage_create_infos[1].module = shader_modules[FRAG];
        pipeline_shader_stage_create_infos[1].pName = "main";
        pipeline_shader_stage_create_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_shader_stage_create_infos[1].pSpecializationInfo = nullptr;

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
        graphics_pipeline_create_info.renderPass = mVulkanRenderer.GetRenderPass();
        graphics_pipeline_create_info.subpass = 0;
        graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        graphics_pipeline_create_info.basePipelineIndex = 0;

        if ( VkResult result = vkCreateGraphicsPipelines ( mVulkanRenderer.GetDevice(), VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &mVkPipeline ) )
        {
            std::ostringstream stream;
            stream << "Pipeline creation failed: ( " << GetVulkanResultString ( result ) << " )";
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
    const VkPipeline VulkanPipeline::GetPipeline() const
    {
        return mVkPipeline;
    }

    const std::vector<VulkanVariable>& VulkanPipeline::GetVertexAttributes() const
    {
        return mAttributes;
    }

    const std::vector<VulkanSamplerLocation>& VulkanPipeline::GetSamplers() const
    {
        return mSamplerLocations;
    }
#if 0
    const VulkanDescriptorSetBinding* VulkanPipeline::GetUniformBlock ( uint32_t name ) const
    {
        auto it = std::find_if ( mDescriptorSets.begin(), mDescriptorSets.end(),
                                 [name] ( const VulkanDescriptorSetBinding & block )
        {
            return block.name == name;
        } );
        return ( it != mDescriptorSets.end() ) ? & ( *it ) : nullptr;
    }
#endif
    const uint32_t VulkanPipeline::GetSamplerBinding ( uint32_t name_hash ) const
    {
        auto it = std::find_if ( mSamplerLocations.begin(), mSamplerLocations.end(),
                                 [name_hash] ( const VulkanSamplerLocation & sampler )
        {
            return sampler.name == name_hash;
        } );
        return ( it != mSamplerLocations.end() ) ? it->location : 0;
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
        mAttributes.reserve ( mAttributes.capacity() + input_vars.size() );
        for ( const auto& i : input_vars )
        {
            if ( i->built_in < 0 )
            {
                const uint32_t name_crc{crc32i ( i->name, strlen ( i->name ) ) };
                auto it = std::lower_bound ( mAttributes.begin(), mAttributes.end(), name_crc,
                                             [] ( const VulkanVariable & a, const uint32_t b )
                {
                    return a.name < b;
                } );
                mAttributes.insert ( it,
                {
                    name_crc,
                    i->location,
                    SpvReflectToVulkanFormat.at ( i->format )
                } );
                mDefaultStride += VkFormatToVulkanSize.at ( SpvReflectToVulkanFormat.at ( i->format ) );
            }
        }
    }

    void VulkanPipeline::ReflectUniforms ( SpvReflectShaderModule& aModule, ShaderType aType )
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

            mDescriptorSets.reserve ( descriptor_set_count );
            for ( const auto& descriptor_set : descriptor_sets )
            {
                auto& bindings = mDescriptorSets.emplace_back ( VulkanDescriptorSet{descriptor_set->set, {}} ).bindings;
                bindings.reserve ( descriptor_set->binding_count );
                for ( uint32_t i = 0; i < descriptor_set->binding_count; ++i )
                {
                    const SpvReflectDescriptorBinding& descriptor_set_binding = *descriptor_set->bindings[i];
                    /// @todo @Kwizatz Haderach store the descriptor type in the binding as well.
                    if ( descriptor_set_binding.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER )
                    {
                        // Process uniform blocks
                        if ( descriptor_set_binding.type_description->type_name != nullptr )
                        {
                            const uint32_t name_crc{crc32i ( descriptor_set_binding.type_description->type_name, strlen ( descriptor_set_binding.type_description->type_name ) ) };
                            auto it = std::lower_bound ( bindings.begin(), bindings.end(), name_crc,
                                                         [] ( const VulkanDescriptorSetBinding & a, const uint32_t b )
                            {
                                return a.name < b;
                            } );
                            if ( it != bindings.end() && it->name == name_crc )
                            {
                                // Already exists, update stage flags
                                it->stageFlags |= ShaderTypeToShaderStageFlagBit.at ( aType );
                                continue;
                            }

                            bindings.insert ( it,
                            {
                                name_crc,
                                descriptor_set_binding.set,
                                descriptor_set_binding.binding,
                                descriptor_set_binding.block.size,
                                static_cast<VkShaderStageFlags> ( ShaderTypeToShaderStageFlagBit.at ( aType ) ),
                            } );

                            std::cout << LogLevel::Debug << "Uniform Block: " << descriptor_set_binding.type_description->type_name
                                      << " (crc: " << std::hex << name_crc << std::dec
                                      << ", set: " << descriptor_set_binding.set << ", binding: " << descriptor_set_binding.binding
                                      << ", size: " << descriptor_set_binding.block.size << ")" << std::endl;


#if 0                                       // Process uniform block members
                            for ( uint32_t j = 0; j < binding.block.member_count; ++j )
                            {
                                const SpvReflectBlockVariable& member = binding.block.members[j];
                                if ( member.name != nullptr )
                                {
                                    VulkanVariable uniform_var;
                                    uniform_var.name = crc32i ( member.name, strlen ( member.name ) );
                                    uniform_var.location = member.offset;
                                    uniform_var.format = VK_FORMAT_UNDEFINED; // Not applicable for uniforms
                                    uniform_block.uniforms.push_back ( uniform_var );

                                    std::cout << "  - Uniform: " << member.name
                                              << " (crc: " << std::hex << uniform_var.name << std::dec
                                              << ", offset: " << member.offset << ")" << std::endl;
                                }
                            }
#endif
                        }
                    }
                    else if ( descriptor_set_binding.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                              descriptor_set_binding.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                              descriptor_set_binding.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER )
                    {
                        // Process samplers
                        if ( descriptor_set_binding.name != nullptr )
                        {
                            const uint32_t name_crc{crc32i ( descriptor_set_binding.name, strlen ( descriptor_set_binding.name ) ) };
                            auto it = std::lower_bound ( mSamplerLocations.begin(), mSamplerLocations.end(), name_crc,
                                                         [] ( const VulkanSamplerLocation & a, const uint32_t b )
                            {
                                return a.name < b;
                            } );
                            if ( it != mSamplerLocations.end() && it->name == name_crc )
                            {
                                // Already exists, skip
                                continue;
                            }
                            mSamplerLocations.insert ( it,
                            {
                                name_crc,
                                descriptor_set_binding.binding
                            } );
                            std::cout << LogLevel::Debug << "Sampler: " << descriptor_set_binding.name
                                      << " (crc: " << std::hex << name_crc << std::dec
                                      << ", binding: " << descriptor_set_binding.binding << ")" << std::endl;
                        }
                    }
                }
            }
        }

        // Sort samplers by name for faster lookup
        std::sort ( mSamplerLocations.begin(), mSamplerLocations.end(),
                    [] ( const VulkanSamplerLocation & a, const VulkanSamplerLocation & b )
        {
            return a.name < b.name;
        } );
    }
}
