/*
Copyright (C) 2019 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.h"
#include "aeongames/Matrix4x4.h"
#include "aeongames/Window.h"
#include "aeongames/UniformBuffer.h"
#include "aeongames/Renderer.h"
#include "aeongames/Node.h"

namespace AeonGames
{
    static const StringId CameraStringId{"Camera"};
    const StringId& Camera::GetClassId()
    {
        return CameraStringId;
    }

    Camera::Camera() : Component{}
    {
    }

    Camera::~Camera() = default;

    const StringId& Camera::GetId() const
    {
        return CameraStringId;
    }

    static constexpr std::array<const StringId, 3> CameraPropertyIds
    {
        {
            {"Near Plane"},
            {"Far Plane"},
            {"Field of Vision"}
        }
    };

    size_t Camera::GetPropertyCount () const
    {
        return CameraPropertyIds.size();
    }

    const StringId* Camera::GetPropertyInfoArray () const
    {
        return CameraPropertyIds.data();
    }

    float Camera::GetNearPlane() const
    {
        return mNearPlane;
    }
    float Camera::GetFarPlane() const
    {
        return mFarPlane;
    }
    void Camera::SetNearPlane ( float aNearPlane )
    {
        mNearPlane = aNearPlane;
    }
    void Camera::SetFarPlane ( float aFarPlane )
    {
        mFarPlane = aFarPlane;
    }

    Property Camera::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case CameraPropertyIds[0]:
            return GetNearPlane();
        case CameraPropertyIds[1]:
            return GetFarPlane();
        }
        return Property{};
    }

    void Camera::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        switch ( aId )
        {
        case CameraPropertyIds[0]:
            if ( std::holds_alternative<float> ( aProperty ) )
            {
                SetNearPlane ( std::get<float> ( aProperty ) );
            }
            break;
        case CameraPropertyIds[1]:
            if ( std::holds_alternative<float> ( aProperty ) )
            {
                SetFarPlane ( std::get<float> ( aProperty ) );
            }
            break;
        }
    }

    void Camera::Update ( Node& aNode, double aDelta )
    {
    }

    void Camera::Render ( const Node& aNode, const Window& aWindow ) const
    {
    }

    void Camera::ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData )
    {
    }
}
