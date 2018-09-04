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
    TEST ( MessageWrapper, SingleFieldAssignment )
    {
        TestBuffer test_buffer;
        MessageWrapper message_wrapper ( &test_buffer );
        const google::protobuf::Descriptor* descriptor = message_wrapper.GetMessagePtr()->GetDescriptor();
        message_wrapper.GetField ( descriptor->FindFieldByName ( "uint32" ) ).SetUInt32 ( 42 );
        EXPECT_EQ ( test_buffer.uint32(), 42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "int32" ) ).SetInt32 ( -42 );
        EXPECT_EQ ( test_buffer.int32(), -42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "uint64" ) ).SetUInt64 ( 42 );
        EXPECT_EQ ( test_buffer.uint64(), 42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "int64" ) ).SetInt64 ( -42 );
        EXPECT_EQ ( test_buffer.int64(), -42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "double_field" ) ).SetDouble ( 3.1415 );
        EXPECT_DOUBLE_EQ ( test_buffer.double_field(), 3.1415 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "float_field" ) ).SetFloat ( 3.1415f );
        EXPECT_FLOAT_EQ ( test_buffer.float_field(), 3.1415f );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "boolean" ) ).SetBool ( true );
        EXPECT_EQ ( test_buffer.boolean(), true );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "string" ) ).SetString ( "String Field" );
        EXPECT_EQ ( test_buffer.string(), "String Field" );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "enumeration" ) ).SetEnum ( descriptor->FindEnumValueByName ( "ONE" ) );
        EXPECT_EQ ( test_buffer.enumeration(), TestBuffer::ONE );
    }

    TEST ( MessageWrapper, SingleFieldAssignmentSubmessage )
    {
        TestBuffer test_buffer;
        MessageWrapper message_wrapper ( &test_buffer );
        const google::protobuf::Descriptor* descriptor = message_wrapper.GetMessagePtr()->GetDescriptor();
        MessageWrapper::Field& field = message_wrapper.GetField ( descriptor->FindFieldByName ( "submessage" ) );

        field.GetField ( descriptor->FindFieldByName ( "uint32" ) ).SetUInt32 ( 42 );
        EXPECT_EQ ( test_buffer.submessage().uint32(), 42 );
        field.GetField ( descriptor->FindFieldByName ( "int32" ) ).SetInt32 ( -42 );
        EXPECT_EQ ( test_buffer.submessage().int32(), -42 );
        field.GetField ( descriptor->FindFieldByName ( "uint64" ) ).SetUInt64 ( 42 );
        EXPECT_EQ ( test_buffer.submessage().uint64(), 42 );
        field.GetField ( descriptor->FindFieldByName ( "int64" ) ).SetInt64 ( -42 );
        EXPECT_EQ ( test_buffer.submessage().int64(), -42 );
        field.GetField ( descriptor->FindFieldByName ( "double_field" ) ).SetDouble ( 3.1415 );
        EXPECT_DOUBLE_EQ ( test_buffer.submessage().double_field(), 3.1415 );
        field.GetField ( descriptor->FindFieldByName ( "float_field" ) ).SetFloat ( 3.1415f );
        EXPECT_FLOAT_EQ ( test_buffer.submessage().float_field(), 3.1415f );
        field.GetField ( descriptor->FindFieldByName ( "boolean" ) ).SetBool ( true );
        EXPECT_EQ ( test_buffer.submessage().boolean(), true );
        field.GetField ( descriptor->FindFieldByName ( "string" ) ).SetString ( "String Field" );
        EXPECT_EQ ( test_buffer.submessage().string(), "String Field" );
        field.GetField ( descriptor->FindFieldByName ( "enumeration" ) ).SetEnum ( descriptor->FindEnumValueByName ( "ONE" ) );
        EXPECT_EQ ( test_buffer.submessage().enumeration(), TestBuffer::ONE );
    }

    TEST ( MessageWrapper, RepeatedFieldAssignment )
    {
        TestBuffer test_buffer;
        MessageWrapper message_wrapper ( &test_buffer );
        const google::protobuf::Descriptor* descriptor = message_wrapper.GetMessagePtr()->GetDescriptor();

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_uint32" ) ).AddUInt32 ( 42 );
        EXPECT_EQ ( test_buffer.repeated_uint32() [0], 42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_uint32" ) ).SetUInt32 ( 84, 0 );
        EXPECT_EQ ( test_buffer.repeated_uint32() [0], 84 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_uint32" ) ).AddUInt32 ( 42 );
        EXPECT_EQ ( test_buffer.repeated_uint32() [1], 42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_uint32" ) ).SetUInt32 ( 84, 1 );
        EXPECT_EQ ( test_buffer.repeated_uint32() [1], 84 );

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_int32" ) ).AddInt32 ( -42 );
        EXPECT_EQ ( test_buffer.repeated_int32() [0], -42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_int32" ) ).SetInt32 ( -84, 0 );
        EXPECT_EQ ( test_buffer.repeated_int32() [0], -84 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_int32" ) ).AddInt32 ( -42 );
        EXPECT_EQ ( test_buffer.repeated_int32() [1], -42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_int32" ) ).SetInt32 ( -84, 1 );
        EXPECT_EQ ( test_buffer.repeated_int32() [1], -84 );

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_uint64" ) ).AddUInt64 ( 42 );
        EXPECT_EQ ( test_buffer.repeated_uint64() [0], 42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_uint64" ) ).SetUInt64 ( 84, 0 );
        EXPECT_EQ ( test_buffer.repeated_uint64() [0], 84 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_uint64" ) ).AddUInt64 ( 42 );
        EXPECT_EQ ( test_buffer.repeated_uint64() [1], 42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_uint64" ) ).SetUInt64 ( 84, 1 );
        EXPECT_EQ ( test_buffer.repeated_uint64() [1], 84 );

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_int64" ) ).AddInt64 ( -42 );
        EXPECT_EQ ( test_buffer.repeated_int64() [0], -42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_int64" ) ).SetInt64 ( -84, 0 );
        EXPECT_EQ ( test_buffer.repeated_int64() [0], -84 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_int64" ) ).AddInt64 ( -42 );
        EXPECT_EQ ( test_buffer.repeated_int64() [1], -42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_int64" ) ).SetInt64 ( -84, 1 );
        EXPECT_EQ ( test_buffer.repeated_int64() [1], -84 );

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_double" ) ).AddDouble ( -42.42 );
        EXPECT_DOUBLE_EQ ( test_buffer.repeated_double() [0], -42.42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_double" ) ).SetDouble ( -84.84, 0 );
        EXPECT_DOUBLE_EQ ( test_buffer.repeated_double() [0], -84.84 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_double" ) ).AddDouble ( -42.42 );
        EXPECT_DOUBLE_EQ ( test_buffer.repeated_double() [1], -42.42 );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_double" ) ).SetDouble ( -84.84, 1 );
        EXPECT_DOUBLE_EQ ( test_buffer.repeated_double() [1], -84.84 );

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_float" ) ).AddFloat ( -42.42f );
        EXPECT_FLOAT_EQ ( test_buffer.repeated_float() [0], -42.42f );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_float" ) ).SetFloat ( -84.84f, 0 );
        EXPECT_FLOAT_EQ ( test_buffer.repeated_float() [0], -84.84f );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_float" ) ).AddFloat ( -42.42f );
        EXPECT_FLOAT_EQ ( test_buffer.repeated_float() [1], -42.42f );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_float" ) ).SetFloat ( -84.84f, 1 );
        EXPECT_FLOAT_EQ ( test_buffer.repeated_float() [1], -84.84f );

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_boolean" ) ).AddBool ( true );
        EXPECT_EQ ( test_buffer.repeated_boolean() [0], true );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_boolean" ) ).SetBool ( false, 0 );
        EXPECT_EQ ( test_buffer.repeated_boolean() [0], false );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_boolean" ) ).AddBool ( true );
        EXPECT_EQ ( test_buffer.repeated_boolean() [1], true );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_boolean" ) ).SetBool ( false, 1 );
        EXPECT_EQ ( test_buffer.repeated_boolean() [1], false );

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_string" ) ).AddString ( "String Field 1" );
        EXPECT_EQ ( test_buffer.repeated_string() [0], "String Field 1" );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_string" ) ).SetString ( "String Field One", 0 );
        EXPECT_EQ ( test_buffer.repeated_string() [0], "String Field One" );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_string" ) ).AddString ( "String Field 2" );
        EXPECT_EQ ( test_buffer.repeated_string() [1], "String Field 2" );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_string" ) ).SetString ( "String Field Two", 1 );
        EXPECT_EQ ( test_buffer.repeated_string() [1], "String Field Two" );

        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_enumeration" ) ).AddEnum ( descriptor->FindEnumValueByName ( "ONE" ) );
        EXPECT_EQ ( test_buffer.repeated_enumeration() [0], TestBuffer::ONE );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_enumeration" ) ).SetEnum ( descriptor->FindEnumValueByName ( "ZERO" ), 0 );
        EXPECT_EQ ( test_buffer.repeated_enumeration() [0], TestBuffer::ZERO );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_enumeration" ) ).AddEnum ( descriptor->FindEnumValueByName ( "TWO" ) );
        EXPECT_EQ ( test_buffer.repeated_enumeration() [1], TestBuffer::TWO );
        message_wrapper.GetField ( descriptor->FindFieldByName ( "repeated_enumeration" ) ).SetEnum ( descriptor->FindEnumValueByName ( "ZERO" ), 1 );
        EXPECT_EQ ( test_buffer.repeated_enumeration() [1], TestBuffer::ZERO );
    }

    TEST ( MessageWrapper, RepeatedFieldAssignmentSubmessage )
    {
        TestBuffer test_buffer;
        MessageWrapper message_wrapper ( &test_buffer );
        const google::protobuf::Descriptor* descriptor = message_wrapper.GetMessagePtr()->GetDescriptor();
        MessageWrapper::Field& field = message_wrapper.GetField ( descriptor->FindFieldByName ( "submessage" ) );

        field.GetField ( descriptor->FindFieldByName ( "repeated_uint32" ) ).AddUInt32 ( 42 );
        EXPECT_EQ ( test_buffer.submessage().repeated_uint32() [0], 42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_uint32" ) ).SetUInt32 ( 84, 0 );
        EXPECT_EQ ( test_buffer.submessage().repeated_uint32() [0], 84 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_uint32" ) ).AddUInt32 ( 42 );
        EXPECT_EQ ( test_buffer.submessage().repeated_uint32() [1], 42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_uint32" ) ).SetUInt32 ( 84, 1 );
        EXPECT_EQ ( test_buffer.submessage().repeated_uint32() [1], 84 );

        field.GetField ( descriptor->FindFieldByName ( "repeated_int32" ) ).AddInt32 ( -42 );
        EXPECT_EQ ( test_buffer.submessage().repeated_int32() [0], -42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_int32" ) ).SetInt32 ( -84, 0 );
        EXPECT_EQ ( test_buffer.submessage().repeated_int32() [0], -84 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_int32" ) ).AddInt32 ( -42 );
        EXPECT_EQ ( test_buffer.submessage().repeated_int32() [1], -42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_int32" ) ).SetInt32 ( -84, 1 );
        EXPECT_EQ ( test_buffer.submessage().repeated_int32() [1], -84 );

        field.GetField ( descriptor->FindFieldByName ( "repeated_uint64" ) ).AddUInt64 ( 42 );
        EXPECT_EQ ( test_buffer.submessage().repeated_uint64() [0], 42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_uint64" ) ).SetUInt64 ( 84, 0 );
        EXPECT_EQ ( test_buffer.submessage().repeated_uint64() [0], 84 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_uint64" ) ).AddUInt64 ( 42 );
        EXPECT_EQ ( test_buffer.submessage().repeated_uint64() [1], 42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_uint64" ) ).SetUInt64 ( 84, 1 );
        EXPECT_EQ ( test_buffer.submessage().repeated_uint64() [1], 84 );

        field.GetField ( descriptor->FindFieldByName ( "repeated_int64" ) ).AddInt64 ( -42 );
        EXPECT_EQ ( test_buffer.submessage().repeated_int64() [0], -42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_int64" ) ).SetInt64 ( -84, 0 );
        EXPECT_EQ ( test_buffer.submessage().repeated_int64() [0], -84 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_int64" ) ).AddInt64 ( -42 );
        EXPECT_EQ ( test_buffer.submessage().repeated_int64() [1], -42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_int64" ) ).SetInt64 ( -84, 1 );
        EXPECT_EQ ( test_buffer.submessage().repeated_int64() [1], -84 );

        field.GetField ( descriptor->FindFieldByName ( "repeated_double" ) ).AddDouble ( -42.42 );
        EXPECT_DOUBLE_EQ ( test_buffer.submessage().repeated_double() [0], -42.42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_double" ) ).SetDouble ( -84.84, 0 );
        EXPECT_DOUBLE_EQ ( test_buffer.submessage().repeated_double() [0], -84.84 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_double" ) ).AddDouble ( -42.42 );
        EXPECT_DOUBLE_EQ ( test_buffer.submessage().repeated_double() [1], -42.42 );
        field.GetField ( descriptor->FindFieldByName ( "repeated_double" ) ).SetDouble ( -84.84, 1 );
        EXPECT_DOUBLE_EQ ( test_buffer.submessage().repeated_double() [1], -84.84 );

        field.GetField ( descriptor->FindFieldByName ( "repeated_float" ) ).AddFloat ( -42.42f );
        EXPECT_FLOAT_EQ ( test_buffer.submessage().repeated_float() [0], -42.42f );
        field.GetField ( descriptor->FindFieldByName ( "repeated_float" ) ).SetFloat ( -84.84f, 0 );
        EXPECT_FLOAT_EQ ( test_buffer.submessage().repeated_float() [0], -84.84f );
        field.GetField ( descriptor->FindFieldByName ( "repeated_float" ) ).AddFloat ( -42.42f );
        EXPECT_FLOAT_EQ ( test_buffer.submessage().repeated_float() [1], -42.42f );
        field.GetField ( descriptor->FindFieldByName ( "repeated_float" ) ).SetFloat ( -84.84f, 1 );
        EXPECT_FLOAT_EQ ( test_buffer.submessage().repeated_float() [1], -84.84f );

        field.GetField ( descriptor->FindFieldByName ( "repeated_boolean" ) ).AddBool ( true );
        EXPECT_EQ ( test_buffer.submessage().repeated_boolean() [0], true );
        field.GetField ( descriptor->FindFieldByName ( "repeated_boolean" ) ).SetBool ( false, 0 );
        EXPECT_EQ ( test_buffer.submessage().repeated_boolean() [0], false );
        field.GetField ( descriptor->FindFieldByName ( "repeated_boolean" ) ).AddBool ( true );
        EXPECT_EQ ( test_buffer.submessage().repeated_boolean() [1], true );
        field.GetField ( descriptor->FindFieldByName ( "repeated_boolean" ) ).SetBool ( false, 1 );
        EXPECT_EQ ( test_buffer.submessage().repeated_boolean() [1], false );

        field.GetField ( descriptor->FindFieldByName ( "repeated_string" ) ).AddString ( "String Field 1" );
        EXPECT_EQ ( test_buffer.submessage().repeated_string() [0], "String Field 1" );
        field.GetField ( descriptor->FindFieldByName ( "repeated_string" ) ).SetString ( "String Field One", 0 );
        EXPECT_EQ ( test_buffer.submessage().repeated_string() [0], "String Field One" );
        field.GetField ( descriptor->FindFieldByName ( "repeated_string" ) ).AddString ( "String Field 2" );
        EXPECT_EQ ( test_buffer.submessage().repeated_string() [1], "String Field 2" );
        field.GetField ( descriptor->FindFieldByName ( "repeated_string" ) ).SetString ( "String Field Two", 1 );
        EXPECT_EQ ( test_buffer.submessage().repeated_string() [1], "String Field Two" );

        field.GetField ( descriptor->FindFieldByName ( "repeated_enumeration" ) ).AddEnum ( descriptor->FindEnumValueByName ( "ONE" ) );
        EXPECT_EQ ( test_buffer.submessage().repeated_enumeration() [0], TestBuffer::ONE );
        field.GetField ( descriptor->FindFieldByName ( "repeated_enumeration" ) ).SetEnum ( descriptor->FindEnumValueByName ( "ZERO" ), 0 );
        EXPECT_EQ ( test_buffer.submessage().repeated_enumeration() [0], TestBuffer::ZERO );
        field.GetField ( descriptor->FindFieldByName ( "repeated_enumeration" ) ).AddEnum ( descriptor->FindEnumValueByName ( "TWO" ) );
        EXPECT_EQ ( test_buffer.submessage().repeated_enumeration() [1], TestBuffer::TWO );
        field.GetField ( descriptor->FindFieldByName ( "repeated_enumeration" ) ).SetEnum ( descriptor->FindEnumValueByName ( "ZERO" ), 1 );
        EXPECT_EQ ( test_buffer.submessage().repeated_enumeration() [1], TestBuffer::ZERO );
    }
}
