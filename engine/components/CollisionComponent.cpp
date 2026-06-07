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
#include <array>
#include <algorithm>
#include <filesystem>
#include <variant>
#include "aeongames/Node.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Collision.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Matrix3x3.hpp"
#include "aeongames/Quaternion.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Vector3.hpp"
#include "CollisionComponent.hpp"

namespace AeonGames
{
    static const StringId CollisionStringId{"Collision Component"};

    const StringId& CollisionComponent::GetClassId()
    {
        return CollisionStringId;
    }

    CollisionComponent::CollisionComponent() = default;
    CollisionComponent::~CollisionComponent() = default;

    const StringId& CollisionComponent::GetId() const
    {
        return CollisionStringId;
    }

    static constexpr std::array<const StringId, 1> CollisionComponentPropertyIds
    {
        {
            {"Collision"},
        }
    };

    size_t CollisionComponent::GetPropertyCount () const
    {
        return CollisionComponentPropertyIds.size();
    }

    const StringId* CollisionComponent::GetPropertyInfoArray () const
    {
        return CollisionComponentPropertyIds.data();
    }

    Property CollisionComponent::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case CollisionComponentPropertyIds[0]:
            return GetCollision().GetPathString();
        }
        return Property{};
    }

    void CollisionComponent::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        switch ( aId )
        {
        case CollisionComponentPropertyIds[0]:
            if ( std::holds_alternative<std::string> ( aProperty ) )
            {
                SetCollision ( {"Collision"_crc32, std::get<std::string> ( aProperty ) } );
            }
            else if ( std::holds_alternative<std::filesystem::path> ( aProperty ) )
            {
                SetCollision ( {"Collision"_crc32, std::get<std::filesystem::path> ( aProperty ).string() } );
            }
            else if ( std::holds_alternative<uint32_t> ( aProperty ) )
            {
                SetCollision ( {"Collision"_crc32, std::get<uint32_t> ( aProperty ) } );
            }
            break;
        }
    }

    void CollisionComponent::SetCollision ( const ResourceId& aCollision )
    {
        mCollision = aCollision;
        mCollision.Store();
    }

    const ResourceId& CollisionComponent::GetCollision() const noexcept
    {
        return mCollision;
    }

    void CollisionComponent::Update ( Node& aNode, double aDelta )
    {
        ( void ) aDelta;
        // Publish the collision geometry's local-space bounds as the node AABB
        // so the scene-wide query helpers can broad-phase cull this node.
        if ( auto collision = mCollision.Cast<Collision>() )
        {
            aNode.SetAABB ( collision->GetAABB() );
        }
    }

    void CollisionComponent::Render ( const Node& /*aNode*/, Renderer& /*aRenderer*/, void* /*aWindowId*/ )
    {
    }

    void CollisionComponent::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ )
    {
    }

    namespace
    {
        // Transform a world-space contact plane returned by the collision
        // resource back into world space given the node's global transform.
        // Normals are rotated (assuming no or uniform scale) and renormalized;
        // the plane distance is recomputed from a point lying on the plane.
        Plane LocalPlaneToWorld ( const Transform& aGlobal, const Plane& aLocalPlane )
        {
            const Vector3& local_normal = aLocalPlane.GetNormal();
            const float local_distance = aLocalPlane.GetDistance();
            Vector3 world_normal = Normalize ( aGlobal.GetRotation() * local_normal );
            // A point on the local plane (unit normal): n * d.
            Vector3 world_point = aGlobal.GetMatrix() * ( local_normal * local_distance );
            float world_distance = Dot ( world_normal, world_point );
            return Plane{ world_normal.GetX(), world_normal.GetY(), world_normal.GetZ(), world_distance };
        }
    }

    float CollisionComponent::Sweep ( const Node& aNode, const AABB& aBox, const Vector3& aDisplacement, Plane* aContactPlane ) const
    {
        auto collision = mCollision.Cast<Collision>();
        if ( !collision )
        {
            return 1.0f;
        }
        const Transform& global = aNode.GetGlobalTransform();
        const Transform inverse = global.GetInverted();
        // The query box transforms as an AABB (center as a point, half-extents
        // enclosing the rotated/scaled box); the displacement is a direction so
        // it uses the scale+rotation part only (no translation).
        AABB local_box = inverse * aBox;
        Vector3 local_displacement = Matrix3x3{ inverse } * aDisplacement;

        Plane local_plane{};
        float fraction = collision->Sweep ( local_box, local_displacement, &local_plane );
        if ( fraction < 1.0f && aContactPlane )
        {
            *aContactPlane = LocalPlaneToWorld ( global, local_plane );
        }
        return fraction;
    }

    float CollisionComponent::RayCast ( const Node& aNode, const Vector3& aOrigin, const Vector3& aDirection, Plane* aContactPlane ) const
    {
        return Sweep ( aNode, AABB{ aOrigin, Vector3{ 0.0f, 0.0f, 0.0f } }, aDirection, aContactPlane );
    }

    bool CollisionComponent::Overlap ( const Node& aNode, const AABB& aBox ) const
    {
        auto collision = mCollision.Cast<Collision>();
        if ( !collision )
        {
            return false;
        }
        const Transform inverse = aNode.GetGlobalTransform().GetInverted();
        return collision->Overlap ( inverse * aBox );
    }

    namespace
    {
        Vector3 ComponentMin ( const Vector3& a, const Vector3& b )
        {
            return Vector3{ std::min ( a.GetX(), b.GetX() ), std::min ( a.GetY(), b.GetY() ), std::min ( a.GetZ(), b.GetZ() ) };
        }
        Vector3 ComponentMax ( const Vector3& a, const Vector3& b )
        {
            return Vector3{ std::max ( a.GetX(), b.GetX() ), std::max ( a.GetY(), b.GetY() ), std::max ( a.GetZ(), b.GetZ() ) };
        }
    }

    float CollisionComponent::Sweep ( Scene& aScene, const AABB& aBox, const Vector3& aDisplacement, Plane* aContactPlane, Node** aHitNode )
    {
        // Conservative world-space bounds of the swept query box, used to
        // broad-phase cull colliders whose world AABB cannot be touched.
        const Vector3& center = aBox.GetCenter();
        const Vector3& radii = aBox.GetRadii();
        Vector3 endpoint = center + aDisplacement;
        Vector3 query_min = ComponentMin ( center, endpoint ) - radii;
        Vector3 query_max = ComponentMax ( center, endpoint ) + radii;
        const AABB query_box { ( query_min + query_max ) * 0.5f, ( query_max - query_min ) * 0.5f };

        float nearest = 1.0f;
        aScene.QueryAABB ( query_box, [&] ( const Node & node )
        {
            Component* component = node.GetComponent ( CollisionComponent::GetClassId() );
            if ( !component )
            {
                return;
            }
            auto* collider = static_cast<CollisionComponent*> ( component );
            Plane plane{};
            float fraction = collider->Sweep ( node, aBox, aDisplacement, &plane );
            if ( fraction < nearest )
            {
                nearest = fraction;
                if ( aContactPlane )
                {
                    *aContactPlane = plane;
                }
                if ( aHitNode )
                {
                    *aHitNode = const_cast<Node*> ( &node );
                }
            }
        } );
        return nearest;
    }

    float CollisionComponent::RayCast ( Scene& aScene, const Vector3& aOrigin, const Vector3& aDirection, Plane* aContactPlane, Node** aHitNode )
    {
        return Sweep ( aScene, AABB{ aOrigin, Vector3{ 0.0f, 0.0f, 0.0f } }, aDirection, aContactPlane, aHitNode );
    }

    bool CollisionComponent::Overlap ( Scene& aScene, const AABB& aBox, Node** aHitNode )
    {
        bool overlapped = false;
        aScene.QueryAABB ( aBox, [&] ( const Node & node )
        {
            if ( overlapped )
            {
                return;
            }
            Component* component = node.GetComponent ( CollisionComponent::GetClassId() );
            if ( !component )
            {
                return;
            }
            auto* collider = static_cast<CollisionComponent*> ( component );
            if ( collider->Overlap ( node, aBox ) )
            {
                overlapped = true;
                if ( aHitNode )
                {
                    *aHitNode = const_cast<Node*> ( &node );
                }
            }
        } );
        return overlapped;
    }
}
