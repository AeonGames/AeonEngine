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

// collision.pb.h is included first so its generated near()/far() accessors are
// parsed before any platform header (pulled in transitively by gtest) can
// define the legacy empty "near"/"far" macros.
#include "aeongames/ProtoBufClasses.hpp"
// Undefine the legacy empty "near"/"far" macros (pulled in transitively by the
// platform headers) BEFORE collision.pb.h is parsed, otherwise they clobber the
// generated near()/far() accessors.
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#include "collision.pb.h"
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#include <string>
#include <cstdint>
#include "aeongames/Collision.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Plane.hpp"
#include "aeongames/Vector3.hpp"
#include "gtest/gtest.h"

using namespace ::testing;

namespace AeonGames
{
    namespace
    {
        /** @brief Serialize a CollisionMsg into a .cln buffer and load it. */
        void LoadCollision ( Collision& aCollision, const CollisionMsg& aMsg )
        {
            std::string buffer{ "AEONCLN" };
            buffer.push_back ( '\0' ); // binary marker; 8-byte magic total.
            std::string serialized;
            ASSERT_TRUE ( aMsg.SerializeToString ( &serialized ) );
            buffer += serialized;
            aCollision.LoadFromMemory ( buffer.data(), buffer.size() );
        }

        /** @brief Encode int32 values as a protobuf bytes field. */
        std::string PackIndices ( const int32_t* aValues, size_t aCount )
        {
            return std::string ( reinterpret_cast<const char*> ( aValues ), aCount * sizeof ( int32_t ) );
        }

        /** @brief Build a single axis-aligned cube brush spanning [-1,1] on each axis,
         *  stored in a degenerate Kd-tree of one leaf. */
        CollisionMsg MakeUnitCube()
        {
            CollisionMsg msg;
            msg.mutable_center()->set_x ( 0.0f );
            msg.mutable_center()->set_y ( 0.0f );
            msg.mutable_center()->set_z ( 0.0f );
            msg.mutable_radii()->set_x ( 1.0f );
            msg.mutable_radii()->set_y ( 1.0f );
            msg.mutable_radii()->set_z ( 1.0f );

            auto* brush = msg.add_brush();
            brush->mutable_sixdop()->mutable_positive()->set_x ( 1.0f );
            brush->mutable_sixdop()->mutable_positive()->set_y ( 1.0f );
            brush->mutable_sixdop()->mutable_positive()->set_z ( 1.0f );
            brush->mutable_sixdop()->mutable_negative()->set_x ( 1.0f );
            brush->mutable_sixdop()->mutable_negative()->set_y ( 1.0f );
            brush->mutable_sixdop()->mutable_negative()->set_z ( 1.0f );
            brush->set_planestart ( 0 );
            brush->set_planecount ( 0 );

            auto* leaf = msg.add_kdleaf();
            leaf->set_brushstart ( 0 );
            leaf->set_brushcount ( 1 );

            const int32_t brush_indices[] = { 0 };
            msg.set_brushindices ( PackIndices ( brush_indices, 1 ) );
            return msg;
        }
    }

    class CollisionTest : public ::testing::Test {};

    // A ray dropping straight down onto the top face stops at the +Y plane.
    TEST_F ( CollisionTest, RayCastHitsTopFace )
    {
        Collision collision;
        LoadCollision ( collision, MakeUnitCube() );

        Plane contact;
        const float fraction = collision.RayCast ( Vector3{ 0.0f, 5.0f, 0.0f }, Vector3{ 0.0f, -5.0f, 0.0f }, &contact );
        EXPECT_NEAR ( fraction, 0.8f, 1e-5f );
        EXPECT_NEAR ( contact.GetNormal() [0], 0.0f, 1e-5f );
        EXPECT_NEAR ( contact.GetNormal() [1], 1.0f, 1e-5f );
        EXPECT_NEAR ( contact.GetNormal() [2], 0.0f, 1e-5f );
    }

    // A ray that travels alongside the cube without crossing it reports a miss.
    TEST_F ( CollisionTest, RayCastMisses )
    {
        Collision collision;
        LoadCollision ( collision, MakeUnitCube() );

        const float fraction = collision.RayCast ( Vector3{ 5.0f, 5.0f, 5.0f }, Vector3{ 0.0f, 0.0f, -10.0f } );
        EXPECT_NEAR ( fraction, 1.0f, 1e-5f );
    }

