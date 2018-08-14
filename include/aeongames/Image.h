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
#ifndef AEONGAMES_IMAGE_H
#define AEONGAMES_IMAGE_H
#include "Platform.h"
#include <cstdint>
#include "aeongames/Memory.h"
#include "aeongames/Platform.h"
#include <functional>
#include <string>
#include <vector>
#include <mutex>
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
        class IRenderImage
        {
        public:
            DLL virtual ~IRenderImage() = 0;
            /** Trigger a full image update from the Image data. */
            virtual void Update () = 0;
        };
        DLL Image();
        DLL Image ( uint32_t aId );
        DLL Image ( uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels = nullptr );
        DLL ~Image();
        DLL void Initialize ( uint32_t aWidth, uint32_t aHeight, ImageFormat aFormat, ImageType aType, const uint8_t* aPixels = nullptr );
        DLL void Finalize();
        DLL uint32_t Width() const;
        DLL uint32_t Height() const;
        DLL ImageFormat Format() const;
        DLL ImageType Type() const;
        DLL const uint8_t* Pixels() const;
        DLL const size_t PixelsSize() const;
        DLL void* Map();
        DLL void Unmap();
        DLL void SetRenderImage ( std::unique_ptr<IRenderImage> aRenderImage ) const;
        DLL const IRenderImage* GetRenderImage() const;
        DLL static const std::shared_ptr<Image> GetImage ( uint32_t aId );
    private:
        mutable std::unique_ptr<IRenderImage>mRenderImage{};
        bool mMapped{false};
        uint32_t mWidth{};
        uint32_t mHeight{};
        ImageFormat mFormat{ImageFormat::Unknown};
        ImageType mType{ImageType::Unknown};
        std::vector<uint8_t> mPixels{};
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
}
#endif
