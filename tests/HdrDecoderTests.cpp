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
#include <string>
#include <vector>
#include "RadianceImage.h"
#include "gtest/gtest.h"

namespace AeonGames
{
    // Decodes a tiny synthetic Radiance image that exercises both scanline
    // encodings: row 0 uses the new-style adaptive RLE, row 1 is flat RGBE.
    // Verifies header parsing, both decode paths and the RGBE -> float mapping
    // (value * 2^(exponent - 136)).
    TEST ( RadianceHDR, DecodesRleAndFlatScanlines )
    {
        std::vector<uint8_t> buffer;
        auto append = [&] ( const std::string & text )
        {
            buffer.insert ( buffer.end(), text.begin(), text.end() );
        };
        append ( "#?RADIANCE\n" );
        append ( "FORMAT=32-bit_rle_rgbe\n" );
        append ( "\n" );
        append ( "-Y 2 +X 8\n" );

        // Row 0: adaptive RLE, 8 pixels all (128,128,128,128) -> 0.5 each.
        const uint8_t rle_header[4] = { 2, 2, 0, 8 };
        buffer.insert ( buffer.end(), rle_header, rle_header + 4 );
        for ( int channel = 0; channel < 4; ++channel )
        {
            buffer.push_back ( 128 + 8 ); // run length 8
            buffer.push_back ( 128 );     // repeated value
        }
        // Row 1: flat RGBE, 8 pixels all (255,128,64,127).
        for ( int x = 0; x < 8; ++x )
        {
            buffer.push_back ( 255 );
            buffer.push_back ( 128 );
            buffer.push_back ( 64 );
            buffer.push_back ( 127 );
        }

        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<float> pixels;
        ASSERT_TRUE ( DecodeRadianceRGBE ( buffer.data(), buffer.size(), width, height, pixels ) );
        EXPECT_EQ ( width, 8u );
        EXPECT_EQ ( height, 2u );
        ASSERT_EQ ( pixels.size(), 8u * 2u * 3u );

        // Row 0, pixel 0: 128 * 2^(128 - 136) = 0.5.
        EXPECT_NEAR ( pixels[0], 0.5f, 1e-5f );
        EXPECT_NEAR ( pixels[1], 0.5f, 1e-5f );
        EXPECT_NEAR ( pixels[2], 0.5f, 1e-5f );
        // Row 1, pixel 0 (offset 8*3): (255,128,64) * 2^(127 - 136) = value / 512.
        EXPECT_NEAR ( pixels[24], 255.0f / 512.0f, 1e-4f );
        EXPECT_NEAR ( pixels[25], 128.0f / 512.0f, 1e-4f );
        EXPECT_NEAR ( pixels[26], 64.0f / 512.0f, 1e-4f );
    }
}
