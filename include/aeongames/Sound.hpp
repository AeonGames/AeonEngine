/*
Copyright (C) 2017-2019,2025 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_SOUND_H
#define AEONGAMES_SOUND_H
#include "aeongames/Platform.hpp"
#include <cstdint>
#include <memory>
#include <functional>
#include <string>
namespace AeonGames
{
    class Sound
    {
    public:
        virtual ~Sound() = default;
    };
    /**@name Decoder Functions */
    /*@{*/
    /***/
    DLL bool RegisterSoundDecoder ( const std::string& aMagick, const std::function < bool ( Sound&, size_t, const void* ) > & aDecoder );
    /***/
    DLL bool UnregisterSoundDecoder ( const std::string& aMagick );
    /***/
    DLL bool DecodeSound ( Sound& aSound, size_t aBufferSize, const void* aBuffer );
    /***/
    DLL bool DecodeSound ( Sound& aSound, const std::string& aFileName );
    /*@}*/
}
#endif
