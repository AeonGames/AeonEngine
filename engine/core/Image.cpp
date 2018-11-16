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
#include <string>
#include "aeongames/Image.h"
#include "Decoder.h"

namespace AeonGames
{
    Image::~Image() = default;

    bool RegisterImageDecoder ( const std::string& aMagick, const std::function < bool ( Image&, size_t, const void* ) > & aDecoder )
    {
        return Decoder<Image>::RegisterDecoder ( aMagick, aDecoder );
    }

    bool UnregisterImageDecoder ( const std::string& aMagick )
    {
        return Decoder<Image>::UnregisterDecoder ( aMagick );
    }

    bool DecodeImage ( Image& aImage, const void* aBuffer, size_t aBufferSize )
    {
        return Decoder<Image>::Decode ( aImage, aBuffer, aBufferSize );
    }

    bool DecodeImage ( Image& aImage, const std::string& aFileName )
    {
        return Decoder<Image>::Decode ( aImage, aFileName );
    }
}
