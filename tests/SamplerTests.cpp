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

#include "gtest/gtest.h"
#include "aeongames/Material.hpp"
#include "aeongames/ProtoBufClasses.hpp"
#include "material.pb.h"

namespace AeonGames
{
    TEST ( SamplerTests, LegacyDefaultsMatchTextureDefaults )
    {
        MaterialMsg message;
        auto* sampler = message.add_sampler();
        sampler->set_name ( "DiffuseMap" );
        sampler->mutable_image()->set_path ( "textures/default.png" );

        Material material;
        material.LoadFromPBMsg ( message );

        ASSERT_EQ ( material.GetSamplers().size(), 1u );
        const Material::SamplerState& state = std::get<2> ( material.GetSamplers().front() );
        EXPECT_EQ ( state.min_filter, Material::SamplerFilter::LINEAR );
        EXPECT_EQ ( state.mag_filter, Material::SamplerFilter::LINEAR );
        EXPECT_EQ ( state.mipmap_mode, Material::SamplerMipmapMode::LINEAR );
        EXPECT_FALSE ( state.mipmap_enable );
        EXPECT_EQ ( state.address_mode_u, Material::SamplerAddressMode::REPEAT );
        EXPECT_FALSE ( state.anisotropy_enable );
        EXPECT_FLOAT_EQ ( state.max_anisotropy, 1.0f );
        EXPECT_FALSE ( state.compare_enable );
        EXPECT_EQ ( state.border_color, Material::SamplerBorderColor::TRANSPARENT_BLACK );
    }

    TEST ( SamplerTests, LoadsCustomSamplerState )
    {
        MaterialMsg message;
        auto* sampler = message.add_sampler();
        sampler->set_name ( "NormalMap" );
        sampler->mutable_image()->set_path ( "textures/flat_normal.png" );
        sampler->set_min_filter ( SamplerMsg_Filter_FILTER_NEAREST );
        sampler->set_mag_filter ( SamplerMsg_Filter_FILTER_NEAREST );
        sampler->set_mipmap_mode ( SamplerMsg_MipmapMode_MIPMAP_NEAREST );
        sampler->set_mipmap_enable ( true );
        sampler->set_address_mode_u ( SamplerMsg_AddressMode_ADDRESS_CLAMP_TO_EDGE );
        sampler->set_address_mode_v ( SamplerMsg_AddressMode_ADDRESS_CLAMP_TO_BORDER );
        sampler->set_anisotropy_enable ( true );
        sampler->set_max_anisotropy ( 8.0f );
        sampler->set_mip_lod_bias ( 0.25f );
        sampler->set_min_lod ( 1.0f );
        sampler->set_max_lod ( 5.0f );
        sampler->set_compare_enable ( true );
        sampler->set_compare_op ( SamplerMsg_CompareOp_COMPARE_GREATER_OR_EQUAL );
        sampler->set_border_color ( SamplerMsg_BorderColor_BORDER_OPAQUE_WHITE );

        Material material;
        material.LoadFromPBMsg ( message );

        ASSERT_EQ ( material.GetSamplers().size(), 1u );
        const Material::SamplerState& state = std::get<2> ( material.GetSamplers().front() );
        EXPECT_EQ ( state.min_filter, Material::SamplerFilter::NEAREST );
        EXPECT_EQ ( state.mag_filter, Material::SamplerFilter::NEAREST );
        EXPECT_EQ ( state.mipmap_mode, Material::SamplerMipmapMode::NEAREST );
        EXPECT_TRUE ( state.mipmap_enable );
        EXPECT_EQ ( state.address_mode_u, Material::SamplerAddressMode::CLAMP_TO_EDGE );
        EXPECT_EQ ( state.address_mode_v, Material::SamplerAddressMode::CLAMP_TO_BORDER );
        EXPECT_TRUE ( state.anisotropy_enable );
        EXPECT_FLOAT_EQ ( state.max_anisotropy, 8.0f );
        EXPECT_FLOAT_EQ ( state.mip_lod_bias, 0.25f );
        EXPECT_FLOAT_EQ ( state.min_lod, 1.0f );
        EXPECT_FLOAT_EQ ( state.max_lod, 5.0f );
        EXPECT_TRUE ( state.compare_enable );
        EXPECT_EQ ( state.compare_op, Material::SamplerCompareOp::GREATER_OR_EQUAL );
        EXPECT_EQ ( state.border_color, Material::SamplerBorderColor::OPAQUE_WHITE );
    }
}