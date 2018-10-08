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
#include "aeongames/Archive.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;

namespace AeonGames
{
    TEST ( Archive, HappyPath )
    {
        AeonGames::Archive<std::string, std::string> archive;
        std::string* value_stored = archive.Store ( "Key", "Value" );
        std::string* value_retrieved = archive.Get ( "Key" );
        EXPECT_EQ ( *value_stored, "Value" );
        EXPECT_EQ ( archive.GetKey ( value_stored ), "Key" );
        EXPECT_EQ ( value_stored, value_retrieved );
        std::unique_ptr<std::string> ownership = archive.Dispose ( "Key" );
        EXPECT_EQ ( *ownership, "Value" );
        EXPECT_EQ ( archive.Get ( "Key" ), nullptr );
    }
    TEST ( Archive, NoArgs )
    {
        AeonGames::Archive<std::string, std::string> archive;
        std::string* value_stored = archive.Store ( "Key" );
        std::string* value_retrieved = archive.Get ( "Key" );
        EXPECT_EQ ( *value_stored, "" );
        EXPECT_EQ ( value_stored, value_retrieved );
        EXPECT_EQ ( archive.GetKey ( value_stored ), "Key" );
        std::unique_ptr<std::string> ownership = archive.Dispose ( "Key" );
        EXPECT_EQ ( *ownership, "" );
        EXPECT_EQ ( archive.Get ( "Key" ), nullptr );
    }
}
