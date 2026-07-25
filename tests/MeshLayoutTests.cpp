/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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

#include "gtest/gtest.h"
#include "aeongames/Mesh.hpp"
#include "aeongames/ProtoBufClasses.hpp"
#include "mesh.pb.h"

namespace AeonGames
{
    TEST ( MeshLayoutTests, LegacyAttributesRemainPacked )
    {
        MeshMsg message;
        auto* position = message.add_attribute();
        position->set_semantic ( AttributeMsg_AttributeSemantic_POSITION );
        position->set_size ( 3 );
        position->set_type ( AttributeMsg_AttributeType_FLOAT );
        auto* uv = message.add_attribute();
        uv->set_semantic ( AttributeMsg_AttributeSemantic_TEXCOORD );
        uv->set_size ( 2 );
        uv->set_type ( AttributeMsg_AttributeType_FLOAT );

        Mesh mesh;
        mesh.LoadFromPBMsg ( message );

        ASSERT_EQ ( mesh.GetAttributes().size(), 2u );
        EXPECT_EQ ( mesh.GetAttributeOffset ( mesh.GetAttributes() [0] ), 0u );
        EXPECT_EQ ( mesh.GetAttributeOffset ( mesh.GetAttributes() [1] ), 12u );
        EXPECT_EQ ( mesh.GetStride(), 20u );
    }

    TEST ( MeshLayoutTests, LoadsExplicitOffsetsAndStride )
    {
        MeshMsg message;
        auto* position = message.add_attribute();
        position->set_semantic ( AttributeMsg_AttributeSemantic_POSITION );
        position->set_size ( 3 );
        position->set_type ( AttributeMsg_AttributeType_FLOAT );
        position->set_offset ( 16 );
        auto* color = message.add_attribute();
        color->set_semantic ( AttributeMsg_AttributeSemantic_COLOR );
        color->set_size ( 4 );
        color->set_type ( AttributeMsg_AttributeType_UNSIGNED_BYTE );
        color->set_offset ( 0 );
        message.set_vertexstride ( 32 );

        Mesh mesh;
        mesh.LoadFromPBMsg ( message );

        ASSERT_EQ ( mesh.GetAttributes().size(), 2u );
        EXPECT_EQ ( mesh.GetAttributeOffset ( mesh.GetAttributes() [0] ), 16u );
        EXPECT_EQ ( mesh.GetAttributeOffset ( mesh.GetAttributes() [1] ), 0u );
        EXPECT_EQ ( mesh.GetStride(), 32u );
    }
}