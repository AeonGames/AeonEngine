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
#include "aeongames/Quaternion.h"
#include "aeongames/Vector3.h"
#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( Quaternion, GetEuler )
    {
        Quaternion quaternion{0, 0, 1, 0};
        Vector3 euler = quaternion.GetEuler();
        EXPECT_EQ ( euler, Vector3 ( 180, 0, 0 ) );
    }
    TEST ( Quaternion, SetEuler )
    {
        Quaternion quaternion;
        quaternion.SetEuler ( {180, 0, 0} );
        EXPECT_EQ ( quaternion, Quaternion ( 0, 0, 1, 0 ) );
    }
}
