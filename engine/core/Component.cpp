/*
Copyright (C) 2018,2019 Rodrigo Jose Hernandez Cordoba

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
#include "Factory.h"
#include "aeongames/StringId.h"
#include "aeongames/Component.h"

namespace AeonGames
{
    Component::~Component() = default;
    void Component::SetProperty ( const StringId& aId, const Property& aProperty )
    {
        SetProperty ( aId.GetId(), aProperty );
    }
    void Component::SetProperty ( const std::string& aId, const Property& aProperty )
    {
        SetProperty ( crc32i ( aId.data(), aId.size() ), aProperty );
    }
    FactoryImplementation ( Component );
}