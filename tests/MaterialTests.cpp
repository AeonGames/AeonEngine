/*
Copyright (C) 2018,2019,2021,2025 Rodrigo Jose Hernandez Cordoba

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
#include <cstdint>
#include "aeongames/Material.hpp"
#include "aeongames/Vector2.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Vector4.hpp"
#include "aeongames/ProtoBufClasses.hpp"
#include "ProtoBufHelpers.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include "material.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    static char material_text[] =
    {
        "AEONMTL\n"
        "property {\n"
        "    uniform_name: \"Vector4\"\n"
        "    display_name: \"Vector 4\"\n"
        "    vector4 {\n"
        "        x: 4.0\n"
        "        y: 3.0\n"
        "        z: 2.0\n"
        "        w: 1.0\n"
        "    }\n"
        "}\n"
        "property {\n"
        "    uniform_name: \"Vector3\"\n"
        "    display_name: \"Vector 3\"\n"
        "    vector3 {\n"
        "        x: 3.0\n"
        "        y: 2.0\n"
        "        z: 1.0\n"
        "    }\n"
        "}\n"
        "property {\n"
        "    uniform_name: \"Vector2\"\n"
        "    display_name: \"Vector 2\"\n"
        "    vector2 {\n"
        "        x: 2.0\n"
        "        y: 1.0\n"
        "    }\n"
        "}\n"
        "property {\n"
        "    uniform_name: \"Float\"\n"
        "    display_name: \"Float\"\n"
        "    scalar_float: 3.14\n"
        "}\n"
        "property {\n"
        "    uniform_name: \"Uint\"\n"
        "    display_name: \"Unsigned Integer\"\n"
        "    scalar_uint: 4\n"
        "}\n"
        "property {\n"
        "    uniform_name: \"Sint\"\n"
        "    display_name: \"Signed Integer\"\n"
        "    scalar_int: -4\n"
        "}\n"
    };

    TEST ( Material, Initialization )
    {
        MaterialMsg material_buffer =
            LoadProtoBufObject<MaterialMsg> ( material_text, sizeof ( material_text ) - 1, "AEONMTL" );
        Material material ( material_buffer );
        EXPECT_TRUE ( material.GetProperties() [0].GetName() == "Vector4" );
        EXPECT_TRUE ( material.GetProperties() [1].GetName() == "Vector3" );
        EXPECT_TRUE ( material.GetProperties() [2].GetName() == "Vector2" );
        EXPECT_TRUE ( material.GetProperties() [3].GetName() == "Float" );
        EXPECT_TRUE ( material.GetProperties() [4].GetName() == "Uint" );
        EXPECT_TRUE ( material.GetProperties() [5].GetName() == "Sint" );
        for ( auto& i : material.GetProperties() )
        {
            switch ( i.GetType() )
            {
            case Material::PropertyType::UINT:
            {
                EXPECT_EQ ( i.GetUint(), 4 );
            }
            break;
            case Material::PropertyType::FLOAT:
            {
                EXPECT_EQ ( i.GetFloat(), 3.14f );
            }
            break;
            case Material::PropertyType::SINT:
            {
                EXPECT_EQ ( i.GetSint(), -4 );
            }
            break;
            case Material::PropertyType::FLOAT_VEC2:
            {
                EXPECT_EQ ( i.GetVector2(), Vector2 ( 2.0f, 1.0f ) );
            }
            break;
            case Material::PropertyType::FLOAT_VEC3:
            {
                EXPECT_EQ ( i.GetVector3(), Vector3 ( 3.0f, 2.0f, 1.0f ) );
            }
            break;
            case Material::PropertyType::FLOAT_VEC4:
            {
                EXPECT_EQ ( i.GetVector4(), Vector4 ( 4.0f, 3.0f, 2.0f, 1.0f ) );
            }
            break;
            default:
                break;
            }
        }
    }
    TEST ( Material, CopyConstruction )
    {
        MaterialMsg material_buffer =
            LoadProtoBufObject<MaterialMsg> ( material_text, sizeof ( material_text ) - 1, "AEONMTL" );
        Material original_material ( material_buffer );
        Material material ( original_material );
        EXPECT_TRUE ( material.GetProperties() [0].GetName() == "Vector4" );
        EXPECT_TRUE ( material.GetProperties() [1].GetName() == "Vector3" );
        EXPECT_TRUE ( material.GetProperties() [2].GetName() == "Vector2" );
        EXPECT_TRUE ( material.GetProperties() [3].GetName() == "Float" );
        EXPECT_TRUE ( material.GetProperties() [4].GetName() == "Uint" );
        EXPECT_TRUE ( material.GetProperties() [5].GetName() == "Sint" );
        for ( auto& i : material.GetProperties() )
        {
            switch ( i.GetType() )
            {
            case Material::PropertyType::UINT:
            {
                EXPECT_EQ ( i.GetUint(), 4 );
            }
            break;
            case Material::PropertyType::FLOAT:
            {
                EXPECT_EQ ( i.GetFloat(), 3.14f );
            }
            break;
            case Material::PropertyType::SINT:
            {
                EXPECT_EQ ( i.GetSint(), -4 );
            }
            break;
            case Material::PropertyType::FLOAT_VEC2:
            {
                EXPECT_EQ ( i.GetVector2(), Vector2 ( 2.0f, 1.0f ) );
            }
            break;
            case Material::PropertyType::FLOAT_VEC3:
            {
                EXPECT_EQ ( i.GetVector3(), Vector3 ( 3.0f, 2.0f, 1.0f ) );
            }
            break;
            case Material::PropertyType::FLOAT_VEC4:
            {
                EXPECT_EQ ( i.GetVector4(), Vector4 ( 4.0f, 3.0f, 2.0f, 1.0f ) );
            }
            break;
            default:
                break;
            }
        }
    }
    TEST ( Material, CopyOperator )
    {
        MaterialMsg material_buffer =
            LoadProtoBufObject<MaterialMsg> ( material_text, sizeof ( material_text ) - 1, "AEONMTL" );
        Material original_material ( material_buffer );
        Material material;
        material = original_material;
        EXPECT_TRUE ( material.GetProperties() [0].GetName() == "Vector4" );
        EXPECT_TRUE ( material.GetProperties() [1].GetName() == "Vector3" );
        EXPECT_TRUE ( material.GetProperties() [2].GetName() == "Vector2" );
        EXPECT_TRUE ( material.GetProperties() [3].GetName() == "Float" );
        EXPECT_TRUE ( material.GetProperties() [4].GetName() == "Uint" );
        EXPECT_TRUE ( material.GetProperties() [5].GetName() == "Sint" );
        for ( auto& i : material.GetProperties() )
        {
            switch ( i.GetType() )
            {
            case Material::PropertyType::UINT:
            {
                EXPECT_EQ ( i.GetUint(), 4 );
            }
            break;
            case Material::PropertyType::FLOAT:
            {
                EXPECT_EQ ( i.GetFloat(), 3.14f );
            }
            break;
            case Material::PropertyType::SINT:
            {
                EXPECT_EQ ( i.GetSint(), -4 );
            }
            break;
            case Material::PropertyType::FLOAT_VEC2:
            {
                EXPECT_EQ ( i.GetVector2(), Vector2 ( 2.0f, 1.0f ) );
            }
            break;
            case Material::PropertyType::FLOAT_VEC3:
            {
                EXPECT_EQ ( i.GetVector3(), Vector3 ( 3.0f, 2.0f, 1.0f ) );
            }
            break;
            case Material::PropertyType::FLOAT_VEC4:
            {
                EXPECT_EQ ( i.GetVector4(), Vector4 ( 4.0f, 3.0f, 2.0f, 1.0f ) );
            }
            break;
            default:
                break;
            }
        }
    }
}
