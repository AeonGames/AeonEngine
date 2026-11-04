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
#include <iostream>
#include <cmath>
#include "OrbitalCamera.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Quaternion.hpp"
#include "aeongames/InputSystem.hpp"
#include "aeongames/LogLevel.hpp"

namespace AeonGames
{
    static const StringId OrbitalCameraStringId{"OrbitalCamera"};
    const StringId& OrbitalCamera::ClassId()
    {
        return OrbitalCameraStringId;
    }

    OrbitalCamera::OrbitalCamera() = default;

    OrbitalCamera::~OrbitalCamera() = default;

    const StringId& OrbitalCamera::GetClassId() const
    {
        return OrbitalCameraStringId;
    }

    float OrbitalCamera::GetDistance() const
    {
        return mDistance;
    }
    float OrbitalCamera::GetAzimuth() const
    {
        return mAzimuth;
    }
    float OrbitalCamera::GetElevation() const
    {
        return mElevation;
    }
    float OrbitalCamera::GetHeightOffset() const
    {
        return mHeightOffset;
    }
    float OrbitalCamera::GetFieldOfView() const
    {
        return mFieldOfView;
    }
    float OrbitalCamera::GetNearPlane() const
    {
        return mNearPlane;
    }
    float OrbitalCamera::GetFarPlane() const
    {
        return mFarPlane;
    }

    void OrbitalCamera::SetDistance ( float aDistance )
    {
        mDistance = aDistance;
    }
    void OrbitalCamera::SetAzimuth ( float aAzimuth )
    {
        mAzimuth = aAzimuth;
    }
    void OrbitalCamera::SetElevation ( float aElevation )
    {
        mElevation = aElevation;
    }
    void OrbitalCamera::SetHeightOffset ( float aHeightOffset )
    {
        mHeightOffset = aHeightOffset;
    }
    void OrbitalCamera::SetFieldOfView ( float aFieldOfView )
    {
        mFieldOfView = aFieldOfView;
    }
    void OrbitalCamera::SetNearPlane ( float aNearPlane )
    {
        mNearPlane = aNearPlane;
    }
    void OrbitalCamera::SetFarPlane ( float aFarPlane )
    {
        mFarPlane = aFarPlane;
    }

    void OrbitalCamera::Update ( Node& aNode, double /*aDelta*/ )
    {
        auto scene = aNode.GetScene();
        if ( !scene || scene->GetCamera() != &aNode )
        {
            return;
        }

        InputSystem* input = scene->GetInputSystem();

        if ( input )
        {
            bool looking = input->IsMouseButtonDown ( MouseButton_Right );
            input->SetRelativeMouseMode ( looking );
            if ( looking )
            {
                mAzimuth   = fmod ( mAzimuth + static_cast<float> ( input->GetMouseDeltaX() ), 360.0f );
                mElevation = mElevation + static_cast<float> ( input->GetMouseDeltaY() );
            }
        }

        const Transform& character = aNode.GetGlobalTransform();
        Transform view_transform{};
        view_transform.RotateInertialSpace ( static_cast<float> ( - ( mAzimuth ) ), 0.0f, 0.0f, 1.0f );
        view_transform.RotateObjectSpace ( static_cast<float> ( - ( mElevation ) ), 1.0f, 0.0f, 0.0f );
        view_transform.MoveInObjectSpace ( 0.0f, -mDistance, 0.0f );
        view_transform.MoveInInertialSpace ( character.GetTranslation() + Vector3{0.0f, 0.0f, mHeightOffset} );
        scene->SetViewMatrix ( view_transform.GetInvertedMatrix() );
        scene->SetFieldOfView ( mFieldOfView );
        scene->SetNear ( mNearPlane );
        scene->SetFar ( mFarPlane );
    }

    void OrbitalCamera::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ )
    {
    }
}
