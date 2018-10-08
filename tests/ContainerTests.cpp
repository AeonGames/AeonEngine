/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

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
#include <string>
#include "aeongames/Container.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;

namespace AeonGames
{
    TEST ( Container, HappyPath )
    {
        AeonGames::Container<std::string> container;
        std::string* value_stored = container.Store ( "Value" );
        EXPECT_EQ ( *value_stored, "Value" );
        std::unique_ptr<std::string> ownership = container.Dispose ( value_stored );
        EXPECT_EQ ( *ownership, "Value" );
    }
    TEST ( Container, NoArgs )
    {
        AeonGames::Container<std::string> container;
        std::string* value_stored = container.Store();
        EXPECT_EQ ( *value_stored, "" );
        std::unique_ptr<std::string> ownership = container.Dispose ( value_stored );
        EXPECT_EQ ( *ownership, "" );
    }
}
