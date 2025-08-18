/*
Copyright (C) 2017,2018,2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Sound.hpp"
#include "aeongames/Utilities.hpp"
#include "aeongames/ResourceCache.hpp"
#include "Decoder.h"
#include <unordered_map>
#include <utility>

namespace AeonGames
{
    bool RegisterSoundDecoder ( const std::string& aMagick, const std::function < bool ( Sound&, size_t, const void* ) > & aDecoder )
    {
        return Decoder<Sound>::RegisterDecoder ( aMagick, aDecoder );
    }

    bool UnregisterSoundDecoder ( const std::string& aMagick )
    {
        return Decoder<Sound>::UnregisterDecoder ( aMagick );
    }

    bool DecodeSound ( Sound& aSound, const void* aBuffer, size_t aBufferSize )
    {
        return Decoder<Sound>::Decode ( aSound, aBuffer, aBufferSize );
    }

    bool DecodeSound ( Sound& aSound, const std::string& aFileName )
    {
        return Decoder<Sound>::Decode ( aSound, aFileName );
    }
}
