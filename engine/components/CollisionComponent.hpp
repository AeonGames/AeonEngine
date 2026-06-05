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
#ifndef AEONGAMES_COLLISIONCOMPONENT_H
#define AEONGAMES_COLLISIONCOMPONENT_H
#include <string>
#include <vector>
#include "aeongames/Component.hpp"
#include "aeongames/ResourceId.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Plane.hpp"

namespace AeonGames
{
    class Node;
    class Scene;
    /** @brief Attaches static collision geometry (a baked @c .cln resource) to a scene node.
     *
     *  The collision geometry is authored and stored in the node's local space.
     *  Queries are issued in world space; the component transforms them into the
     *  node's local space using the inverse of the node's global transform,
     *  dispatches to the underlying @ref Collision resource, and transforms any
     *  resulting contact plane back into world space.
     *
     *  Each instance also publishes the collision geometry's bounding box as the
     *  node's AABB during Update so the scene-wide query helpers can broad-phase
     *  cull candidate nodes before doing the (more expensive) Kd-tree query. */
    class CollisionComponent final : public Component
    {
    public:
        /** @brief Default constructor. */
        CollisionComponent();
        /** @name Overrides */
        ///@{
        ~CollisionComponent() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, Renderer& aRenderer, void* aWindowId ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}

        /** @name Properties */
        ///@{
        /** @brief Sets the collision resource.
            @param aCollision Resource identifier of the collision geometry. */
        void SetCollision ( const ResourceId& aCollision );
        /** @brief Returns the current collision resource identifier. */
        const ResourceId& GetCollision() const noexcept;
        ///@}

        /** @name Single-node queries (world space)
         *  Each query transforms its world-space arguments into the node's local
         *  space, dispatches to the collision resource, and transforms the
         *  optional contact plane back into world space. */
        ///@{
        /** @brief Sweep an axis-aligned box through this node's collision geometry.
            @return Fraction in [0,1]; 1 means no contact. */
        float Sweep ( const Node& aNode, const AABB& aBox, const Vector3& aDisplacement, Plane* aContactPlane = nullptr ) const;
        /** @brief Cast a ray (point sweep) through this node's collision geometry.
            @return Fraction in [0,1]; 1 means no contact. */
        float RayCast ( const Node& aNode, const Vector3& aOrigin, const Vector3& aDirection, Plane* aContactPlane = nullptr ) const;
        /** @brief Test whether an axis-aligned box overlaps this node's collision geometry. */
        bool Overlap ( const Node& aNode, const AABB& aBox ) const;
        ///@}

        /** @name Scene-wide queries
         *  Traverse @p aScene, broad-phase cull candidate nodes by their world
         *  AABB, and dispatch to every node that carries a CollisionComponent,
         *  combining the results (nearest contact / first overlap). */
        ///@{
        /** @brief Sweep an axis-aligned box through every collider in the scene.
            @param[out] aHitNode Optional. Receives the node that was hit, or is left untouched on a miss.
            @return Nearest contact fraction in [0,1]; 1 means no contact. */
        static float Sweep ( Scene& aScene, const AABB& aBox, const Vector3& aDisplacement, Plane* aContactPlane = nullptr, Node** aHitNode = nullptr );
        /** @brief Cast a ray through every collider in the scene. */
        static float RayCast ( Scene& aScene, const Vector3& aOrigin, const Vector3& aDirection, Plane* aContactPlane = nullptr, Node** aHitNode = nullptr );
        /** @brief Test whether an axis-aligned box overlaps any collider in the scene.
            @param[out] aHitNode Optional. Receives the first overlapping node. */
        static bool Overlap ( Scene& aScene, const AABB& aBox, Node** aHitNode = nullptr );
        ///@}

        /** @brief Returns the class identifier for the CollisionComponent. */
        static const StringId& GetClassId();
    private:
        ResourceId mCollision{};
    };
}
#endif
