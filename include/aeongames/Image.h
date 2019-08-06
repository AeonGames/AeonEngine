/*
Copyright (C) 2016-2019 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_IMAGE_H
#define AEONGAMES_IMAGE_H
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include "aeongames/Platform.h"
#include "Platform.h"

namespace AeonGames
{
    class Image
    {
    public:
        enum class ImageFormat : uint32_t
        {
            Unknown = 0,
            RGB,
            RGBA
        };
        enum class ImageType : uint32_t
        {
            Unknown = 0,
            UNSIGNED_BYTE,
            UNSIGNED_SHORT
        };
        DLL virtual ~Image() = 0;
        virtual void Load ( const std::string& aPath ) = 0;
        virtual void Load ( uint32_t aId ) = 0;
        virtual void Initialize ( uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels = nullptr ) = 0;
        virtual void Resize ( uint32_t aWidth, uint32_t aHeight, const uint8_t* aPixels = nullptr ) = 0;
        virtual void WritePixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels ) = 0;
        virtual void Finalize() = 0;
        virtual uint32_t Width() const = 0;
        virtual uint32_t Height() const = 0;
        virtual ImageFormat Format() const = 0;
        virtual ImageType Type() const = 0;
    };
    /**@name Decoder Functions */
    /*@{*/
    /***/
    DLL bool RegisterImageDecoder ( const std::string& aMagick, const std::function < bool ( Image&, size_t, const void* ) > & aDecoder );
    /***/
    DLL bool UnregisterImageDecoder ( const std::string& aMagick );
    /***/
    DLL bool DecodeImage ( Image& aImage, const void* aBuffer, size_t aBufferSize );
    /***/
    DLL bool DecodeImage ( Image& aImage, const std::string& aFileName );
    /*@}*/
    static constexpr size_t GetPixelSize ( Image::ImageFormat aFormat, Image::ImageType aType )
    {
        return ( aFormat == Image::ImageFormat::Unknown || aType == Image::ImageType::Unknown ) ? 0 :
               ( ( aFormat == Image::ImageFormat::RGB ) ? 3 : 4 ) * ( ( aType == Image::ImageType::UNSIGNED_BYTE ) ? 1 : 2 );
    }
}
#endif
