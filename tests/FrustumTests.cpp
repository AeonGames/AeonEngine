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

#include <cstdint>
#include "aeongames/Frustum.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Vector4.hpp"
#include "aeongames/GpuLight.hpp"
#include "gtest/gtest.h"

using namespace ::testing;

namespace AeonGames
{
    // Build a frustum from a perspective view-projection so the planes are a
    // realistic asymmetric set rather than the trivial NDC cube.
    static Frustum MakeFrustum()
    {
        Matrix4x4 projection {};
        projection.Perspective ( 60.0f, 4.0f / 3.0f, 1.0f, 100.0f );
        return Frustum { projection };
    }

    // A point light's culling result must match the result of testing its
    // bounding AABB directly: same centre, radii equal to the falloff radius.
    TEST ( FrustumLight, PointLightMatchesBoundingAABB )
    {
        const Frustum frustum = MakeFrustum();
        for ( const Vector3 centre :
              {
                  Vector3 { 0.0f, 10.0f, 0.0f },   // straight ahead (+Y forward)
                  Vector3 { 0.0f, -10.0f, 0.0f },  // behind the camera
                  Vector3 { 1000.0f, 10.0f, 0.0f } // far off to the side
              } )
        {
            const float radius = 5.0f;
            GpuLight light {};
            light.type = static_cast<uint32_t> ( LightType::Point );
            light.position_radius = Vector4 { centre.GetX(), centre.GetY(), centre.GetZ(), radius };
            const AABB bounds { centre, Vector3 { radius, radius, radius } };
            EXPECT_EQ ( frustum.Intersects ( light ), frustum.Intersects ( bounds ) );
        }
    }

    // A spot light is also bounded by its position_radius sphere and must match
    // its bounding AABB just like a point light.
    TEST ( FrustumLight, SpotLightMatchesBoundingAABB )
    {
        const Frustum frustum = MakeFrustum();
        const Vector3 centre { 0.0f, 20.0f, 0.0f };
        const float radius = 8.0f;
        GpuLight light {};
        light.type = static_cast<uint32_t> ( LightType::Spot );
        light.position_radius = Vector4 { centre.GetX(), centre.GetY(), centre.GetZ(), radius };
        const AABB bounds { centre, Vector3 { radius, radius, radius } };
        EXPECT_EQ ( frustum.Intersects ( light ), frustum.Intersects ( bounds ) );
    }

    // Directional lights have no position or falloff and must never be culled,
    // regardless of where their (unused) position points.
    TEST ( FrustumLight, DirectionalLightIsNeverCulled )
    {
        const Frustum frustum = MakeFrustum();
        GpuLight light {};
        light.type = static_cast<uint32_t> ( LightType::Directional );
        light.position_radius = Vector4 { 1000.0f, 1000.0f, 1000.0f, 0.0f };
        EXPECT_TRUE ( frustum.Intersects ( light ) );
    }

    // Sanity check that the frustum actually discriminates: a light clearly in
    // front is kept while one far off to the side is culled, so the equivalence
    // tests above are not vacuously true. The engine's projection looks down
    // +Y (see Matrix4x4::Frustum), so "in front" is positive Y.
    TEST ( FrustumLight, DiscriminatesInsideFromOutside )
    {
        const Frustum frustum = MakeFrustum();
        GpuLight inside {};
        inside.type = static_cast<uint32_t> ( LightType::Point );
        inside.position_radius = Vector4 { 0.0f, 10.0f, 0.0f, 1.0f };
        GpuLight outside {};
        outside.type = static_cast<uint32_t> ( LightType::Point );
        outside.position_radius = Vector4 { 1000.0f, 10.0f, 0.0f, 1.0f };
        EXPECT_TRUE ( frustum.Intersects ( inside ) );
        EXPECT_FALSE ( frustum.Intersects ( outside ) );
    }
}