    // Sweeping a box stops earlier than a ray because of its half-extents.
    TEST_F ( CollisionTest, SweepBoxStopsBeforeRay )
    {
        Collision collision;
        LoadCollision ( collision, MakeUnitCube() );

        const float fraction = collision.Sweep ( AABB{ Vector3{ 0.0f, 5.0f, 0.0f }, Vector3{ 0.5f, 0.5f, 0.5f } }, Vector3{ 0.0f, -5.0f, 0.0f } );
        // Top face at y=1 expanded outward by 0.5 -> contact at y=1.5: (5-1.5)/5.
        EXPECT_NEAR ( fraction, 0.7f, 1e-5f );
    }

    // Overlap is true inside the cube and false well outside it.
    TEST_F ( CollisionTest, OverlapInsideAndOutside )
    {
        Collision collision;
        LoadCollision ( collision, MakeUnitCube() );

        EXPECT_TRUE ( collision.Overlap ( AABB{ Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.1f, 0.1f, 0.1f } } ) );
        EXPECT_FALSE ( collision.Overlap ( AABB{ Vector3{ 5.0f, 0.0f, 0.0f }, Vector3{ 0.1f, 0.1f, 0.1f } } ) );
    }

    // Two brushes split by a Kd-node: the ray must hit the nearer brush first.
    TEST_F ( CollisionTest, KdTreeTraversalHitsNearestBrush )
    {
        CollisionMsg msg;
        msg.mutable_center()->set_x ( 0.0f );
        msg.mutable_center()->set_y ( 0.0f );
        msg.mutable_center()->set_z ( 0.0f );
        msg.mutable_radii()->set_x ( 3.0f );
        msg.mutable_radii()->set_y ( 1.0f );
        msg.mutable_radii()->set_z ( 1.0f );

        // Brush A: x in [-3,-1].
        auto* brush_a = msg.add_brush();
        brush_a->mutable_sixdop()->mutable_positive()->set_x ( -1.0f );
        brush_a->mutable_sixdop()->mutable_positive()->set_y ( 1.0f );
        brush_a->mutable_sixdop()->mutable_positive()->set_z ( 1.0f );
        brush_a->mutable_sixdop()->mutable_negative()->set_x ( 3.0f );
        brush_a->mutable_sixdop()->mutable_negative()->set_y ( 1.0f );
        brush_a->mutable_sixdop()->mutable_negative()->set_z ( 1.0f );
        brush_a->set_planestart ( 0 );
        brush_a->set_planecount ( 0 );

        // Brush B: x in [1,3].
        auto* brush_b = msg.add_brush();
        brush_b->mutable_sixdop()->mutable_positive()->set_x ( 3.0f );
        brush_b->mutable_sixdop()->mutable_positive()->set_y ( 1.0f );
        brush_b->mutable_sixdop()->mutable_positive()->set_z ( 1.0f );
        brush_b->mutable_sixdop()->mutable_negative()->set_x ( -1.0f );
        brush_b->mutable_sixdop()->mutable_negative()->set_y ( 1.0f );
        brush_b->mutable_sixdop()->mutable_negative()->set_z ( 1.0f );
        brush_b->set_planestart ( 0 );
        brush_b->set_planecount ( 0 );

        auto* leaf_near = msg.add_kdleaf();
        leaf_near->set_brushstart ( 0 );
        leaf_near->set_brushcount ( 1 );
        auto* leaf_far = msg.add_kdleaf();
        leaf_far->set_brushstart ( 1 );
        leaf_far->set_brushcount ( 1 );

        auto* node = msg.add_kdnode();
        node->set_axis ( 0 );
        node->set_distance ( 0.0f );
        node->set_near ( -1 ); // leaf 0 (brush A, x<0)
        node->set_far ( -2 );  // leaf 1 (brush B, x>0)

        const int32_t brush_indices[] = { 0, 1 };
        msg.set_brushindices ( PackIndices ( brush_indices, 2 ) );

        Collision collision;
        LoadCollision ( collision, msg );

        // Travel from +X toward -X; brush B (entry x=3) is hit first.
        Plane contact;
        const float fraction = collision.RayCast ( Vector3{ 5.0f, 0.0f, 0.0f }, Vector3{ -10.0f, 0.0f, 0.0f }, &contact );
        EXPECT_NEAR ( fraction, 0.2f, 1e-5f );
        EXPECT_NEAR ( contact.GetNormal() [0], 1.0f, 1e-5f );
    }
}
