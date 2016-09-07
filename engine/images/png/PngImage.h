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
#ifndef AEONGAMES_PNGIMAGE_H
#define AEONGAMES_PNGIMAGE_H
#include <string>
#include <vector>
#include "aeongames/Image.h"

namespace AeonGames
{
    class PngImage : public Image
    {
    public:
        PngImage ( const std::string & aFileName );
        ~PngImage() final;
        uint32_t Width() const final;
        uint32_t Height() const final;
        ImageFormat Format() const final;
        ImageType Type() const final;
        const uint8_t* Data() const final;
    private:
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        ImageFormat mFormat = ImageFormat::Unknown;
        ImageType mType = ImageType::Unknown;
        std::vector<uint8_t> mData;
    };
}
#endif
