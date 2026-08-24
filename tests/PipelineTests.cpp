/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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

#include <cstdint>
#include "gtest/gtest.h"
#include "aeongames/CRC.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/ProtoBufClasses.hpp"
#include "pipeline.pb.h"

namespace AeonGames
{
    TEST ( PipelineTests, LegacyDefaultsPreserveRendererState )
    {
        PipelineMsg message;
        Pipeline pipeline;
        pipeline.LoadFromPBMsg ( message );

        EXPECT_EQ ( pipeline.GetRasterState().cull_mode, PipelineCullMode::BACK );
        EXPECT_EQ ( pipeline.GetRasterState().front_face, PipelineFrontFace::COUNTER_CLOCKWISE );
        EXPECT_EQ ( pipeline.GetRasterState().polygon_mode, PipelinePolygonMode::FILL );
        EXPECT_EQ ( pipeline.GetRasterState().depth_bias, PipelineToggle::ENABLED );
        EXPECT_EQ ( pipeline.GetDepthStencilState().depth_test, PipelineToggle::ENABLED );
        EXPECT_EQ ( pipeline.GetDepthStencilState().depth_write, PipelineToggle::ENABLED );
        EXPECT_EQ ( pipeline.GetDepthStencilState().depth_compare, PipelineCompareOp::LESS_OR_EQUAL );
        EXPECT_EQ ( pipeline.GetBlendState().enabled, PipelineToggle::ENABLED );
        EXPECT_EQ ( pipeline.GetBlendState().source_color, PipelineBlendFactor::SOURCE_ALPHA );
        EXPECT_EQ ( pipeline.GetBlendState().destination_color, PipelineBlendFactor::ONE_MINUS_SOURCE_ALPHA );
        EXPECT_EQ ( pipeline.GetMultisampleState().sample_count, PipelineSampleCount::ONE );
        EXPECT_EQ ( pipeline.GetMultisampleState().alpha_to_coverage, PipelineToggle::ENABLED );
    }

    TEST ( PipelineTests, LoadsRenderState )
    {
        PipelineMsg message;
        auto* raster = message.mutable_raster_state();
        raster->set_cull_mode ( PipelineMsg_CullMode_CULL_NONE );
        raster->set_front_face ( PipelineMsg_FrontFace_FRONT_CLOCKWISE );
        raster->set_polygon_mode ( PipelineMsg_PolygonMode_POLYGON_LINE );
        raster->set_depth_bias_constant ( 4.0f );
        raster->set_depth_bias_slope ( 2.5f );

        auto* depth_stencil = message.mutable_depth_stencil();
        depth_stencil->set_depth_test ( PipelineMsg_Toggle_DISABLED );
        depth_stencil->set_depth_write ( PipelineMsg_Toggle_ENABLED );
        depth_stencil->set_depth_compare ( PipelineMsg_CompareOp_COMPARE_LESS );
        depth_stencil->set_stencil_test ( PipelineMsg_Toggle_ENABLED );
        depth_stencil->set_stencil_reference ( 7 );
        depth_stencil->set_stencil_compare_mask ( 0x0f );

        auto* blend = message.mutable_blend();
        blend->set_enabled ( PipelineMsg_Toggle_DISABLED );
        blend->set_color_write_mask ( 3 );
        blend->set_source_color ( PipelineMsg_BlendFactor_BLEND_ONE );
        blend->set_destination_color ( PipelineMsg_BlendFactor_BLEND_ZERO );
        blend->set_color_operation ( PipelineMsg_BlendOp_BLEND_REVERSE_SUBTRACT );

        auto* multisample = message.mutable_multisample();
        multisample->set_sample_count ( PipelineMsg_SampleCount_SAMPLE_COUNT_4 );
        multisample->set_sample_shading ( PipelineMsg_Toggle_ENABLED );
        multisample->set_min_sample_shading ( 0.5f );
        multisample->set_alpha_to_coverage ( PipelineMsg_Toggle_DISABLED );

        Pipeline pipeline;
        pipeline.LoadFromPBMsg ( message );

        EXPECT_EQ ( pipeline.GetRasterState().cull_mode, PipelineCullMode::NONE );
        EXPECT_EQ ( pipeline.GetRasterState().front_face, PipelineFrontFace::CLOCKWISE );
        EXPECT_EQ ( pipeline.GetRasterState().polygon_mode, PipelinePolygonMode::LINE );
        EXPECT_FLOAT_EQ ( pipeline.GetRasterState().depth_bias_constant, 4.0f );
        EXPECT_FLOAT_EQ ( pipeline.GetRasterState().depth_bias_slope, 2.5f );
        EXPECT_EQ ( pipeline.GetDepthStencilState().depth_test, PipelineToggle::DISABLED );
        EXPECT_EQ ( pipeline.GetDepthStencilState().depth_write, PipelineToggle::ENABLED );
        EXPECT_EQ ( pipeline.GetDepthStencilState().depth_compare, PipelineCompareOp::LESS );
        EXPECT_EQ ( pipeline.GetDepthStencilState().stencil_reference, 7u );
        EXPECT_EQ ( pipeline.GetBlendState().enabled, PipelineToggle::DISABLED );
        EXPECT_EQ ( pipeline.GetBlendState().color_write_mask, 3u );
        EXPECT_EQ ( pipeline.GetBlendState().color_operation, PipelineBlendOp::REVERSE_SUBTRACT );
        EXPECT_EQ ( pipeline.GetMultisampleState().sample_count, PipelineSampleCount::FOUR );
        EXPECT_EQ ( pipeline.GetMultisampleState().sample_shading, PipelineToggle::ENABLED );
        EXPECT_FLOAT_EQ ( pipeline.GetMultisampleState().min_sample_shading, 0.5f );
        EXPECT_EQ ( pipeline.GetMultisampleState().alpha_to_coverage, PipelineToggle::DISABLED );
    }

