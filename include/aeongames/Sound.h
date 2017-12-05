/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

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
#include "Platform.h"
#include <cstdint>
#include "aeongames/Memory.h"
#include <functional>
#include <string>
namespace AeonGames
{
    class Sound
    {
    public:
        virtual ~Sound() = default;
    };
    /** Factory Function */
    DLL std::shared_ptr<Sound> GetSound ( const std::string& aIdentifier, const std::string& aFilename );
    /** Registers a sound loader for a filename extension.*/
    DLL bool RegisterSoundLoader ( const std::string& aIdentifier, std::function<std::shared_ptr<Sound> ( const std::string& ) > aLoader );
    /** Unregisters a sound loader for a filename extension.*/
    DLL bool UnregisterSoundLoader ( const std::string& aIdentifier );
}
#endif
