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
/** \file Implements a decoder for Radiance RGBE (.hdr) high-dynamic-range images. */
#include "RadianceImage.h"
#include "aeongames/Texture.hpp"
#include "aeongames/LogLevel.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace AeonGames
{
    namespace
    {
        // Convert a Radiance RGBE quadruplet to linear float RGB. The shared
        // exponent scales the three mantissas; a zero exponent encodes black.
        inline void RGBEToFloat ( const uint8_t* aRGBE, float* aRGB )
        {
            if ( aRGBE[3] != 0 )
            {
                const float scale = std::ldexp ( 1.0f, static_cast<int> ( aRGBE[3] ) - ( 128 + 8 ) );
                aRGB[0] = static_cast<float> ( aRGBE[0] ) * scale;
                aRGB[1] = static_cast<float> ( aRGBE[1] ) * scale;
                aRGB[2] = static_cast<float> ( aRGBE[2] ) * scale;
            }
            else
            {
                aRGB[0] = aRGB[1] = aRGB[2] = 0.0f;
            }
        }
    }

    bool DecodeRadianceRGBE ( const void* aBuffer, size_t aBufferSize,
                              uint32_t& aWidth, uint32_t& aHeight, std::vector<float>& aPixels )
    {
        const uint8_t* cursor = static_cast<const uint8_t*> ( aBuffer );
        const uint8_t* end = cursor + aBufferSize;
        // Radiance files begin with "#?" followed by a program name
        // (RADIANCE / RGBE). Anything else is not our format.
        if ( aBufferSize < 2 || std::memcmp ( cursor, "#?", 2 ) != 0 )
        {
            return false;
        }
        try
        {
            auto read_line = [&] () -> std::string
            {
                std::string line;
                while ( cursor < end && *cursor != '\n' )
                {
                    line.push_back ( static_cast<char> ( *cursor++ ) );
                }
                if ( cursor < end )
                {
                    ++cursor; // consume the newline
                }
                return line;
            };

            // First line is the magic; the header ends at the first blank line.
            read_line();
            bool format_ok = false;
            for ( std::string line = read_line(); ; line = read_line() )
            {
                if ( line.empty() )
                {
                    break;
                }
                if ( line.rfind ( "FORMAT=", 0 ) == 0 )
                {
                    format_ok = line.find ( "32-bit_rle_rgbe" ) != std::string::npos;
                }
                if ( cursor >= end )
                {
                    throw std::runtime_error ( "Unterminated Radiance header." );
                }
            }
            if ( !format_ok )
            {
                throw std::runtime_error ( "Unsupported Radiance FORMAT (only 32-bit_rle_rgbe)." );
            }

            // Resolution line. Only the standard "-Y <height> +X <width>"
            // orientation (rows top-to-bottom) is supported.
            std::istringstream resolution ( read_line() );
            std::string y_axis, x_axis;
            int height = 0;
            int width = 0;
            resolution >> y_axis >> height >> x_axis >> width;
            if ( y_axis != "-Y" || x_axis != "+X" || width <= 0 || height <= 0 )
            {
                throw std::runtime_error ( "Unsupported Radiance resolution/orientation." );
            }

            std::vector<float> pixels ( static_cast<size_t> ( width ) * height * 3 );
            std::vector<uint8_t> scanline ( static_cast<size_t> ( width ) * 4 );

            for ( int y = 0; y < height; ++y )
            {
                if ( end - cursor < 4 )
                {
                    throw std::runtime_error ( "Truncated Radiance scanline header." );
                }
                const bool new_rle = cursor[0] == 2 && cursor[1] == 2 &&
                                     ( ( cursor[2] << 8 ) | cursor[3] ) == width && width >= 8 && width <= 0x7fff;
                if ( new_rle )
                {
                    cursor += 4;
                    // Adaptive RLE: each of the four channels is run-length
                    // encoded independently across the scanline.
                    for ( int channel = 0; channel < 4; ++channel )
                    {
                        int x = 0;
                        while ( x < width )
                        {
                            if ( cursor >= end )
                            {
                                throw std::runtime_error ( "Truncated Radiance RLE data." );
                            }
                            uint8_t count = *cursor++;
                            if ( count > 128 )
                            {
                                // A run of (count - 128) copies of the next byte.
                                const int run = count - 128;
                                if ( cursor >= end )
                                {
                                    throw std::runtime_error ( "Truncated Radiance RLE run." );
                                }
                                const uint8_t value = *cursor++;
                                for ( int i = 0; i < run && x < width; ++i, ++x )
                                {
                                    scanline[static_cast<size_t> ( x ) * 4 + channel] = value;
                                }
                            }
                            else
                            {
                                // A literal span of 'count' bytes.
                                for ( int i = 0; i < count && x < width; ++i, ++x )
                                {
                                    if ( cursor >= end )
                                    {
                                        throw std::runtime_error ( "Truncated Radiance literal span." );
                                    }
                                    scanline[static_cast<size_t> ( x ) * 4 + channel] = *cursor++;
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Flat RGBE, optionally with the legacy (1,1,1,count) run
                    // encoding that repeats the previous pixel.
                    int x = 0;
                    int shift = 0;
                    while ( x < width )
                    {
                        if ( end - cursor < 4 )
                        {
                            throw std::runtime_error ( "Truncated Radiance flat scanline." );
                        }
                        if ( cursor[0] == 1 && cursor[1] == 1 && cursor[2] == 1 && x > 0 )
                        {
                            const int run = cursor[3] << shift;
                            for ( int i = 0; i < run && x < width; ++i, ++x )
                            {
                                std::memcpy ( &scanline[static_cast<size_t> ( x ) * 4],
                                              &scanline[static_cast<size_t> ( x - 1 ) * 4], 4 );
                            }
                            shift += 8;
                        }
                        else
                        {
                            std::memcpy ( &scanline[static_cast<size_t> ( x ) * 4], cursor, 4 );
                            ++x;
                            shift = 0;
                        }
                        cursor += 4;
                    }
                }

                float* row = pixels.data() + static_cast<size_t> ( y ) * width * 3;
                for ( int x = 0; x < width; ++x )
                {
                    RGBEToFloat ( &scanline[static_cast<size_t> ( x ) * 4], row + static_cast<size_t> ( x ) * 3 );
                }
            }

            aWidth = static_cast<uint32_t> ( width );
            aHeight = static_cast<uint32_t> ( height );
            aPixels = std::move ( pixels );
        }
        catch ( const std::runtime_error& e )
        {
            std::cout << LogLevel::Error << "Radiance HDR decode failed: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool DecodeHDR ( Texture& aTexture, size_t aBufferSize, const void* aBuffer )
    {
        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<float> pixels;
        if ( !DecodeRadianceRGBE ( aBuffer, aBufferSize, width, height, pixels ) )
        {
            return false;
        }
        aTexture.Resize ( width, height, reinterpret_cast<const uint8_t*> ( pixels.data() ),
                          Texture::Format::RGB, Texture::Type::FLOAT );
        return true;
    }
}
