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
#include <fstream>
#include <regex>
#include "aeongames/AeonEngine.h"
#include "aeongames/CRC.h"
#include "aeongames/ProtoBufClasses.h"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "pipeline.pb.h"
#include "property.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include <vulkan/vulkan.h>
#include "VulkanPipeline.h"
#include "VulkanImage.h"
#include "VulkanRenderer.h"
#include "VulkanMaterial.h"
#include "VulkanUtilities.h"
#include "VulkanBuffer.h"
#include "SPIR-V/CompilerLinker.h"

namespace AeonGames
{
    VulkanPipeline::VulkanPipeline ( const VulkanRenderer& aVulkanRenderer, uint32_t aPath ) :
        mVulkanRenderer { aVulkanRenderer }, mDefaultMaterial {mVulkanRenderer}
    {
        if ( aPath )
        {
            Load ( aPath );
        }
    }

    VulkanPipeline::~VulkanPipeline()
    {
        Unload();
    }

    const VulkanMaterial& VulkanPipeline::GetDefaultMaterial() const
    {
        return mDefaultMaterial;
    }

    const VkPipelineLayout VulkanPipeline::GetPipelineLayout() const
    {
        return mVkPipelineLayout;
    }
    const VkPipeline VulkanPipeline::GetPipeline() const
    {
        return mVkPipeline;
    }

