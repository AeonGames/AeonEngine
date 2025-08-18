/*
Copyright (C) 2016-2018,2020,2021,2025 Rodrigo Jose Hernandez Cordoba

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
}
