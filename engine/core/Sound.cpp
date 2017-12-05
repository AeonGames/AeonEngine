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
#include "aeongames/Sound.h"
#include "aeongames/Utilities.h"
#include "aeongames/ResourceCache.h"
#include "Factory.h"
#include <unordered_map>

namespace AeonGames
{
    std::shared_ptr<Sound> GetSound ( const std::string& aIdentifier, const std::string& aFilename )
    {
        return Factory<Sound, const std::string&>::Get ( aIdentifier, aFilename );
    }
    bool RegisterSoundLoader ( const std::string& aExt, std::function<std::shared_ptr<Sound> ( const std::string& ) > aLoader )
    {
        return Factory<Sound, const std::string&>::RegisterLoader ( aExt, aLoader );
    }
    bool UnregisterSoundLoader ( const std::string& aExt )
    {
        return Factory<Sound, const std::string&>::UnregisterLoader ( aExt );
    }
}
