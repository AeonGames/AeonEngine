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
#import <Metal/Metal.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <vector>
#include "aeongames/Texture.hpp"
#include "MetalTexture.h"

namespace AeonGames
{
    namespace
    {
        uint32_t MipCount ( uint32_t aWidth, uint32_t aHeight )
        {
            return 1u + static_cast<uint32_t> ( std::floor ( std::log2 ( std::max ( aWidth, aHeight ) ) ) );
        }

        std::vector<uint8_t> ConvertToRgba8 ( const Texture& aTexture )
        {
            const size_t pixel_count = static_cast<size_t> ( aTexture.GetWidth() ) * aTexture.GetHeight();
            std::vector<uint8_t> rgba ( pixel_count * 4, 255 );
            const size_t channels = aTexture.GetFormat() == Texture::Format::RGB ? 3 : 4;
            if ( aTexture.GetType() == Texture::Type::UNSIGNED_BYTE ||
                 aTexture.GetType() == Texture::Type::UNSIGNED_INT_8_8_8_8_REV )
            {
                const uint8_t* source = aTexture.GetPixels().data();
                for ( size_t pixel = 0; pixel < pixel_count; ++pixel )
                {
                    if ( aTexture.GetFormat() == Texture::Format::BGRA )
                    {
                        rgba[pixel * 4 + 0] = source[pixel * channels + 2];
                        rgba[pixel * 4 + 1] = source[pixel * channels + 1];
                        rgba[pixel * 4 + 2] = source[pixel * channels + 0];
                    }
                    else
                    {
                        std::memcpy ( rgba.data() + pixel * 4, source + pixel * channels,
                                      std::min<size_t> ( channels, 3 ) );
                    }
                    if ( channels == 4 )
                    {
                        rgba[pixel * 4 + 3] = source[pixel * channels + 3];
                    }
                }
                return rgba;
            }
            if ( aTexture.GetType() == Texture::Type::UNSIGNED_SHORT )
            {
                const auto* source = reinterpret_cast<const uint16_t*> ( aTexture.GetPixels().data() );
                for ( size_t pixel = 0; pixel < pixel_count; ++pixel )
                {
                    for ( size_t channel = 0; channel < channels; ++channel )
                    {
                        const size_t destination = aTexture.GetFormat() == Texture::Format::BGRA && channel < 3
                                                   ? 2 - channel : channel;
                        rgba[pixel * 4 + destination] = static_cast<uint8_t> ( source[pixel * channels + channel] >> 8 );
                    }
                }
                return rgba;
            }
            throw std::runtime_error ( "MetalTexture cannot convert this texture type to RGBA8" );
        }

        std::vector<float> ConvertToRgba32 ( const Texture& aTexture )
        {
            const size_t pixel_count = static_cast<size_t> ( aTexture.GetWidth() ) * aTexture.GetHeight();
            const size_t channels = aTexture.GetFormat() == Texture::Format::RGB ? 3 : 4;
            const auto* source = reinterpret_cast<const float*> ( aTexture.GetPixels().data() );
            std::vector<float> rgba ( pixel_count * 4, 1.0f );
            for ( size_t pixel = 0; pixel < pixel_count; ++pixel )
            {
                if ( aTexture.GetFormat() == Texture::Format::BGRA )
                {
                    rgba[pixel * 4 + 0] = source[pixel * channels + 2];
                    rgba[pixel * 4 + 1] = source[pixel * channels + 1];
                    rgba[pixel * 4 + 2] = source[pixel * channels + 0];
                }
                else
                {
                    std::copy_n ( source + pixel * channels, std::min<size_t> ( channels, 3 ), rgba.data() + pixel * 4 );
                }
                if ( channels == 4 )
                {
                    rgba[pixel * 4 + 3] = source[pixel * channels + 3];
                }
            }
            return rgba;
        }
    }

    class MetalTexture::Impl
    {
    public:
        Impl ( id<MTLDevice> aDevice, id<MTLCommandQueue> aCommandQueue, const Texture& aTexture ) :
            mTextureResource{&aTexture}
        {
            if ( aTexture.GetWidth() == 0 || aTexture.GetHeight() == 0 || aTexture.GetPixels().empty() )
            {
                throw std::runtime_error ( "MetalTexture cannot upload an empty texture" );
            }
            const bool floating_point = aTexture.GetType() == Texture::Type::FLOAT;
            const MTLPixelFormat format = floating_point ? MTLPixelFormatRGBA32Float : MTLPixelFormatRGBA8Unorm;
            MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                width:aTexture.GetWidth()
                                                height:aTexture.GetHeight()
                                                mipmapped:YES];
            descriptor.mipmapLevelCount = MipCount ( aTexture.GetWidth(), aTexture.GetHeight() );
            descriptor.storageMode = MTLStorageModeShared;
            descriptor.usage = MTLTextureUsageShaderRead;
            mTexture = [aDevice newTextureWithDescriptor:descriptor];
            if ( mTexture == nil )
            {
                throw std::runtime_error ( "Metal texture allocation failed" );
            }
            const MTLRegion region = MTLRegionMake2D ( 0, 0, aTexture.GetWidth(), aTexture.GetHeight() );
            if ( floating_point )
            {
                const std::vector<float> pixels = ConvertToRgba32 ( aTexture );
                [mTexture replaceRegion:region mipmapLevel:0 withBytes:pixels.data()
                 bytesPerRow:static_cast<NSUInteger> ( aTexture.GetWidth() ) * sizeof ( float ) * 4];
            }
            else
            {
                const std::vector<uint8_t> pixels = ConvertToRgba8 ( aTexture );
                [mTexture replaceRegion:region mipmapLevel:0 withBytes:pixels.data()
                 bytesPerRow:static_cast<NSUInteger> ( aTexture.GetWidth() ) * 4];
            }
            if ( descriptor.mipmapLevelCount > 1 )
            {
                id<MTLCommandBuffer> command_buffer = [aCommandQueue commandBuffer];
                id<MTLBlitCommandEncoder> blit = [command_buffer blitCommandEncoder];
                [blit generateMipmapsForTexture:mTexture];
                [blit endEncoding];
                [command_buffer commit];
                [command_buffer waitUntilCompleted];
                if ( command_buffer.status == MTLCommandBufferStatusError )
                {
                    throw std::runtime_error ( std::string {"Metal mip generation failed: "} +
                                               ( command_buffer.error.localizedDescription.UTF8String ? : "unknown error" ) );
                }
            }
        }

        const Texture* mTextureResource{nullptr};
        id<MTLTexture> mTexture{nil};
    };

    MetalTexture::MetalTexture ( void* aDevice, void* aCommandQueue, const Texture& aTexture ) :
        mImpl{std::make_unique<Impl> ( ( __bridge id<MTLDevice> ) aDevice,
                                       ( __bridge id<MTLCommandQueue> ) aCommandQueue, aTexture ) }
    {
    }

    MetalTexture::~MetalTexture() = default;
    MetalTexture::MetalTexture ( MetalTexture&& ) noexcept = default;
    MetalTexture& MetalTexture::operator= ( MetalTexture&& ) noexcept = default;

    const Texture& MetalTexture::GetTexture() const
    {
        return *mImpl->mTextureResource;
    }
    void* MetalTexture::GetNativeTexture() const
    {
        return ( __bridge void* ) mImpl->mTexture;
    }
}