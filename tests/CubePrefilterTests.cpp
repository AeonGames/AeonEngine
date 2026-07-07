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
#include <cmath>
#include <algorithm>
#include <vector>
#include "aeongames/Texture.hpp"
#include "gtest/gtest.h"

namespace AeonGames
{
    namespace
    {
        constexpr float kPi = 3.14159265358979323846f;

        // Builds a synthetic equirectangular image (aWidth*aHeight*3 floats)
        // whose RGB encodes the sampling direction, remapped from [-1,1] to
        // [0,1], using the same +Z-up mapping the prefilter samples with. This
        // lets a test read a cube-face texel back and recover the world
        // direction that face+texel is expected to represent.
        std::vector<float> MakeDirectionEquirect ( int aWidth, int aHeight )
        {
            std::vector<float> pixels ( static_cast<size_t> ( aWidth ) * aHeight * 3 );
            for ( int y = 0; y < aHeight; ++y )
            {
                const float v = ( y + 0.5f ) / aHeight;
                const float theta = v * kPi;
                for ( int x = 0; x < aWidth; ++x )
                {
                    const float u = ( x + 0.5f ) / aWidth;
                    const float phi = ( u - 0.5f ) * 2.0f * kPi;
                    const float dx = std::sin ( theta ) * std::cos ( phi );
                    const float dy = std::sin ( theta ) * std::sin ( phi );
                    const float dz = std::cos ( theta );
                    float* p = pixels.data() + ( static_cast<size_t> ( y ) * aWidth + x ) * 3;
                    p[0] = dx * 0.5f + 0.5f;
                    p[1] = dy * 0.5f + 0.5f;
                    p[2] = dz * 0.5f + 0.5f;
                }
            }
            return pixels;
        }
    }

    // A constant environment must prefilter to that same constant everywhere
    // (the convolution is energy preserving) and produce the documented
    // six-faces-per-mip RGBA layout at each mip size.
    TEST ( PrefilterCube, ConstantEnvironmentAndLayout )
    {
        const int width = 32;
        const int height = 16;
        std::vector<float> equirect ( static_cast<size_t> ( width ) * height * 3 );
        for ( size_t i = 0; i < equirect.size(); i += 3 )
        {
            equirect[i + 0] = 0.25f;
            equirect[i + 1] = 0.5f;
            equirect[i + 2] = 0.75f;
        }
        const uint32_t face_size = 8;
        const uint32_t mip_count = 4;
        std::vector<std::vector<float>> mips;
        ASSERT_TRUE ( PrefilterEnvironmentCube ( equirect.data(), width, height, face_size, mip_count, mips ) );
        ASSERT_EQ ( mips.size(), mip_count );
        for ( uint32_t mip = 0; mip < mip_count; ++mip )
        {
            const uint32_t face = std::max<uint32_t> ( 1u, face_size >> mip );
            ASSERT_EQ ( mips[mip].size(), static_cast<size_t> ( 6 ) * face * face * 4 );
            for ( size_t texel = 0; texel < mips[mip].size(); texel += 4 )
            {
                EXPECT_NEAR ( mips[mip][texel + 0], 0.25f, 1e-3f );
                EXPECT_NEAR ( mips[mip][texel + 1], 0.5f, 1e-3f );
                EXPECT_NEAR ( mips[mip][texel + 2], 0.75f, 1e-3f );
                EXPECT_NEAR ( mips[mip][texel + 3], 1.0f, 1e-6f );
            }
        }
    }

    // Each cube face's centre texel must read back the direction of that face's
    // axis, validating the shared Vulkan/OpenGL face convention
    // (+X, -X, +Y, -Y, +Z, -Z) against the +Z-up equirect mapping. Uses mip 0
    // (a sharp resample, no GGX blur) and an odd face size so the centre texel
    // lands exactly on the face axis.
    TEST ( PrefilterCube, FaceCentresMatchAxisDirections )
    {
        const int width = 128;
        const int height = 64;
        const std::vector<float> equirect = MakeDirectionEquirect ( width, height );
        const uint32_t face_size = 15;
        std::vector<std::vector<float>> mips;
        ASSERT_TRUE ( PrefilterEnvironmentCube ( equirect.data(), width, height, face_size, 1u, mips ) );
        ASSERT_EQ ( mips.size(), 1u );

        const float axis[6][3] =
        {
            {  1.0f,  0.0f,  0.0f }, // +X
            { -1.0f,  0.0f,  0.0f }, // -X
            {  0.0f,  1.0f,  0.0f }, // +Y
            {  0.0f, -1.0f,  0.0f }, // -Y
            {  0.0f,  0.0f,  1.0f }, // +Z
            {  0.0f,  0.0f, -1.0f }, // -Z
        };
        const uint32_t centre = face_size / 2;
        for ( int face = 0; face < 6; ++face )
        {
            const float* face_pixels = mips[0].data() + static_cast<size_t> ( face ) * face_size * face_size * 4;
            const float* texel = face_pixels + ( static_cast<size_t> ( centre ) * face_size + centre ) * 4;
            EXPECT_NEAR ( texel[0], axis[face][0] * 0.5f + 0.5f, 0.05f ) << "face " << face << " channel x";
            EXPECT_NEAR ( texel[1], axis[face][1] * 0.5f + 0.5f, 0.05f ) << "face " << face << " channel y";
            EXPECT_NEAR ( texel[2], axis[face][2] * 0.5f + 0.5f, 0.05f ) << "face " << face << " channel z";
        }
    }
}
