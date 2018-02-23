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
#include <utility>

#include "aeongames/Renderer.h"
#include "aeongames/Model.h"
#include "Factory.h"

namespace AeonGames
{
    std::shared_ptr<Renderer> GetRenderer ( const std::string& aIdentifier )
    {
        return Factory<Renderer>::Get ( aIdentifier );
    }
    bool RegisterRendererLoader ( const std::string& aIdentifier, const std::function<std::shared_ptr<Renderer>() >& aLoader )
    {
        return Factory<Renderer>::RegisterLoader ( aIdentifier, aLoader );
    }
    bool UnregisterRendererLoader ( const std::string& aIdentifier )
    {
        return Factory<Renderer>::UnregisterLoader ( aIdentifier );
    }
    void EnumerateRendererLoaders ( const std::function<bool ( const std::string& ) >& aEnumerator )
    {
        Factory<Renderer>::EnumerateLoaders ( aEnumerator );
    }
}
