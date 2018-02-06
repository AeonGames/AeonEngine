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
#include <functional>
#include <string>
#include "aeongames/CRC.h"
#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( CRC, CRC32 )
    {
        EXPECT_TRUE ( crc32i ( "AeonGames", 9 ) == 0x2B0C3B );
    }

    TEST ( CRC, CRC64 )
    {
        EXPECT_TRUE ( crc64i ( "AeonGames", 9 ) == 0x187936cc3eca327f );
    }
}
