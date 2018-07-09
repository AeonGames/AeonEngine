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
#if 0
        MOCK_METHOD1 ( SetterOneValue, void ( size_t ) );
        MOCK_METHOD2 ( SetterTwoValues, void ( size_t, size_t ) );
        MOCK_CONST_METHOD0 ( GetterOneValue, size_t() );
        MOCK_CONST_METHOD0 ( GetterTwoValues, size_t() );
#endif
    };

    TEST ( Property, OneFloat )
    {
        Property<RuntimeClass> property ( "FloatValue", "Float Values", "f", [] ( RuntimeClass * aRuntimeClass, const void* aTuple ) {}, [] ( const RuntimeClass * aRuntimeClass, void* aTuple ) {} );
        EXPECT_EQ ( property.GetBufferSize(), sizeof ( float ) );
    }
    TEST ( Property, TwoFloats )
    {
        Property<RuntimeClass> propertyff ( "FloatValuesA", "Float Values A", "ff", [] ( RuntimeClass * aRuntimeClass, const void* aTuple ) {}, [] ( const RuntimeClass * aRuntimeClass, void* aTuple ) {} );
        EXPECT_EQ ( propertyff.GetBufferSize(), sizeof ( float ) * 2 );
        Property<RuntimeClass> property2f ( "FloatValuesB", "Float Values B", "2f", [] ( RuntimeClass * aRuntimeClass, const void* aTuple ) {}, [] ( const RuntimeClass * aRuntimeClass, void* aTuple ) {} );
        EXPECT_EQ ( propertyff.GetBufferSize(), sizeof ( float ) * 2 );
    }
    TEST ( Property, AllTypes )
    {
        Property<RuntimeClass> propertyff ( "AllTypes", "All Types", "cbB?hHiIlLfqQd", [] ( RuntimeClass * aRuntimeClass, const void* aTuple ) {}, [] ( const RuntimeClass * aRuntimeClass, void* aTuple ) {} );
        EXPECT_EQ ( propertyff.GetBufferSize(), sizeof ( uint8_t ) * 4 + sizeof ( uint16_t ) * 2 + sizeof ( uint32_t ) * 5 + sizeof ( uint64_t ) * 3 );
    }
}
