/*
Copyright (C) 2017-2019 Rodrigo Jose Hernandez Cordoba

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
#include <memory>
#include <functional>

namespace AeonGames
{
    class StringId;
    class SoundSystem
    {
    public:
    private:
    };

    /**@name Factory Functions */
    /*@{*/
    DLL std::unique_ptr<SoundSystem> ConstructSoundSystem ( const StringId& aIdentifier );
    /** Registers a SoundSystem loader for a specific identifier.*/
    DLL bool RegisterSoundSystemConstructor ( const StringId& aIdentifier, const std::function<std::unique_ptr<SoundSystem>() >& aConstructor );
    /** Unregisters a SoundSystem loader for a specific identifier.*/
    DLL bool UnregisterSoundSystemConstructor ( const StringId& aIdentifier );
    /** Enumerates SoundSystem loader identifiers via an enumerator functor.*/
    DLL void EnumerateSoundSystemConstructors ( const std::function<bool ( const StringId& ) >& aEnumerator );
    /*@}*/
}
#endif
