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
#include "aeongames/MessageWrapper.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif
#include "test.pb.h"
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( MessageWrapper, Construction )
    {
        TestBuffer test_buffer;
        MessageWrapper message_wrapper ( &test_buffer );
        const google::protobuf::Descriptor* descriptor = message_wrapper.GetMessagePtr()->GetDescriptor();
        for ( int i = 0; i < descriptor->field_count(); ++i )
        {
            MessageWrapper::Field& field = message_wrapper.GetField ( descriptor->field ( i ) );
            /**@todo add Set mutators to Field class.*/
        }
    }
}
