/*
Copyright (C) 2016-2018,2020,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <string>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include "aeongames/Texture.hpp"
#include "Decoder.h"

namespace AeonGames
{
    Texture::~Texture() = default;

    bool RegisterImageDecoder ( const std::string& aMagick, const std::function < bool ( Texture&, size_t, const void* ) > & aDecoder )
    {
        return Decoder<Texture>::RegisterDecoder ( aMagick, aDecoder );
    }

    bool UnregisterImageDecoder ( const std::string& aMagick )
    {
        return Decoder<Texture>::UnregisterDecoder ( aMagick );
    }

    bool DecodeImage ( Texture& aTexture, const void* aBuffer, size_t aBufferSize )
    {
        return Decoder<Texture>::Decode ( aTexture, aBuffer, aBufferSize );
    }

    bool DecodeImage ( Texture& aTexture, const std::string& aFileName )
    {
        return Decoder<Texture>::Decode ( aTexture, aFileName );
    }

    void Texture::Resize ( uint32_t aWidth, uint32_t aHeight, const uint8_t* aPixels, Format aFormat, Type aType )
    {
        mWidth = aWidth;
        mHeight = aHeight;
        mFormat = aFormat;
        mType = aType;
        mPixels.resize ( aWidth * aHeight * GetPixelSize ( mFormat, mType ) );
        if ( aPixels != nullptr )
        {
            memcpy ( mPixels.data(), aPixels, mPixels.size() );
        }
        else
        {
            memset ( mPixels.data(), 0, mPixels.size() );
        }
    }

    void Texture::WritePixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Format aFormat, Type aType, const uint8_t* aPixels )
    {
        ///@todo Implement format-type translation.
        assert ( mType == aType && mFormat == aFormat );
        uint8_t* cursor = mPixels.data() + ( ( aYOffset * mWidth ) + aXOffset );
        size_t pixel_size = GetPixelSize ( mFormat, mType );
        for ( size_t i = 0; i < aHeight; ++i )
        {
            memcpy ( cursor, aPixels + ( i * pixel_size * aWidth ), pixel_size * aWidth );
        }
    }

    uint32_t Texture::GetWidth() const
    {
        return mWidth;
    }
    uint32_t Texture::GetHeight() const
    {
        return mHeight;
    }
    Texture::Format Texture::GetFormat() const
    {
        return mFormat;
    }
    Texture::Type Texture::GetType() const
    {
        return mType;
    }

    const std::vector<uint8_t>& Texture::GetPixels() const
    {
        return mPixels;
    }

    void Texture::LoadFromMemory ( const void* aBuffer, size_t aBufferSize )
    {
        DecodeImage ( *this, aBuffer, aBufferSize );
    }

    void Texture::Unload()
    {
        mPixels.clear();
        mType = Type::Unknown;
        mFormat = Format::Unknown;
        mWidth = 0;
        mHeight = 0;
    }

    namespace
    {
        constexpr float kPrefilterPi = 3.14159265358979323846f;

        // Van der Corput radical inverse for the Hammersley sequence.
        float RadicalInverseVdC ( uint32_t bits )
        {
            bits = ( bits << 16u ) | ( bits >> 16u );
            bits = ( ( bits & 0x55555555u ) << 1u ) | ( ( bits & 0xAAAAAAAAu ) >> 1u );
            bits = ( ( bits & 0x33333333u ) << 2u ) | ( ( bits & 0xCCCCCCCCu ) >> 2u );
            bits = ( ( bits & 0x0F0F0F0Fu ) << 4u ) | ( ( bits & 0xF0F0F0F0u ) >> 4u );
            bits = ( ( bits & 0x00FF00FFu ) << 8u ) | ( ( bits & 0xFF00FF00u ) >> 8u );
            return static_cast<float> ( bits ) * 2.3283064365386963e-10f; // / 2^32
        }

        // Bilinear sample of an equirectangular float-RGB environment at a
        // world-space direction (+Z up); longitude wraps, latitude clamps.
        void SampleEquirect ( const float* aPixels, int aWidth, int aHeight,
                              float aDx, float aDy, float aDz, float aOut[3] )
        {
            const float u = std::atan2 ( aDy, aDx ) / ( 2.0f * kPrefilterPi ) + 0.5f;
            const float v = std::acos ( std::clamp ( aDz, -1.0f, 1.0f ) ) / kPrefilterPi;
            const float fx = u * aWidth - 0.5f;
            const float fy = v * aHeight - 0.5f;
            const int x0 = static_cast<int> ( std::floor ( fx ) );
            const int y0 = static_cast<int> ( std::floor ( fy ) );
            const float tx = fx - x0;
            const float ty = fy - y0;
            auto fetch = [&] ( int x, int y, int c ) -> float
            {
                x = ( ( x % aWidth ) + aWidth ) % aWidth;
                y = std::clamp ( y, 0, aHeight - 1 );
                return aPixels[ ( static_cast<size_t> ( y ) * aWidth + x ) * 3 + c ];
            };
            for ( int c = 0; c < 3; ++c )
            {
                const float a = fetch ( x0, y0, c ) * ( 1.0f - tx ) + fetch ( x0 + 1, y0, c ) * tx;
                const float b = fetch ( x0, y0 + 1, c ) * ( 1.0f - tx ) + fetch ( x0 + 1, y0 + 1, c ) * tx;
                aOut[c] = a * ( 1.0f - ty ) + b * ty;
            }
        }
    }

    bool PrefilterEnvironmentEquirect ( const Texture& aEnvironment, uint32_t aBaseWidth,
                                        uint32_t aMipCount, std::vector<std::vector<float>>& aMips )
    {
        if ( aEnvironment.GetType() != Texture::Type::FLOAT ||
             aEnvironment.GetFormat() != Texture::Format::RGB ||
             aEnvironment.GetPixels().empty() || aBaseWidth == 0 || aMipCount == 0 )
        {
            return false;
        }
        const int env_width = static_cast<int> ( aEnvironment.GetWidth() );
        const int env_height = static_cast<int> ( aEnvironment.GetHeight() );
        const float* env = reinterpret_cast<const float*> ( aEnvironment.GetPixels().data() );
        constexpr uint32_t kSamples = 64u;
        aMips.assign ( aMipCount, {} );
        for ( uint32_t mip = 0; mip < aMipCount; ++mip )
        {
            const int mip_width = std::max<int> ( 1, static_cast<int> ( aBaseWidth >> mip ) );
            const int mip_height = std::max<int> ( 1, static_cast<int> ( ( aBaseWidth / 2 ) >> mip ) );
            const float roughness = ( aMipCount > 1 ) ? static_cast<float> ( mip ) / static_cast<float> ( aMipCount - 1 ) : 0.0f;
            const float alpha = roughness * roughness;
            std::vector<float>& out = aMips[mip];
            out.resize ( static_cast<size_t> ( mip_width ) * mip_height * 4 );
            for ( int y = 0; y < mip_height; ++y )
            {
                const float theta = ( ( y + 0.5f ) / mip_height ) * kPrefilterPi;
                const float sin_theta = std::sin ( theta );
                const float cos_theta = std::cos ( theta );
                for ( int x = 0; x < mip_width; ++x )
                {
                    const float phi = ( ( ( x + 0.5f ) / mip_width ) - 0.5f ) * 2.0f * kPrefilterPi;
                    // Prefilter assumption: reflection = normal = view = this texel's direction.
                    const float nx = sin_theta * std::cos ( phi );
                    const float ny = sin_theta * std::sin ( phi );
                    const float nz = cos_theta;
                    float color[3] = { 0.0f, 0.0f, 0.0f };
                    if ( mip == 0 || roughness <= 0.0f )
                    {
                        SampleEquirect ( env, env_width, env_height, nx, ny, nz, color );
                    }
                    else
                    {
                        // Orthonormal tangent frame around N for GGX sampling.
                        float ux = ( std::fabs ( nz ) < 0.999f ) ? 0.0f : 1.0f;
                        float uy = 0.0f;
                        float uz = ( std::fabs ( nz ) < 0.999f ) ? 1.0f : 0.0f;
                        float tx = uy * nz - uz * ny;
                        float ty = uz * nx - ux * nz;
                        float tz = ux * ny - uy * nx;
                        const float tl = std::sqrt ( tx * tx + ty * ty + tz * tz );
                        tx /= tl;
                        ty /= tl;
                        tz /= tl;
                        const float bx = ny * tz - nz * ty;
                        const float by = nz * tx - nx * tz;
                        const float bz = nx * ty - ny * tx;
                        float total_weight = 0.0f;
                        for ( uint32_t i = 0; i < kSamples; ++i )
                        {
                            const float xi_x = static_cast<float> ( i ) / static_cast<float> ( kSamples );
                            const float xi_y = RadicalInverseVdC ( i );
                            const float sample_phi = 2.0f * kPrefilterPi * xi_x;
                            const float cos_h = std::sqrt ( ( 1.0f - xi_y ) / ( 1.0f + ( alpha * alpha - 1.0f ) * xi_y ) );
                            const float sin_h = std::sqrt ( std::max ( 0.0f, 1.0f - cos_h * cos_h ) );
                            const float hxt = sin_h * std::cos ( sample_phi );
                            const float hyt = sin_h * std::sin ( sample_phi );
                            const float hzt = cos_h;
                            const float hx = tx * hxt + bx * hyt + nx * hzt;
                            const float hy = ty * hxt + by * hyt + ny * hzt;
                            const float hz = tz * hxt + bz * hyt + nz * hzt;
                            const float vdoth = nx * hx + ny * hy + nz * hz;
                            const float lx = 2.0f * vdoth * hx - nx;
                            const float ly = 2.0f * vdoth * hy - ny;
                            const float lz = 2.0f * vdoth * hz - nz;
                            const float ndotl = nx * lx + ny * ly + nz * lz;
                            if ( ndotl > 0.0f )
                            {
                                float s[3];
                                SampleEquirect ( env, env_width, env_height, lx, ly, lz, s );
                                color[0] += s[0] * ndotl;
                                color[1] += s[1] * ndotl;
                                color[2] += s[2] * ndotl;
                                total_weight += ndotl;
                            }
                        }
                        if ( total_weight > 0.0f )
                        {
                            color[0] /= total_weight;
                            color[1] /= total_weight;
                            color[2] /= total_weight;
                        }
                        else
                        {
                            SampleEquirect ( env, env_width, env_height, nx, ny, nz, color );
                        }
                    }
                    float* texel = out.data() + ( static_cast<size_t> ( y ) * mip_width + x ) * 4;
                    texel[0] = color[0];
                    texel[1] = color[1];
                    texel[2] = color[2];
                    texel[3] = 1.0f;
                }
            }
        }
        return true;
    }
}
