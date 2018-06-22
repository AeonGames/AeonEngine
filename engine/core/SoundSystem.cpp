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
#include <utility>

#include "aeongames/SoundSystem.h"
#include "Factory.h"

namespace AeonGames
{
    std::unique_ptr<SoundSystem> GetSoundSystem ( const std::string& aIdentifier )
    {
        return Factory<SoundSystem>::Construct ( aIdentifier );
    }
    bool RegisterSoundSystemLoader ( const std::string& aIdentifier, const std::function<std::unique_ptr<SoundSystem>() >& aLoader )
    {
        return Factory<SoundSystem>::RegisterConstructor ( aIdentifier, aLoader );
    }
    bool UnregisterSoundSystemLoader ( const std::string& aIdentifier )
    {
        return Factory<SoundSystem>::UnregisterConstructor ( aIdentifier );
    }
    void EnumerateSoundSystemLoaders ( const std::function<bool ( const std::string& ) >& aEnumerator )
    {
        Factory<SoundSystem>::EnumerateConstructors ( aEnumerator );
    }
}
