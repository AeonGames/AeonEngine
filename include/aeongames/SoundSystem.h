/*
Copyright (C) 2017,2018 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_SOUNDSYSTEM_H
#define AEONGAMES_SOUNDSYSTEM_H

#include "Platform.h"
#include "aeongames/Memory.h"
#include <string>
#include <functional>

namespace AeonGames
{
    class SoundSystem
    {
    public:
    private:
    };

    /**@name Factory Functions */
    /*@{*/
    DLL std::unique_ptr<SoundSystem> GetSoundSystem ( const std::string& aIdentifier );
    /** Registers a SoundSystem loader for a specific identifier.*/
    DLL bool RegisterSoundSystemLoader ( const std::string& aIdentifier, const std::function<std::unique_ptr<SoundSystem>() >& aLoader );
    /** Unregisters a SoundSystem loader for a specific identifier.*/
    DLL bool UnregisterSoundSystemLoader ( const std::string& aIdentifier );
    /** Enumerates SoundSystem loader identifiers via an enumerator functor.*/
    DLL void EnumerateSoundSystemLoaders ( const std::function<bool ( const std::string& ) >& aEnumerator );
    /*@}*/
}
#endif
