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
#ifndef AEONGAMES_RADIANCEIMAGE_H
#define AEONGAMES_RADIANCEIMAGE_H
#include <cstddef>
#include <cstdint>
#include <vector>
namespace AeonGames
{
    class Texture;
    /** @brief Decodes a Radiance RGBE (.hdr) buffer into linear float RGB pixels.
     *
     * Texture-free core so it can be unit tested without the engine's DLL.
     * @param aBuffer Pointer to the encoded .hdr data.
     * @param aBufferSize Size of the buffer in bytes.
     * @param aWidth Receives the image width in pixels.
     * @param aHeight Receives the image height in pixels.
     * @param aPixels Receives width*height*3 linear float RGB components.
     * @return True on success, false if the buffer is not a supported .hdr image.
     */
    bool DecodeRadianceRGBE ( const void* aBuffer, size_t aBufferSize,
                              uint32_t& aWidth, uint32_t& aHeight, std::vector<float>& aPixels );
    /** @brief Decodes a Radiance RGBE (.hdr) image into a float RGB texture.
     * @param aTexture Target texture; filled as Format::RGB, Type::FLOAT.
     * @param aBufferSize Size of the encoded buffer in bytes.
     * @param aBuffer Pointer to the encoded .hdr data.
     * @return True on success, false if the buffer is not a supported .hdr image.
     */
    bool DecodeHDR ( Texture& aTexture, size_t aBufferSize, const void* aBuffer );
}
#endif
