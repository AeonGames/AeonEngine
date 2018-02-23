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
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"
#include "aeongames/ResourceCache.h"
#include "Factory.h"
#include <unordered_map>
#include <utility>

namespace AeonGames
{
    std::shared_ptr<Image> GetImage ( const std::string& aIdentifier, const std::string& aFilename )
    {
        return Factory<Image, const std::string&>::Get ( aIdentifier, aFilename );
    }
    bool RegisterImageLoader ( const std::string& aExt, const std::function<std::shared_ptr<Image> ( const std::string& ) >& aLoader )
    {
        return Factory<Image, const std::string&>::RegisterLoader ( aExt, aLoader );
    }
    bool UnregisterImageLoader ( const std::string& aExt )
    {
        return Factory<Image, const std::string&>::UnregisterLoader ( aExt );
    }
}
