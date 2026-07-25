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
#include "aeongames/Scene.hpp"
#include "aeongames/GpuLight.hpp"

namespace AeonGames
{
    TEST ( ShadowSettingsTests, SpotShadowResolutionUpdatesTexelSize )
    {
        Scene scene;
        GpuLight light{};
        light.type = static_cast<uint32_t> ( LightType::Spot );
        light.position_radius = Vector4 { 0.0f, 1.0f, 0.0f, 10.0f };
        light.direction_cosOuter = Vector4 { 0.0f, -1.0f, 0.0f, 0.8f };
        scene.AddLight ( light );

        GpuSpotShadowParams params{};
        EXPECT_EQ ( scene.GetSpotShadowCasters ( params, 256 ), 1u );
        EXPECT_FLOAT_EQ ( params.params[0], 1.0f / 256.0f );
    }

    TEST ( ShadowSettingsTests, PointShadowResolutionUpdatesTexelSize )
    {
        Scene scene;
        GpuLight light{};
        light.type = static_cast<uint32_t> ( LightType::Point );
        light.position_radius = Vector4 { 0.0f, 1.0f, 0.0f, 10.0f };
        scene.AddLight ( light );

        GpuPointShadowParams params{};
        EXPECT_EQ ( scene.GetPointShadowCasters ( params, 512 ), 1u );
        EXPECT_FLOAT_EQ ( params.params[0], 1.0f / 512.0f );
    }
}