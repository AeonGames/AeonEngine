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
        // Resolve the parent Node (the character). If the parent is the
        // scene root rather than a Node, fall back to the world origin so
        // this component remains well-defined while prototyping.
        Vector3 parent_origin{0.0f, 0.0f, 0.0f};
        if ( Node * parent = GetNodePtr ( aNode.GetParent() ) )
        {
            parent_origin = parent->GetGlobalTransform().GetTranslation();
        }

        // Apply the over-the-shoulder offset in world space for this first
        // pass (no yaw tracking yet). The caller is expected to tune these
        // three floats to match the engine's axis convention; with Z-up /
        // +Y-forward that maps to (right, -back, up), so a positive
        // mDistanceBehind moves the camera along -Y.
        Vector3 offset{ mShoulderOffset, -mDistanceBehind, mHeightOffset };
        Vector3 camera_position = parent_origin + offset;

        Transform global = aNode.GetGlobalTransform();
        global.SetTranslation ( camera_position );
        aNode.SetGlobalTransform ( global );

        // Mirror the Camera component behaviour: if this node is the active
        // scene camera, push projection parameters into the scene.
        auto scene = aNode.GetScene();
        if ( scene && scene->GetCamera() == &aNode )
        {
            scene->SetFieldOfView ( mFieldOfView );
            scene->SetNear ( mNearPlane );
            scene->SetFar ( mFarPlane );
        }
    }

    void OverTheShoulderCamera::Render ( const Node& /*aNode*/, Renderer& /*aRenderer*/, void* /*aWindowId*/ )
    {
    }

    void OverTheShoulderCamera::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ )
    {
    }
}
