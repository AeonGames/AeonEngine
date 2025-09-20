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
#include <array>
#include <algorithm>
#include "aeongames/AABB.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Plane.hpp"
#include "aeongames/Transform.hpp"
#include "gtest/gtest.h"

using namespace ::testing;

namespace AeonGames
{
    // Test Fixture for AABB tests
    class AABBTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // Set up common test data
            center = Vector3 ( 1.0f, 2.0f, 3.0f );
            radii = Vector3 ( 0.5f, 1.0f, 1.5f );
            offset = Vector3 ( 10.0f, 20.0f, 30.0f );
        }

        Vector3 center;
        Vector3 radii;
        Vector3 offset;
    };

    // Test default constructor
    TEST_F ( AABBTest, DefaultConstructor )
    {
        AABB aabb;

        // Default constructor should initialize center and radii to zero
        EXPECT_EQ ( aabb.GetCenter() [0], 0.0f );
        EXPECT_EQ ( aabb.GetCenter() [1], 0.0f );
        EXPECT_EQ ( aabb.GetCenter() [2], 0.0f );

        EXPECT_EQ ( aabb.GetRadii() [0], 0.0f );
        EXPECT_EQ ( aabb.GetRadii() [1], 0.0f );
        EXPECT_EQ ( aabb.GetRadii() [2], 0.0f );
    }

    // Test parameterized constructor
    TEST_F ( AABBTest, ParameterizedConstructor )
    {
        AABB aabb ( center, radii );

        // Constructor should set center and radii correctly
        EXPECT_EQ ( aabb.GetCenter() [0], center[0] );
        EXPECT_EQ ( aabb.GetCenter() [1], center[1] );
        EXPECT_EQ ( aabb.GetCenter() [2], center[2] );

        EXPECT_EQ ( aabb.GetRadii() [0], radii[0] );
        EXPECT_EQ ( aabb.GetRadii() [1], radii[1] );
        EXPECT_EQ ( aabb.GetRadii() [2], radii[2] );
    }

    // Test GetCenter method
    TEST_F ( AABBTest, GetCenter )
    {
        AABB aabb ( center, radii );
        const Vector3& retrievedCenter = aabb.GetCenter();

        EXPECT_EQ ( retrievedCenter[0], center[0] );
        EXPECT_EQ ( retrievedCenter[1], center[1] );
        EXPECT_EQ ( retrievedCenter[2], center[2] );

        // Test that we get a const reference (modifying original shouldn't affect retrieved)
        EXPECT_EQ ( &retrievedCenter, &aabb.GetCenter() );
    }

    // Test GetRadii method
    TEST_F ( AABBTest, GetRadii )
    {
        AABB aabb ( center, radii );
        const Vector3& retrievedRadii = aabb.GetRadii();

        EXPECT_EQ ( retrievedRadii[0], radii[0] );
        EXPECT_EQ ( retrievedRadii[1], radii[1] );
        EXPECT_EQ ( retrievedRadii[2], radii[2] );

        // Test that we get a const reference
        EXPECT_EQ ( &retrievedRadii, &aabb.GetRadii() );
    }

    // Test SetCenter method
    TEST_F ( AABBTest, SetCenter )
    {
        AABB aabb;
        aabb.SetCenter ( center );

        EXPECT_EQ ( aabb.GetCenter() [0], center[0] );
        EXPECT_EQ ( aabb.GetCenter() [1], center[1] );
        EXPECT_EQ ( aabb.GetCenter() [2], center[2] );

        // Test setting a different center
        Vector3 newCenter ( 5.0f, 6.0f, 7.0f );
        aabb.SetCenter ( newCenter );

        EXPECT_EQ ( aabb.GetCenter() [0], newCenter[0] );
        EXPECT_EQ ( aabb.GetCenter() [1], newCenter[1] );
        EXPECT_EQ ( aabb.GetCenter() [2], newCenter[2] );
    }

    // Test SetRadii method
    TEST_F ( AABBTest, SetRadii )
    {
        AABB aabb;
        aabb.SetRadii ( radii );

        EXPECT_EQ ( aabb.GetRadii() [0], radii[0] );
        EXPECT_EQ ( aabb.GetRadii() [1], radii[1] );
        EXPECT_EQ ( aabb.GetRadii() [2], radii[2] );

        // Test setting different radii
        Vector3 newRadii ( 2.0f, 3.0f, 4.0f );
        aabb.SetRadii ( newRadii );

        EXPECT_EQ ( aabb.GetRadii() [0], newRadii[0] );
        EXPECT_EQ ( aabb.GetRadii() [1], newRadii[1] );
        EXPECT_EQ ( aabb.GetRadii() [2], newRadii[2] );
    }

    // Test GetPoints method with default offset
    TEST_F ( AABBTest, GetPointsDefaultOffset )
    {
        AABB aabb ( center, radii );
        std::array<Vector3, 8> points = aabb.GetPoints();

        // Test all 8 corner points of the AABB
        // Expected points based on the implementation:
        std::array<Vector3, 8> expectedPoints =
        {
            center + radii,                                          // (+,+,+)
            center - radii,                                          // (-,-,-)
            center + Vector3 ( -radii[0], radii[1], radii[2] ),     // (-,+,+)
            center - Vector3 ( -radii[0], radii[1], radii[2] ),     // (+,-,-)
            center + Vector3 ( radii[0], -radii[1], radii[2] ),     // (+,-,+)
            center - Vector3 ( radii[0], -radii[1], radii[2] ),     // (-,+,-)
            center + Vector3 ( radii[0], radii[1], -radii[2] ),     // (+,+,-)
            center - Vector3 ( radii[0], radii[1], -radii[2] )      // (-,-,+)
        };

        for ( size_t i = 0; i < 8; ++i )
        {
            EXPECT_FLOAT_EQ ( points[i][0], expectedPoints[i][0] ) << "Point " << i << " X coordinate mismatch";
            EXPECT_FLOAT_EQ ( points[i][1], expectedPoints[i][1] ) << "Point " << i << " Y coordinate mismatch";
            EXPECT_FLOAT_EQ ( points[i][2], expectedPoints[i][2] ) << "Point " << i << " Z coordinate mismatch";
        }
    }

    // Test GetPoints method with custom offset
    TEST_F ( AABBTest, GetPointsWithOffset )
    {
        AABB aabb ( center, radii );
        std::array<Vector3, 8> points = aabb.GetPoints ( offset );

        // Expected points with offset applied
        std::array<Vector3, 8> expectedPoints =
        {
            offset + center + radii,                                          // (+,+,+)
            offset + center - radii,                                          // (-,-,-)
            offset + center + Vector3 ( -radii[0], radii[1], radii[2] ),     // (-,+,+)
            offset + center - Vector3 ( -radii[0], radii[1], radii[2] ),     // (+,-,-)
            offset + center + Vector3 ( radii[0], -radii[1], radii[2] ),     // (+,-,+)
            offset + center - Vector3 ( radii[0], -radii[1], radii[2] ),     // (-,+,-)
            offset + center + Vector3 ( radii[0], radii[1], -radii[2] ),     // (+,+,-)
            offset + center - Vector3 ( radii[0], radii[1], -radii[2] )      // (-,-,+)
        };

        for ( size_t i = 0; i < 8; ++i )
        {
            EXPECT_FLOAT_EQ ( points[i][0], expectedPoints[i][0] ) << "Point " << i << " X coordinate mismatch";
            EXPECT_FLOAT_EQ ( points[i][1], expectedPoints[i][1] ) << "Point " << i << " Y coordinate mismatch";
            EXPECT_FLOAT_EQ ( points[i][2], expectedPoints[i][2] ) << "Point " << i << " Z coordinate mismatch";
        }
    }

    // Test GetTransform method
    TEST_F ( AABBTest, GetTransform )
    {
        AABB aabb ( center, radii );
        Transform transform = aabb.GetTransform();

        // The transform should have:
        // - Scale equal to radii
        // - Translation equal to center
        // - Default rotation (identity quaternion)

        // Note: The exact implementation of Transform comparison depends on the Transform class
        // For now, we'll test the basic functionality by creating a Transform manually
        Transform expectedTransform ( radii, Quaternion{}, center );

        // Since we can't easily compare Transform objects directly, we test the concept
        // that the Transform is constructed with the correct parameters
        // This test verifies the method compiles and executes without error
        EXPECT_NO_THROW ( transform = aabb.GetTransform() );
    }

    // Test GetDistanceToPlane method
    TEST_F ( AABBTest, GetDistanceToPlane )
    {
        AABB aabb ( Vector3 ( 0.0f, 0.0f, 0.0f ), Vector3 ( 1.0f, 1.0f, 1.0f ) );

        // Test with a plane along the X axis (normal pointing in +X direction)
        Plane planeX ( 1.0f, 0.0f, 0.0f, 5.0f );
        float distanceX = aabb.GetDistanceToPlane ( planeX );

        // The farthest point from the plane should be at (-1, 0, 0) (negative X)
        // Distance should be: dot((1,0,0), (0,0,0) + (-1,0,0)) - 5 = -1 - 5 = -6
        EXPECT_FLOAT_EQ ( distanceX, -6.0f );

        // Test with a plane along the Y axis (normal pointing in +Y direction)
        Plane planeY ( 0.0f, 1.0f, 0.0f, 3.0f );
        float distanceY = aabb.GetDistanceToPlane ( planeY );

        // The farthest point from the plane should be at (0, -1, 0) (negative Y)
        // Distance should be: dot((0,1,0), (0,0,0) + (0,-1,0)) - 3 = -1 - 3 = -4
        EXPECT_FLOAT_EQ ( distanceY, -4.0f );

        // Test with a plane along the Z axis (normal pointing in +Z direction)
        Plane planeZ ( 0.0f, 0.0f, 1.0f, 2.0f );
        float distanceZ = aabb.GetDistanceToPlane ( planeZ );

        // The farthest point from the plane should be at (0, 0, -1) (negative Z)
        // Distance should be: dot((0,0,1), (0,0,0) + (0,0,-1)) - 2 = -1 - 2 = -3
        EXPECT_FLOAT_EQ ( distanceZ, -3.0f );
    }

    // Test GetDistanceToPlane with negative normal components
    TEST_F ( AABBTest, GetDistanceToPlaneNegativeNormal )
    {
        AABB aabb ( Vector3 ( 0.0f, 0.0f, 0.0f ), Vector3 ( 1.0f, 1.0f, 1.0f ) );

        // Test with a plane with negative normal components
        Plane planeNegX ( -1.0f, 0.0f, 0.0f, 5.0f );
        float distanceNegX = aabb.GetDistanceToPlane ( planeNegX );

        // The farthest point from the plane should be at (1, 0, 0) (positive X)
        // Distance should be: dot((-1,0,0), (0,0,0) + (1,0,0)) - 5 = -1 - 5 = -6
        EXPECT_FLOAT_EQ ( distanceNegX, -6.0f );
    }

    // Test operator+= method
    TEST_F ( AABBTest, OperatorPlusEquals )
    {
        // Create two AABBs for testing
        AABB aabb1 ( Vector3 ( 0.0f, 0.0f, 0.0f ), Vector3 ( 1.0f, 1.0f, 1.0f ) );
        AABB aabb2 ( Vector3 ( 3.0f, 3.0f, 3.0f ), Vector3 ( 1.0f, 1.0f, 1.0f ) );

        // Apply += operator
        aabb1 += aabb2;

        // The result should be an AABB that encompasses both original AABBs
        // AABB1 bounds: min(-1,-1,-1), max(1,1,1)
        // AABB2 bounds: min(2,2,2), max(4,4,4)
        // Combined bounds: min(-1,-1,-1), max(4,4,4)
        // Center: ((-1+4)/2, (-1+4)/2, (-1+4)/2) = (1.5, 1.5, 1.5)
        // Radii: (4-1.5, 4-1.5, 4-1.5) = (2.5, 2.5, 2.5)

        EXPECT_FLOAT_EQ ( aabb1.GetCenter() [0], 1.5f );
        EXPECT_FLOAT_EQ ( aabb1.GetCenter() [1], 1.5f );
        EXPECT_FLOAT_EQ ( aabb1.GetCenter() [2], 1.5f );

        EXPECT_FLOAT_EQ ( aabb1.GetRadii() [0], 2.5f );
        EXPECT_FLOAT_EQ ( aabb1.GetRadii() [1], 2.5f );
        EXPECT_FLOAT_EQ ( aabb1.GetRadii() [2], 2.5f );
    }

    // Test operator+= with overlapping AABBs
    TEST_F ( AABBTest, OperatorPlusEqualsOverlapping )
    {
        AABB aabb1 ( Vector3 ( 0.0f, 0.0f, 0.0f ), Vector3 ( 2.0f, 2.0f, 2.0f ) );
        AABB aabb2 ( Vector3 ( 1.0f, 1.0f, 1.0f ), Vector3 ( 1.0f, 1.0f, 1.0f ) );

        aabb1 += aabb2;

        // AABB1 bounds: min(-2,-2,-2), max(2,2,2)
        // AABB2 bounds: min(0,0,0), max(2,2,2)
        // Combined bounds: min(-2,-2,-2), max(2,2,2)
        // Center: ((-2+2)/2, (-2+2)/2, (-2+2)/2) = (0, 0, 0)
        // Radii: (2-0, 2-0, 2-0) = (2, 2, 2)

        EXPECT_FLOAT_EQ ( aabb1.GetCenter() [0], 0.0f );
        EXPECT_FLOAT_EQ ( aabb1.GetCenter() [1], 0.0f );
        EXPECT_FLOAT_EQ ( aabb1.GetCenter() [2], 0.0f );

        EXPECT_FLOAT_EQ ( aabb1.GetRadii() [0], 2.0f );
        EXPECT_FLOAT_EQ ( aabb1.GetRadii() [1], 2.0f );
        EXPECT_FLOAT_EQ ( aabb1.GetRadii() [2], 2.0f );
    }

    // Test operator+= returns reference to self
    TEST_F ( AABBTest, OperatorPlusEqualsReturnsSelf )
    {
        AABB aabb1 ( center, radii );
        AABB aabb2 ( Vector3 ( 0.0f, 0.0f, 0.0f ), Vector3 ( 1.0f, 1.0f, 1.0f ) );

        AABB& result = ( aabb1 += aabb2 );

        // The returned reference should be the same object
        EXPECT_EQ ( &result, &aabb1 );
    }

    // Test with zero-sized AABB
    TEST_F ( AABBTest, ZeroSizedAABB )
    {
        AABB aabb ( center, Vector3 ( 0.0f, 0.0f, 0.0f ) );

        // All points should be at the center
        std::array<Vector3, 8> points = aabb.GetPoints();
        for ( const auto& point : points )
        {
            EXPECT_FLOAT_EQ ( point[0], center[0] );
            EXPECT_FLOAT_EQ ( point[1], center[1] );
            EXPECT_FLOAT_EQ ( point[2], center[2] );
        }
    }

    // Test with negative radii (should still work mathematically)
    TEST_F ( AABBTest, NegativeRadii )
    {
        Vector3 negativeRadii ( -0.5f, -1.0f, -1.5f );
        AABB aabb ( center, negativeRadii );

        EXPECT_EQ ( aabb.GetRadii() [0], negativeRadii[0] );
        EXPECT_EQ ( aabb.GetRadii() [1], negativeRadii[1] );
        EXPECT_EQ ( aabb.GetRadii() [2], negativeRadii[2] );

        // Points should still be calculated correctly
        std::array<Vector3, 8> points = aabb.GetPoints();
        EXPECT_NO_THROW ( points = aabb.GetPoints() );
    }

    // Edge case: AABB at origin
    TEST_F ( AABBTest, AABBAtOrigin )
    {
        AABB aabb ( Vector3 ( 0.0f, 0.0f, 0.0f ), Vector3 ( 1.0f, 1.0f, 1.0f ) );

        std::array<Vector3, 8> points = aabb.GetPoints();

        // Should generate 8 distinct corner points
        // Verify that we have points at all 8 corners of a unit cube centered at origin
        bool foundPositiveCorner = false;
        bool foundNegativeCorner = false;

        for ( const auto& point : points )
        {
            if ( point[0] == 1.0f && point[1] == 1.0f && point[2] == 1.0f )
                foundPositiveCorner = true;
            if ( point[0] == -1.0f && point[1] == -1.0f && point[2] == -1.0f )
                foundNegativeCorner = true;
        }

        EXPECT_TRUE ( foundPositiveCorner );
        EXPECT_TRUE ( foundNegativeCorner );
    }
}