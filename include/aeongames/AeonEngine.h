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
#ifndef AEONGAMES_AEONENGINE_H
#define AEONGAMES_AEONENGINE_H
#include "Platform.h"
#include "Memory.h"
#include <string>
#include <vector>

namespace AeonGames
{
    DLL bool Initialize();
    DLL void Finalize();
    DLL std::vector<std::string> GetResourcePath();
    DLL void SetResourcePath ( const std::initializer_list<std::string>& aPath );
}
#endif
