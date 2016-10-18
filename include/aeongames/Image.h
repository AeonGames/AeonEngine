/*
Copyright 2016 Rodrigo Jose Hernandez Cordoba

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
#include <memory>
#include <functional>
#include <string>
namespace AeonGames
{
    class Image
    {
    public:
        enum class ImageFormat : std::uint32_t
        {
            Unknown,
            RGB,
            RGBA
        };
        enum class ImageType : std::uint32_t
        {
            Unknown,
            UNSIGNED_BYTE,
            UNSIGNED_SHORT
        };
        virtual uint32_t Width() const = 0;
        virtual uint32_t Height() const = 0;
        virtual ImageFormat Format() const = 0;
        virtual ImageType Type() const = 0;
        virtual const uint8_t* Data() const = 0;
        virtual ~Image() = default;
    };
    /** Factory Function */
    DLL std::unique_ptr<Image> GetImage ( const std::string& aIdentifier, const std::string& aFilename );
    /** Registers an image loader for a filename extension.*/
    DLL bool RegisterImageLoader ( const std::string& aIdentifier, std::function<std::unique_ptr<Image> ( const std::string& ) > aLoader );
    /** Unregisters an image loader for a filename extension.*/
    DLL bool UnregisterImageLoader ( const std::string& aIdentifier );
}
#endif
