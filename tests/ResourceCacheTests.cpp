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
#include <memory>
#include "aeongames/ResourceCache.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( ResourceCache, HappyPath )
    {
        ResourceCache<size_t, std::string> cache;
        std::shared_ptr<std::string> string_ptr = cache.Get ( 0, "Test!" );
        EXPECT_EQ ( *string_ptr, "Test!" );
        EXPECT_EQ ( cache.GetKey ( string_ptr ), 0 );
    }
}