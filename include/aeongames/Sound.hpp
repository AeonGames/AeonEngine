/*
Copyright (C) 2017-2019,2025,2026 Rodrigo Jose Hernandez Cordoba

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
    /** @brief Abstract base class representing a decoded sound resource. */
    class Sound
    {
    public:
        /** @brief Virtual destructor. */
        virtual ~Sound() = default;
    };
    /**@name Decoder Functions */
    /*@{*/
    /** @brief Register a sound decoder for the given magic identifier.
     *  @param aMagick  Magic string that identifies the sound format.
     *  @param aDecoder Callable that decodes raw data into a Sound object.
     *  @return true if registration succeeded, false otherwise. */
    DLL bool RegisterSoundDecoder ( const std::string& aMagick, const std::function < bool ( Sound&, size_t, const void* ) > & aDecoder );
    /** @brief Unregister a previously registered sound decoder.
     *  @param aMagick Magic string of the decoder to remove.
     *  @return true if the decoder was found and removed, false otherwise. */
    DLL bool UnregisterSoundDecoder ( const std::string& aMagick );
    /** @brief Decode sound data from a memory buffer.
     *  @param aSound      Sound object to populate.
     *  @param aBufferSize Size of the data buffer in bytes.
     *  @param aBuffer     Pointer to the raw sound data.
     *  @return true on successful decode, false otherwise. */
    DLL bool DecodeSound ( Sound& aSound, size_t aBufferSize, const void* aBuffer );
    /** @brief Decode sound data from a file.
     *  @param aSound    Sound object to populate.
     *  @param aFileName Path to the sound file.
     *  @return true on successful decode, false otherwise. */
    DLL bool DecodeSound ( Sound& aSound, const std::string& aFileName );
    /*@}*/
}
#endif
