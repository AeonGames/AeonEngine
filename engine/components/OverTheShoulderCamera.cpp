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
#include "OverTheShoulderCamera.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Quaternion.hpp"

namespace AeonGames
{
    static const StringId OverTheShoulderCameraStringId{"OverTheShoulderCamera"};
    const StringId& OverTheShoulderCamera::GetClassId()
    {
        return OverTheShoulderCameraStringId;
    }

    OverTheShoulderCamera::OverTheShoulderCamera() : Component{}
    {
    }

    OverTheShoulderCamera::~OverTheShoulderCamera() = default;

    const StringId& OverTheShoulderCamera::GetId() const
    {
        return OverTheShoulderCameraStringId;
    }

    static constexpr std::array<const StringId, 6> OverTheShoulderCameraPropertyIds
    {
        {
            {"Shoulder Offset"},
            {"Height Offset"},
            {"Distance Behind"},
            {"Field of View"},
            {"Near Plane"},
            {"Far Plane"}
        }
    };

    size_t OverTheShoulderCamera::GetPropertyCount () const
    {
        return OverTheShoulderCameraPropertyIds.size();
    }

    const StringId* OverTheShoulderCamera::GetPropertyInfoArray () const
    {
        return OverTheShoulderCameraPropertyIds.data();
    }

    float OverTheShoulderCamera::GetShoulderOffset() const
    {
        return mShoulderOffset;
    }
    float OverTheShoulderCamera::GetHeightOffset() const
    {
        return mHeightOffset;
    }
    float OverTheShoulderCamera::GetDistanceBehind() const
    {
        return mDistanceBehind;
    }
    float OverTheShoulderCamera::GetFieldOfView() const
    {
        return mFieldOfView;
    }
    float OverTheShoulderCamera::GetNearPlane() const
    {
        return mNearPlane;
    }
    float OverTheShoulderCamera::GetFarPlane() const
    {
        return mFarPlane;
    }

    void OverTheShoulderCamera::SetShoulderOffset ( float aShoulderOffset )
    {
        mShoulderOffset = aShoulderOffset;
    }
    void OverTheShoulderCamera::SetHeightOffset ( float aHeightOffset )
    {
        mHeightOffset = aHeightOffset;
    }
    void OverTheShoulderCamera::SetDistanceBehind ( float aDistanceBehind )
    {
        mDistanceBehind = aDistanceBehind;
    }
    void OverTheShoulderCamera::SetFieldOfView ( float aFieldOfView )
    {
        mFieldOfView = aFieldOfView;
    }
    void OverTheShoulderCamera::SetNearPlane ( float aNearPlane )
    {
        mNearPlane = aNearPlane;
    }
    void OverTheShoulderCamera::SetFarPlane ( float aFarPlane )
    {
        mFarPlane = aFarPlane;
    }

    Property OverTheShoulderCamera::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case OverTheShoulderCameraPropertyIds[0]:
            return GetShoulderOffset();
        case OverTheShoulderCameraPropertyIds[1]:
            return GetHeightOffset();
        case OverTheShoulderCameraPropertyIds[2]:
            return GetDistanceBehind();
        case OverTheShoulderCameraPropertyIds[3]:
            return GetFieldOfView();
        case OverTheShoulderCameraPropertyIds[4]:
            return GetNearPlane();
        case OverTheShoulderCameraPropertyIds[5]:
            return GetFarPlane();
        }
        return Property{};
    }

    void OverTheShoulderCamera::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        if ( !std::holds_alternative<float> ( aProperty ) )
        {
            return;
        }
        float value = std::get<float> ( aProperty );
        switch ( aId )
        {
        case OverTheShoulderCameraPropertyIds[0]:
            SetShoulderOffset ( value );
            break;
        case OverTheShoulderCameraPropertyIds[1]:
            SetHeightOffset ( value );
            break;
        case OverTheShoulderCameraPropertyIds[2]:
            SetDistanceBehind ( value );
            break;
        case OverTheShoulderCameraPropertyIds[3]:
            SetFieldOfView ( value );
            break;
        case OverTheShoulderCameraPropertyIds[4]:
            SetNearPlane ( value );
            break;
        case OverTheShoulderCameraPropertyIds[5]:
            SetFarPlane ( value );
            break;
        }
    }

    void OverTheShoulderCamera::Update ( Node& aNode, double /*aDelta*/ )
    {
        // The OTSC lives on the same node as the character/model, so it
        // must NOT mutate the node's transform. Instead, when this node is
        // the active scene camera, compute a virtual view transform offset
        // from the character and push it (and the projection parameters)
        // into the scene. The renderer reads scene->GetViewMatrix().
        auto scene = aNode.GetScene();
        if ( !scene || scene->GetCamera() != &aNode )
        {
            return;
        }

        const Transform& character = aNode.GetGlobalTransform();

        // Offset is expressed in the character's local frame so that the
        // camera orbits with the character as it turns. Axis convention
        // assumed: +X right, +Y forward, +Z up (so "behind" = -Y).
        Vector3 local_offset{ mShoulderOffset, -mDistanceBehind, mHeightOffset };
        Vector3 world_offset = character.GetRotation() * local_offset;

        Transform view_transform;
        view_transform.SetTranslation ( character.GetTranslation() + world_offset );
        view_transform.SetRotation ( character.GetRotation() );

        scene->SetViewMatrix ( view_transform.GetInvertedMatrix() );
        scene->SetFieldOfView ( mFieldOfView );
        scene->SetNear ( mNearPlane );
        scene->SetFar ( mFarPlane );
    }

    void OverTheShoulderCamera::Render ( const Node& /*aNode*/, Renderer& /*aRenderer*/, void* /*aWindowId*/ )
    {
    }

    void OverTheShoulderCamera::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ )
    {
    }
}
