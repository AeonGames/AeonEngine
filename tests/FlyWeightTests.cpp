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

#include "aeongames/FlyWeight.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;

namespace AeonGames
{
    class FlyWeightPayload : public FlyWeight<size_t, FlyWeightPayload>
    {
    public:
        FlyWeightPayload() = default;
        FlyWeightPayload ( const FlyWeightPayload& aPayload ) :
            FlyWeight<size_t, FlyWeightPayload> ( aPayload ) {};
        virtual ~FlyWeightPayload() = default;
        FlyWeightPayload ( size_t aKey ) : FlyWeight ( aKey ) {}
        MOCK_CONST_METHOD0 ( Function, void() );
    };
    TEST ( FlyWeight, ZeroKeyConstructorThrowsException )
    {
        EXPECT_THROW ( FlyWeightPayload{0}, std::runtime_error );
    }
    TEST ( FlyWeight, ZeroKeyPackingThrowsException )
    {
        FlyWeightPayload payload;
        EXPECT_THROW ( payload.Pack ( 0 ), std::runtime_error );
    }

    TEST ( FlyWeight, ZeroKeyHandleConstructorThrowsException )
    {
        EXPECT_THROW ( FlyWeightPayload::Handle{0}, std::runtime_error );
    }

    TEST ( FlyWeight, PackingWorks )
    {
        FlyWeightPayload payload;
        FlyWeightPayload::Handle handle = payload.Pack ( 1 );
        EXPECT_EQ ( &payload, handle.Get() );
    }

    TEST ( FlyWeight, ContextInvalidatesHandle )
    {
        FlyWeightPayload::Handle handle{1};
        {
            FlyWeightPayload payload{1};
            EXPECT_CALL ( payload, Function() ).Times ( 1 );
            EXPECT_EQ ( &payload, handle.Get() );
            handle->Function();
        }
        EXPECT_EQ ( nullptr, handle.Get() );
        EXPECT_THROW ( handle->Function(), std::runtime_error );
    }

    TEST ( FlyWeight, MemberOfPointerOperator )
    {
        FlyWeightPayload payload{1};
        EXPECT_CALL ( payload, Function() ).Times ( 1 );
        payload.GetHandle()->Function();
    }
    TEST ( FlyWeight, IndirectionOperator )
    {
        FlyWeightPayload payload{1};
        EXPECT_CALL ( payload, Function() ).Times ( 1 );
        ( *payload.GetHandle() ).Function();
    }
    TEST ( FlyWeight, AddressOfOperator )
    {
        FlyWeightPayload payload{1};
        EXPECT_CALL ( payload, Function() ).Times ( 1 );
        ( &payload.GetHandle() )->Function();
    }
    TEST ( FlyWeight, CopyConstructor )
    {
        FlyWeightPayload payload1{1};
        FlyWeightPayload payload2{payload1};
        EXPECT_THROW ( payload2.GetHandle()->Function(), std::runtime_error );
    }
}