    TEST ( PipelineTests, BlendEnabledPipelinesDisableDepthWritesByDefault )
    {
        PipelineMsg message;
        message.mutable_blend()->set_enabled ( PipelineMsg_Toggle_ENABLED );
        message.mutable_blend()->set_source_color ( PipelineMsg_BlendFactor_BLEND_SOURCE_ALPHA );
        message.mutable_blend()->set_destination_color ( PipelineMsg_BlendFactor_BLEND_ONE_MINUS_SOURCE_ALPHA );
        message.mutable_blend()->set_source_alpha ( PipelineMsg_BlendFactor_BLEND_SOURCE_ALPHA );
        message.mutable_blend()->set_destination_alpha ( PipelineMsg_BlendFactor_BLEND_ONE_MINUS_SOURCE_ALPHA );

        Pipeline pipeline;
        pipeline.LoadFromPBMsg ( message );

        EXPECT_EQ ( pipeline.GetBlendState().enabled, PipelineToggle::ENABLED );
        EXPECT_EQ ( pipeline.GetDepthStencilState().depth_write, PipelineToggle::DISABLED );
        EXPECT_EQ ( pipeline.GetDepthStencilState().depth_test, PipelineToggle::ENABLED );
    }

    TEST ( PipelineTests, ShaderInterfaceResolvesRendererVariant )
    {
        PipelineMsg message;
        auto& metal_interfaces = ( *message.mutable_shader_interface() ) ["Metal"];
        auto* metal_compute = metal_interfaces.add_stage();
        metal_compute->set_stage ( PipelineMsg_ShaderInterface_Stage_STAGE_COMPUTE );
        metal_compute->set_compute_stage_index ( 1 );
        metal_compute->set_entry_point ( "main0" );
        metal_compute->set_local_size_x ( 64 );
        metal_compute->set_local_size_y ( 2 );
        metal_compute->set_local_size_z ( 1 );
        auto* output = metal_compute->add_resource();
        output->set_name ( "Output" );
        output->set_set ( 3 );
        output->set_binding ( 7 );
        output->set_count ( 1 );
        output->set_type ( PipelineMsg_ShaderResource_Type_STORAGE_BUFFER );
        auto* position = metal_compute->add_input();
        position->set_name ( "VertexPosition" );
        position->set_location ( 0 );
        position->set_components ( 3 );
        position->set_scalar_type ( PipelineMsg_ShaderInterface_Input_ScalarType_SCALAR_FLOAT );

        auto& default_interfaces = ( *message.mutable_shader_interface() ) [""];
        auto* default_vertex = default_interfaces.add_stage();
        default_vertex->set_stage ( PipelineMsg_ShaderInterface_Stage_STAGE_VERTEX );
        default_vertex->set_entry_point ( "main" );

        Pipeline pipeline;
        pipeline.LoadFromPBMsg ( message );

        const ShaderInterface* compute = pipeline.GetShaderInterface ( COMP, "Metal", 1 );
        ASSERT_NE ( compute, nullptr );
        EXPECT_EQ ( compute->entry_point, "main0" );
        EXPECT_EQ ( compute->local_size, ( std::array<uint32_t, 3> {64, 2, 1} ) );
        ASSERT_EQ ( compute->resources.size(), 1u );
        EXPECT_EQ ( compute->resources[0].name_hash, "Output"_crc32 );
        EXPECT_EQ ( compute->resources[0].set, 3u );
        EXPECT_EQ ( compute->resources[0].binding, 7u );
        EXPECT_EQ ( compute->resources[0].type, ShaderResourceType::StorageBuffer );
        ASSERT_EQ ( compute->inputs.size(), 1u );
        EXPECT_EQ ( compute->inputs[0].name_hash, "VertexPosition"_crc32 );
        EXPECT_EQ ( compute->inputs[0].components, 3u );
        EXPECT_EQ ( compute->inputs[0].scalar_type, ShaderInputScalarType::Float );

        EXPECT_EQ ( pipeline.GetShaderInterface ( COMP, "Metal", 0 ), nullptr );
        const ShaderInterface* fallback = pipeline.GetShaderInterface ( VERT, "Other" );
        ASSERT_NE ( fallback, nullptr );
        EXPECT_EQ ( fallback->entry_point, "main" );
    }
}