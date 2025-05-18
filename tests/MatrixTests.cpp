/*
Copyright (C) 2025 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/Matrix4x4.h"
#include "../engine/math/3DMath.h"
#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( Matrix4x4, Multiplication )
    {
        Matrix4x4 a{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
        Matrix4x4 b{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
        Matrix4x4 c{a * b};
        EXPECT_EQ ( c[0], a[0] * b[0] + a[ 4 ] * b[ 1 ] + a[ 8 ] * b[ 2 ] + a[ 12 ] * b[ 3 ] );
        EXPECT_EQ ( c[ 4 ], a[0] * b[ 4 ] + a[ 4 ] * b[5] + a[ 8 ] * b[ 6 ] + a[ 12 ] * b[ 7 ] );
        EXPECT_EQ ( c[ 8 ], a[0] * b[ 8 ] + a[ 4 ] * b[ 9 ] + a[ 8 ] * b[10] + a[ 12 ] * b[ 11 ] );
        EXPECT_EQ ( c[ 12 ], a[0] * b[ 12 ] + a[ 4 ] * b[ 13 ] + a[ 8 ] * b[ 14 ] + a[ 12 ] * b[15] );
        EXPECT_EQ ( c[ 1 ], a[ 1 ] * b[0] + a[5] * b[ 1 ] + a[ 9 ] * b[ 2 ] + a[ 13 ] * b[ 3 ] );
        EXPECT_EQ ( c[5], a[ 1 ] * b[ 4 ] + a[5] * b[5] + a[ 9 ] * b[ 6 ] + a[ 13 ] * b[ 7 ] );
        EXPECT_EQ ( c[ 9 ], a[ 1 ] * b[ 8 ] + a[5] * b[ 9 ] + a[ 9 ] * b[10] + a[ 13 ] * b[ 11 ] );
        EXPECT_EQ ( c[ 13 ], a[ 1 ] * b[ 12 ] + a[5] * b[ 13 ] + a[ 9 ] * b[ 14 ] + a[ 13 ] * b[15] );
        EXPECT_EQ ( c[ 2 ], a[ 2 ] * b[0] + a[ 6 ] * b[ 1 ] + a[10] * b[ 2 ] + a[ 14 ] * b[ 3 ] );
        EXPECT_EQ ( c[ 6 ], a[ 2 ] * b[ 4 ] + a[ 6 ] * b[5] + a[10] * b[ 6 ] + a[ 14 ] * b[ 7 ] );
        EXPECT_EQ ( c[10], a[ 2 ] * b[ 8 ] + a[ 6 ] * b[ 9 ] + a[10] * b[10] + a[ 14 ] * b[ 11 ] );
        EXPECT_EQ ( c[ 14 ], a[ 2 ] * b[ 12 ] + a[ 6 ] * b[ 13 ] + a[10] * b[ 14 ] + a[ 14 ] * b[15] );
        EXPECT_EQ ( c[ 3 ], a[ 3 ] * b[0] + a[ 7 ] * b[ 1 ] + a[ 11 ] * b[ 2 ] + a[15] * b[ 3 ] );
        EXPECT_EQ ( c[ 7 ], a[ 3 ] * b[ 4 ] + a[ 7 ] * b[5] + a[ 11 ] * b[ 6 ] + a[15] * b[ 7 ] );
        EXPECT_EQ ( c[ 11 ], a[ 3 ] * b[ 8 ] + a[ 7 ] * b[ 9 ] + a[ 11 ] * b[10] + a[15] * b[ 11 ] );
        EXPECT_EQ ( c[15], a[ 3 ] * b[ 12 ] + a[ 7 ] * b[ 13 ] + a[ 11 ] * b[ 14 ] + a[15] * b[15] );
    }
}
