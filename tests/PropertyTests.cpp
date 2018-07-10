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
#include "aeongames/Property.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;
namespace AeonGames
{
    class RuntimeClass
    {
    public:
        RuntimeClass() = default;
        ~RuntimeClass() = default;
        float FirstFloat{};
        float SecondFloat{};
    };

    TEST ( Property, OneFloat )
    {
        RuntimeClass runtime_class;
        float value{};
        Property<RuntimeClass> property ( "FloatValue", "Float Values", "f",
                                          [] ( RuntimeClass * aRuntimeClass, const void* aTuple )
        {
            aRuntimeClass->FirstFloat = *reinterpret_cast<const float*> ( aTuple );
        },
        [] ( const RuntimeClass * aRuntimeClass, void* aTuple )
        {
            *reinterpret_cast<float*> ( aTuple ) = aRuntimeClass->FirstFloat;
        } );
        EXPECT_EQ ( property.GetBufferSize(), sizeof ( float ) );
        property.SetByString ( &runtime_class, "3.1415" );
        EXPECT_EQ ( runtime_class.FirstFloat, 3.1415f );
        property.Get ( &runtime_class, &value );
        EXPECT_EQ ( runtime_class.FirstFloat, value );
    }
    TEST ( Property, TwoFloats )
    {
        RuntimeClass runtime_class;
        float value[2] {};
        Property<RuntimeClass> propertyff ( "FloatValuesA", "Float Values A", "ff",
                                            [] ( RuntimeClass * aRuntimeClass, const void* aTuple )
        {
            aRuntimeClass->FirstFloat = reinterpret_cast<const float*> ( aTuple ) [0];
            aRuntimeClass->SecondFloat = reinterpret_cast<const float*> ( aTuple ) [1];
        },
        [] ( const RuntimeClass * aRuntimeClass, void* aTuple )
        {
            reinterpret_cast<float*> ( aTuple ) [0] = aRuntimeClass->FirstFloat;
            reinterpret_cast<float*> ( aTuple ) [1] = aRuntimeClass->SecondFloat;
        } );
        EXPECT_EQ ( propertyff.GetBufferSize(), sizeof ( float ) * 2 );
        propertyff.SetByString ( &runtime_class, "3.1415 19.1277" );
        EXPECT_EQ ( runtime_class.FirstFloat, 3.1415f );
        EXPECT_EQ ( runtime_class.SecondFloat, 19.1277f );
        propertyff.Get ( &runtime_class, value );
        EXPECT_EQ ( runtime_class.FirstFloat, value[0] );
        EXPECT_EQ ( runtime_class.SecondFloat, value[1] );

        Property<RuntimeClass> property2f ( "FloatValuesB", "Float Values B", "2f",
                                            [] ( RuntimeClass * aRuntimeClass, const void* aTuple )
        {
            aRuntimeClass->FirstFloat = reinterpret_cast<const float*> ( aTuple ) [0];
            aRuntimeClass->SecondFloat = reinterpret_cast<const float*> ( aTuple ) [1];
        },
        [] ( const RuntimeClass * aRuntimeClass, void* aTuple )
        {
            reinterpret_cast<float*> ( aTuple ) [0] = aRuntimeClass->FirstFloat;
            reinterpret_cast<float*> ( aTuple ) [1] = aRuntimeClass->SecondFloat;
        } );
        EXPECT_EQ ( property2f.GetBufferSize(), sizeof ( float ) * 2 );
        property2f.SetByString ( &runtime_class, "3.1415 19.1277" );
        EXPECT_EQ ( runtime_class.FirstFloat, 3.1415f );
        EXPECT_EQ ( runtime_class.SecondFloat, 19.1277f );
        property2f.Get ( &runtime_class, value );
        EXPECT_EQ ( runtime_class.FirstFloat, value[0] );
        EXPECT_EQ ( runtime_class.SecondFloat, value[1] );
    }
    TEST ( Property, AllTypes )
    {
        Property<RuntimeClass> propertyff ( "AllTypes", "All Types", "cbB?hHiIlLfqQd", [] ( RuntimeClass * aRuntimeClass, const void* aTuple ) {}, [] ( const RuntimeClass * aRuntimeClass, void* aTuple ) {} );
        EXPECT_EQ ( propertyff.GetBufferSize(), sizeof ( uint8_t ) * 4 + sizeof ( uint16_t ) * 2 + sizeof ( uint32_t ) * 5 + sizeof ( uint64_t ) * 3 );
    }
}
