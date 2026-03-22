/*
Copyright (C) 2016-2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Platform.hpp"
#include "aeongames/Resource.hpp"

namespace AeonGames
{
    /** @brief Represents a 2D texture image resource.
     *
     * Provides functionality for loading, resizing, and accessing pixel data
     * in various formats and types. Supports pluggable image decoders.
     */
    class Texture : public Resource
    {
    public:
        /** @brief Pixel channel layout format.
         *
         * Specifies the ordering and number of color channels per pixel.
         */
        enum class Format : uint32_t
        {
            Unknown = 0, ///< Unspecified or invalid format.
            RGB,         ///< 3-channel red, green, blue.
            RGBA,        ///< 4-channel red, green, blue, alpha.
            BGRA         ///< 4-channel blue, green, red, alpha.
        };
        /** @brief Pixel component data type.
         *
         * Specifies the data type used for each color channel value.
         */
        enum class Type : uint32_t
        {
            Unknown = 0,                ///< Unspecified or invalid type.
            UNSIGNED_BYTE,              ///< 8-bit unsigned integer per channel.
            UNSIGNED_SHORT,             ///< 16-bit unsigned integer per channel.
            UNSIGNED_INT_8_8_8_8_REV    ///< Packed 32-bit unsigned integer with reversed byte order.
        };
        /** @brief Destructor. */
        DLL ~Texture();
        /** @brief Resizes the texture and optionally replaces pixel data.
         * @param aWidth New width in pixels.
         * @param aHeight New height in pixels.
         * @param aPixels Optional pointer to new pixel data.
         * @param aFormat Pixel format of the new data. Keeps current format if Unknown.
         * @param aType Pixel type of the new data. Keeps current type if Unknown.
         */
        DLL void Resize ( uint32_t aWidth, uint32_t aHeight, const uint8_t* aPixels = nullptr, Format aFormat = Format::Unknown, Type aType = Type::Unknown );
        /** @brief Writes pixel data into a sub-region of the texture.
         * @param aXOffset Horizontal offset in pixels.
         * @param aYOffset Vertical offset in pixels.
         * @param aWidth Width of the region to write.
         * @param aHeight Height of the region to write.
         * @param aFormat Pixel format of the source data.
         * @param aType Pixel type of the source data.
         * @param aPixels Pointer to the source pixel data.
         */
        DLL void WritePixels ( int32_t aXOffset, int32_t aYOffset, uint32_t aWidth, uint32_t aHeight, Format aFormat, Type aType, const uint8_t* aPixels );
        /** @brief Loads texture data from a raw memory buffer.
         * @param aBuffer Pointer to the encoded image data.
         * @param aBufferSize Size of the buffer in bytes.
         */
        DLL void LoadFromMemory ( const void* aBuffer, size_t aBufferSize ) final;
        /** @brief Releases all texture data and resets the texture to an empty state. */
        DLL void Unload() final;
        /** @brief Returns the texture width in pixels. */
        DLL uint32_t GetWidth() const;
        /** @brief Returns the texture height in pixels. */
        DLL uint32_t GetHeight() const;
        /** @brief Returns the pixel format. */
        DLL Format GetFormat() const;
        /** @brief Returns the pixel component type. */
        DLL Type GetType() const;
        /** @brief Returns a reference to the raw pixel data. */
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
    /** @brief Registers an image decoder for a specific file magic identifier.
     * @param aMagick Magic bytes or identifier string for the image format.
     * @param aDecoder Callback function that decodes image data into a Texture.
     * @return True if the decoder was registered successfully.
     */
    DLL bool RegisterImageDecoder ( const std::string& aMagick, const std::function < bool ( Texture&, size_t, const void* ) > & aDecoder );
    /** @brief Unregisters a previously registered image decoder.
     * @param aMagick Magic bytes or identifier string of the decoder to remove.
     * @return True if the decoder was found and removed.
     */
    DLL bool UnregisterImageDecoder ( const std::string& aMagick );
    /** @brief Decodes image data from a memory buffer into a Texture.
     * @param aTexture Target texture to receive the decoded image.
     * @param aBuffer Pointer to the encoded image data.
     * @param aBufferSize Size of the buffer in bytes.
     * @return True if decoding succeeded.
     */
    DLL bool DecodeImage ( Texture& aTexture, const void* aBuffer, size_t aBufferSize );
    /** @brief Decodes an image file into a Texture.
     * @param aTexture Target texture to receive the decoded image.
     * @param aFileName Path to the image file.
     * @return True if decoding succeeded.
     */
    DLL bool DecodeImage ( Texture& aTexture, const std::string& aFileName );
    /*@}*/
    /** @brief Computes the size in bytes of a single pixel for the given format and type.
     * @param aFormat Pixel format.
     * @param aType Pixel component type.
     * @return Size in bytes, or 0 if either parameter is Unknown.
     */
    static constexpr size_t GetPixelSize ( Texture::Format aFormat, Texture::Type aType )
    {
        return ( aFormat == Texture::Format::Unknown || aType == Texture::Type::Unknown ) ? 0 :
               ( ( aFormat == Texture::Format::RGB ) ? 3 : 4 ) * ( ( aType == Texture::Type::UNSIGNED_BYTE ) ? 1 : 2 );
    }
}
#endif
