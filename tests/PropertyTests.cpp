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
#include <cstdint>
#include <string>
#include "aeongames/Property.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( PropertyRef, Constructor )
    {
        std::string string{""};
        PropertyRef property{"reference", string};
        EXPECT_TRUE ( property.HasType<std::string>() );
        EXPECT_FALSE ( property.HasType<int>() );
        EXPECT_EQ ( property.Get<std::string>(), "" );
        property.Get<std::string>() = "String";
        EXPECT_EQ ( property.Get<std::string>(), "String" );
        EXPECT_EQ ( property.GetName(), "reference" );
    }

    TEST ( PropertyRef, ModifyFromConstReference )
    {
        std::string string{""};
        const PropertyRef property{"reference", string};
        EXPECT_EQ ( property.Get<std::string>(), "" );
        property.Get<std::string>() = "String";
        EXPECT_EQ ( property.Get<std::string>(), "String" );
    }

    TEST ( PropertyRef, ModifyFromConstReferenceUsingSet )
    {
        std::string string{""};
        const PropertyRef property{"reference", string};
        EXPECT_EQ ( property.Get<std::string>(), "" );
        property.Set ( std::string{"String"} );
        EXPECT_EQ ( property.Get<std::string>(), "String" );
    }
}