    void VulkanPipeline::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }
    void VulkanPipeline::Load ( uint32_t aId )
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
    void VulkanPipeline::Load ( const void* aBuffer, size_t aBufferSize )
    {
        static PipelineBuffer pipeline_buffer;
        LoadProtoBufObject ( pipeline_buffer, aBuffer, aBufferSize, "AEONPRG" );
        Load ( pipeline_buffer );
        pipeline_buffer.Clear();
    }

    enum AttributeBits
    {
        VertexPositionBit = 0x1,
        VertexNormalBit = 0x2,
        VertexTangentBit = 0x4,
        VertexBitangentBit = 0x8,
        VertexUVBit = 0x10,
        VertexWeightIndicesBit = 0x20,
        VertexWeightsBit = 0x40,
        VertexAllBits = VertexPositionBit |
                        VertexNormalBit |
                        VertexTangentBit |
                        VertexBitangentBit |
                        VertexUVBit |
                        VertexWeightIndicesBit |
                        VertexWeightsBit
    };

    enum AttributeFormat
    {
        Vector2Float,
        Vector3Float,
        Vector4Byte,
        Vector4ByteNormalized,
    };

    static const std::array<const char*, 7> AttributeStrings
    {
        {
            "VertexPosition",
            "VertexNormal",
            "VertexTangent",
            "VertexBitangent",
            "VertexUV",
            "VertexWeightIndices",
            "VertexWeights"
        }
    };

    static const std::array<const char*, 7> AttributeTypes
    {
        {
            "vec3",
            "vec3",
            "vec3",
            "vec3",
            "vec2",
            "uvec4",
            "vec4"
        }
    };

    /**@note static const regex: construct once, use for ever.*/
    static const std::regex attribute_regex (
        "\\bVertexPosition\\b|"
        "\\bVertexNormal\\b|"
        "\\bVertexTangent\\b|"
        "\\bVertexBitangent\\b|"
        "\\bVertexUV\\b|"
        "\\bVertexWeightIndices\\b|"
        "\\bVertexWeights\\b" );

    static uint32_t GetAttributes ( const PipelineBuffer& aPipelineBuffer )
    {
        std::smatch attribute_matches;
        uint32_t attributes{};
        std::string code = aPipelineBuffer.vertex_shader().code();
        while ( std::regex_search ( code, attribute_matches, attribute_regex ) )
        {
            for ( uint32_t i = 0; i < AttributeStrings.size(); ++i )
            {
                if ( attribute_matches.str().substr ( 6 ) == AttributeStrings[i] + 6 )
                {
                    if ( ! ( attributes & ( 1 << i ) ) )
                    {
                        attributes |= ( 1 << i );
                    }
                    break;
                }
            }
            code = attribute_matches.suffix();
        }
        return attributes;
    }

    static std::string GetVertexShaderCode ( const PipelineBuffer& aPipelineBuffer )
    {
        std::string vertex_shader{ "#version 450\n" };
        uint32_t attributes{GetAttributes ( aPipelineBuffer ) };
        for ( uint32_t i = 0; i < 7; ++i )
        {
            if ( attributes & ( 1 << i ) )
            {
                vertex_shader.append ( "layout(location = " );
                vertex_shader.append ( std::to_string ( i ) );
                vertex_shader.append ( ") in " );
                vertex_shader.append ( AttributeTypes[i] );
                vertex_shader.append ( " " );
                vertex_shader.append ( AttributeStrings[i] );
                vertex_shader.append ( ";\n" );
            }
        }

        uint32_t set_count = 0;

        std::string transforms (
            "layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };\n"
            "layout(set = " + std::to_string ( set_count++ ) + ", binding = 0, std140) uniform Matrices{\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );

        vertex_shader.append ( transforms );

        if ( aPipelineBuffer.default_material().property().size() )
        {
            std::string properties (
                "layout(set = " + std::to_string ( set_count++ ) + ", binding = 0,std140) uniform Properties{\n"
            );
            for ( auto& i : aPipelineBuffer.default_material().property() )
            {
                switch ( i.value_case() )
                {
                case PropertyBuffer::ValueCase::kScalarFloat:
                    properties += "float " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kScalarUint:
                    properties += "uint " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kScalarInt:
                    properties += "int " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kVector2:
                    properties += "vec2 " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kVector3:
                    properties += "vec3 " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kVector4:
                    properties += "vec4 " + i.name() + ";\n";
                    break;
                default:
                    throw std::runtime_error ( "Unknown Type." );
                }
            }

            properties.append ( "};\n" );

            std::string samplers ( "//----SAMPLERS-START----\n" );
            {
                uint32_t sampler_binding = 0;
                for ( auto& i : aPipelineBuffer.default_material().sampler() )
                {
                    samplers += "layout(set = " + std::to_string ( set_count ) + ", binding = " + std::to_string ( sampler_binding ) + ", location =" + std::to_string ( sampler_binding ) + ") uniform sampler2D " + i.name() + ";\n";
                    ++sampler_binding;
                }
                set_count += 1;
            }
            samplers.append ( "//----SAMPLERS-END----\n" );

            vertex_shader.append ( properties );
            vertex_shader.append ( samplers );
        }

        // Skeleton needs rework
        if ( attributes & ( VertexWeightIndicesBit | VertexWeightsBit ) )
        {
            std::string skeleton (
                "layout(set = 2, binding = 2, std140) uniform Skeleton{\n"
                "mat4 skeleton[256];\n"
                "};\n"
            );
            vertex_shader.append ( skeleton );
        }

        switch ( aPipelineBuffer.vertex_shader().source_case() )
        {
        case ShaderBuffer::SourceCase::kCode:
        {
            vertex_shader.append ( aPipelineBuffer.vertex_shader().code() );
        }
        break;
        default:
            throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
        }
        return vertex_shader;
    }

    static std::string GetFragmentShaderCode ( const PipelineBuffer& aPipelineBuffer )
    {
        uint32_t set_count = 0;
        std::string fragment_shader{"#version 450\n"};
        std::string transforms (
            "layout(push_constant) uniform PushConstant { mat4 ModelMatrix; };\n"
            "layout(set = " + std::to_string ( set_count++ ) + ", binding = 0, std140) uniform Matrices{\n"
            "mat4 ProjectionMatrix;\n"
            "mat4 ViewMatrix;\n"
            "};\n"
        );

        fragment_shader.append ( transforms );

        if ( aPipelineBuffer.default_material().property().size() )
        {
            std::string properties (
                "layout(set = " + std::to_string ( set_count++ ) + ", binding = 0,std140) uniform Properties{\n"
            );
            for ( auto& i : aPipelineBuffer.default_material().property() )
            {
                switch ( i.value_case() )
                {
                case PropertyBuffer::ValueCase::kScalarFloat:
                    properties += "float " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kScalarUint:
                    properties += "uint " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kScalarInt:
                    properties += "int " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kVector2:
                    properties += "vec2 " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kVector3:
                    properties += "vec3 " + i.name() + ";\n";
                    break;
                case PropertyBuffer::ValueCase::kVector4:
                    properties += "vec4 " + i.name() + ";\n";
                    break;
                default:
                    throw std::runtime_error ( "Unknown Type." );
                }
            }
            properties.append ( "};\n" );

            std::string samplers ( "//----SAMPLERS-START----\n" );
            {
                uint32_t sampler_binding = 0;
                for ( auto& i : aPipelineBuffer.default_material().sampler() )
                {
                    samplers += "layout(set = " + std::to_string ( set_count ) + ", binding = " + std::to_string ( sampler_binding ) + ", location =" + std::to_string ( sampler_binding ) + ") uniform sampler2D " + i.name() + ";\n";
                    ++sampler_binding;
                }
                set_count += 1;
            }
            samplers.append ( "//----SAMPLERS-END----\n" );

            fragment_shader.append ( properties );
            fragment_shader.append ( samplers );
        }
        switch ( aPipelineBuffer.fragment_shader().source_case() )
        {
        case ShaderBuffer::SourceCase::kCode:
            fragment_shader.append ( aPipelineBuffer.fragment_shader().code() );
            break;
        default:
            throw std::runtime_error ( "ByteCode Shader Type not implemented yet." );
        }
        return fragment_shader;
    }

    static uint32_t GetLocation ( AttributeBits aAttributeBit )
    {
        return ffs ( aAttributeBit );
    }

    static VkFormat GetFormat ( AttributeBits aAttributeBit )
    {
        return ( aAttributeBit & VertexUVBit ) ? VK_FORMAT_R32G32_SFLOAT :
               ( aAttributeBit & VertexWeightIndicesBit ) ? VK_FORMAT_R8G8B8A8_UINT :
               ( aAttributeBit & VertexWeightsBit ) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R32G32B32_SFLOAT;
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

    static VkPrimitiveTopology GetVulkanTopology ( PipelineBuffer_Topology aTopology )
    {
        switch ( aTopology )
        {
        case PipelineBuffer_Topology::PipelineBuffer_Topology_POINT_LIST:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_LINE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_LINE_LIST:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_FAN:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_LIST:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_LINE_LIST_WITH_ADJACENCY:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_LINE_STRIP_WITH_ADJACENCY:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_LIST_WITH_ADJACENCY:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_TRIANGLE_STRIP_WITH_ADJACENCY:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
        case PipelineBuffer_Topology::PipelineBuffer_Topology_PATCH_LIST:
            return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        default:
            break;
        }
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }

    void VulkanPipeline::Load ( const PipelineBuffer& aPipelineBuffer )
    {
        mDefaultMaterial.Load ( aPipelineBuffer.default_material() );
        std::string vertex_shader_code = GetVertexShaderCode ( aPipelineBuffer );
        std::string fragment_shader_code = GetFragmentShaderCode ( aPipelineBuffer );

        CompilerLinker compiler_linker;
        compiler_linker.AddShaderSource ( EShLanguage::EShLangVertex, vertex_shader_code.c_str() );
        compiler_linker.AddShaderSource ( EShLanguage::EShLangFragment, fragment_shader_code.c_str() );
        if ( CompilerLinker::FailCode result = compiler_linker.CompileAndLink() )
        {
            std::ostringstream stream;
            stream << vertex_shader_code << std::endl;
            stream << fragment_shader_code << std::endl;
            stream << ( ( result == CompilerLinker::EFailCompile ) ? "Compilation" : "Linking" ) <<
                   " Error:" << std::endl << compiler_linker.GetLog();
            throw std::runtime_error ( stream.str().c_str() );
        }
        {
            VkShaderModuleCreateInfo shader_module_create_info{};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = compiler_linker.GetSpirV ( EShLanguage::EShLangVertex ).size() * sizeof ( uint32_t );
            shader_module_create_info.pCode = compiler_linker.GetSpirV ( EShLanguage::EShLangVertex ).data();

            if ( VkResult result = vkCreateShaderModule ( mVulkanRenderer.GetDevice(), &shader_module_create_info, nullptr, &mVkShaderModules[ffs ( VK_SHADER_STAGE_VERTEX_BIT )] ) )
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
            if ( VkResult result = vkCreateShaderModule ( mVulkanRenderer.GetDevice(), &shader_module_create_info, nullptr, &mVkShaderModules[ffs ( VK_SHADER_STAGE_FRAGMENT_BIT )] ) )
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
        uint32_t attributes = GetAttributes ( aPipelineBuffer );
        std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions ( popcount ( attributes ) );
        for ( auto& i : vertex_input_attribute_descriptions )
        {
            uint32_t attribute_bit = ( 1 << ffs ( attributes ) );
            i.location = GetLocation ( static_cast<AttributeBits> ( attribute_bit ) );
            i.binding = 0;
            i.format = GetFormat ( static_cast<AttributeBits> ( attribute_bit ) );
            i.offset = GetOffset ( static_cast<AttributeBits> ( attribute_bit ) );
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
        pipeline_input_assembly_state_create_info.topology = GetVulkanTopology ( aPipelineBuffer.topology() );
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
        pipeline_multisample_state_create_info.sampleShadingEnable = VK_FALSE;
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
        std::array<VkPushConstantRange, 1> push_constant_ranges {};
        push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; ///@todo determine ALL stage flags based on usage.
        push_constant_ranges[0].offset = 0;
        push_constant_ranges[0].size = sizeof ( float ) * 16; // the push constant will contain just the Model Matrix

        std::array<VkDescriptorSetLayout, 3> descriptor_set_layouts;

        uint32_t descriptor_set_layout_count = 0;

        if ( mVulkanRenderer.GetMatrixDescriptorSetLayout() != VK_NULL_HANDLE )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = mVulkanRenderer.GetMatrixDescriptorSetLayout();
        }
        for ( auto& i : mDefaultMaterial.GetDescriptorSetLayouts() )
        {
            descriptor_set_layouts[descriptor_set_layout_count++] = i;
        }

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
    }

    void VulkanPipeline::Unload()
    {
        mDefaultMaterial.Unload();
        if ( mVkPipeline != VK_NULL_HANDLE )
        {
            vkDestroyPipeline ( mVulkanRenderer.GetDevice(), mVkPipeline, nullptr );
            mVkPipeline = VK_NULL_HANDLE;
        }
        if ( mVkPipelineLayout != VK_NULL_HANDLE )
        {
            vkDestroyPipelineLayout ( mVulkanRenderer.GetDevice(), mVkPipelineLayout, nullptr );
            mVkPipelineLayout = VK_NULL_HANDLE;
        }
        for ( auto& i : mVkShaderModules )
        {
            if ( i != VK_NULL_HANDLE )
            {
                vkDestroyShaderModule ( mVulkanRenderer.GetDevice(), i, nullptr );
                i = VK_NULL_HANDLE;
            }
        }
    }
#if 0
    void VulkanPipeline::InitializeSkeletonUniform()
    {
        if ( GetAttributes() & ( Pipeline::VertexWeightIndicesBit | Pipeline::VertexWeightsBit ) )
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
}
