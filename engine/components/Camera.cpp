/*
Copyright (C) 2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <cstring>
#include <cmath>
#include "Camera.h"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Buffer.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"

namespace AeonGames
{
    static const StringId CameraStringId{"Camera"};
    const StringId& Camera::ClassId()
    {
        return CameraStringId;
    }

    Camera::Camera() = default;

    Camera::~Camera() = default;

    const StringId& Camera::GetClassId() const
    {
        return CameraStringId;
    }

    float Camera::GetFieldOfView() const
    {
        return mFieldOfView;
    }
    float Camera::GetNearPlane() const
    {
        return mNearPlane;
    }
    float Camera::GetFarPlane() const
    {
        return mFarPlane;
    }
    void Camera::SetFieldOfView ( float aFieldOfView )
    {
        mFieldOfView = aFieldOfView;
    }
    void Camera::SetNearPlane ( float aNearPlane )
    {
        mNearPlane = aNearPlane;
    }
    void Camera::SetFarPlane ( float aFarPlane )
    {
        mFarPlane = aFarPlane;
    }

    void Camera::Update ( Node& aNode, double aDelta )
    {
        auto scene = aNode.GetScene();
        if ( scene && scene->GetCamera() == &aNode )
        {
            scene->SetViewMatrix ( aNode.GetGlobalTransform().GetInvertedMatrix() );
            scene->SetFieldOfView ( mFieldOfView );
            scene->SetNear ( mNearPlane );
            scene->SetFar ( mFarPlane );
        }
    }

    void Camera::ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData )
    {
    }
}
