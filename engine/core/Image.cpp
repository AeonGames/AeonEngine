/*
Copyright (C) 2016-2018 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"
#include "aeongames/ResourceCache.h"
#include "Decoder.h"
#include <unordered_map>
#include <utility>

namespace AeonGames
{
    enum class ImageFormat : uint32_t
    {
        Unknown,
        RGB,
        RGBA
    };
    enum class ImageType : uint32_t
    {
        Unknown,
        UNSIGNED_BYTE,
        UNSIGNED_SHORT
    };

    static constexpr size_t GetPixelSize ( Image::ImageFormat aFormat, Image::ImageType aType )
    {
        return ( aFormat == Image::ImageFormat::Unknown || aType == Image::ImageType::Unknown ) ? 0 :
               ( ( aFormat == Image::ImageFormat::RGB ) ? 3 : 4 ) * ( ( aType == Image::ImageType::UNSIGNED_BYTE ) ? 1 : 2 );
    }

    Image::Image() = default;
    Image::Image ( uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels ) :
        mWidth{aWidth}, mHeight{aHeight}, mFormat{aFormat}, mType{aType}, mPixels ( aWidth * aHeight * GetPixelSize ( aFormat, aType ), 0 )
    {
        if ( aPixels && mPixels.size() )
        {
            memcpy ( mPixels.data(), aPixels, mPixels.size() );
        }
    }

    DLL Image::IRenderImage::~IRenderImage() = default;

    void Image::Initialize ( uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels )
    {
        if ( mMapped )
        {
            throw std::runtime_error ( "Cannot initialize Image object, memory is mapped." );
        }
        mWidth = aWidth;
        mHeight = aHeight;
        mFormat = aFormat;
        mType = aType;
        mPixels.resize ( aWidth * aHeight * GetPixelSize ( aFormat, aType ), 0 );
        if ( aPixels && mPixels.size() )
        {
            memcpy ( mPixels.data(), aPixels, mPixels.size() );
        }
    }
    void Image::Finalize()
    {
        if ( mMapped )
        {
            throw std::runtime_error ( "Cannot finalize Image object, memory is mapped." );
        }
        mWidth = 0;
        mHeight = 0;
        mFormat = {};
        mType = {};
        mPixels.clear();
    }
    Image::~Image() = default;
    uint32_t Image::Width() const
    {
        return mWidth;
    }
    uint32_t Image::Height() const
    {
        return mHeight;
    }
    Image::ImageFormat Image::Format() const
    {
        return mFormat;
    }
    Image::ImageType Image::Type() const
    {
        return mType;
    }
    const uint8_t* Image::Pixels() const
    {
        return mPixels.data();
    }
    const size_t Image::PixelsSize() const
    {
        return mPixels.size();
    }

    void* Image::Map()
    {
        if ( mMapped )
        {
            throw std::runtime_error ( "Image memory has already been mapped." );
        }
        mMapped = true;
        return mPixels.data();
    }
    void Image::Unmap()
    {
        if ( !mMapped )
        {
            throw std::runtime_error ( "Image memory is not mapped." );
        }
        mMapped = false;
    }

    void Image::SetRenderImage ( std::unique_ptr<IRenderImage> aRenderImage ) const
    {
        mRenderImage = std::move ( aRenderImage );
    }

    const Image::IRenderImage* Image::GetRenderImage() const
    {
        return mRenderImage.get();
    }

    bool RegisterImageDecoder ( const std::string& aMagick, const std::function < bool ( Image&, size_t, const void* ) > & aDecoder )
    {
        return Decoder<Image>::RegisterDecoder ( aMagick, aDecoder );
    }

    bool UnregisterImageDecoder ( const std::string& aMagick )
    {
        return Decoder<Image>::UnregisterDecoder ( aMagick );
    }

    bool DecodeImage ( Image& aImage, size_t aBufferSize, const void* aBuffer )
    {
        return Decoder<Image>::Decode ( aImage, aBufferSize, aBuffer );
    }

    bool DecodeImage ( Image& aImage, const std::string& aFileName )
    {
        return Decoder<Image>::Decode ( aImage, aFileName );
    }
}
