/*
Copyright (C) 2016-2021 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_TEXTURE_H
#define AEONGAMES_TEXTURE_H
#include <cstdint>
#include <string>
#include <functional>
#include <vector>
#include "aeongames/Platform.h"
#include "aeongames/Resource.h"

namespace AeonGames
{
    class Texture : public Resource
    {
    public:
        enum class Format : uint32_t
        {
            Unknown = 0,
            RGB,
            RGBA,
            BGRA
        };
        enum class Type : uint32_t
        {
            Unknown = 0,
            UNSIGNED_BYTE,
            UNSIGNED_SHORT,
            UNSIGNED_INT_8_8_8_8_REV
        };
        DLL ~Texture();
        DLL void Resize ( uint32_t aWidth, uint32_t aHeight, const uint8_t* aPixels = nullptr, Format aFormat = Format::Unknown, Type aType = Type::Unknown );
        DLL void WritePixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Format aFormat, Type aType, const uint8_t* aPixels );
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        DLL void Unload() final;
        DLL uint32_t GetWidth() const;
        DLL uint32_t GetHeight() const;
        DLL Format GetFormat() const;
        DLL Type GetType() const;
        DLL const std::vector<uint8_t>& GetPixels() const;
    private:
        std::vector<uint8_t> mPixels{};
        uint32_t mWidth{};
        uint32_t mHeight{};
        Format mFormat{};
        Type mType{};
    };
    /**@name Decoder Functions */
    /*@{*/
    /***/
    DLL bool RegisterImageDecoder ( const std::string& aMagick, const std::function < bool ( Texture&, size_t, const void* ) > & aDecoder );
    /***/
    DLL bool UnregisterImageDecoder ( const std::string& aMagick );
    /***/
    DLL bool DecodeImage ( Texture& aTexture, const void* aBuffer, size_t aBufferSize );
    /***/
    DLL bool DecodeImage ( Texture& aTexture, const std::string& aFileName );
    /*@}*/
    static constexpr size_t GetPixelSize ( Texture::Format aFormat, Texture::Type aType )
    {
        return ( aFormat == Texture::Format::Unknown || aType == Texture::Type::Unknown ) ? 0 :
               ( ( aFormat == Texture::Format::RGB ) ? 3 : 4 ) * ( ( aType == Texture::Type::UNSIGNED_BYTE ) ? 1 : 2 );
    }
}
#endif
