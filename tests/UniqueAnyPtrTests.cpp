/*
Copyright (C) 2018,2025 Rodrigo Jose Hernandez Cordoba

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

#include <iostream>
#include <cstdint>
#include <string>
#include "aeongames/UniqueAnyPtr.hpp"
#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( UniqueAnyPtr, Constructor )
    {
        UniqueAnyPtr resource1{new std::string{"Resource 1"}};
        UniqueAnyPtr resource2{std::make_unique<std::string> ( "Resource 2" ) };
        UniqueAnyPtr resource3{std::move ( resource2 ) };
        UniqueAnyPtr resource4{MakeUniqueAny<std::string> ( "Resource 4" ) };
        UniqueAnyPtr resource5{std::string ( "Resource 5" ) };
    }

    TEST ( UniqueAnyPtr, Assignment )
    {
        UniqueAnyPtr resource1 = new std::string{"Resource 1"};
        UniqueAnyPtr resource2 = std::make_unique<std::string> ( "Resource 2" );
        UniqueAnyPtr resource3 = std::move ( resource2 );
        UniqueAnyPtr resource4 = MakeUniqueAny<std::string> ( "Resource 4" );
    }
}
