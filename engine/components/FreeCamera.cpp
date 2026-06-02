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
#include <cmath>
#include <algorithm>
#include "FreeCamera.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/InputSystem.hpp"

namespace AeonGames
{
    static const StringId FreeCameraStringId{"Free Camera"};
    const StringId& FreeCamera::GetClassId()
    {
        return FreeCameraStringId;
    }

    // Default key bindings. These target Win32 virtual key codes
    // (W=0x57, A=0x41, S=0x53, D=0x44, E=0x45, Q=0x51, arrows=0x25..0x28,
    // Left Shift=0xA0). The Win32 front end forwards wParam directly to
    // InputSystem::OnKeyEvent, so these values work as-is on Windows. Other
    // front-ends will eventually need a translation layer or platform
    // specific default bindings.
    namespace
    {
        constexpr uint32_t kKeyW      = 0x57;
        constexpr uint32_t kKeyA      = 0x41;
        constexpr uint32_t kKeyS      = 0x53;
        constexpr uint32_t kKeyD      = 0x44;
        constexpr uint32_t kKeyE      = 0x45;
        constexpr uint32_t kKeyQ      = 0x51;
        constexpr uint32_t kKeyLeft   = 0x25;
        constexpr uint32_t kKeyUp     = 0x26;
        constexpr uint32_t kKeyRight  = 0x27;
        constexpr uint32_t kKeyDown   = 0x28;
        constexpr uint32_t kKeyLShift = 0xA0;

        constexpr float kPi = 3.14159265358979323846f;
        constexpr float kRadToDeg = 180.0f / kPi;
        constexpr float kMaxPitch = 89.0f;
    }

    FreeCamera::FreeCamera() : Component{}
    {
        mActionMap.Bind ( Action_MoveForward,  { ActionMap::BindingType::Key, kKeyW } );
        mActionMap.Bind ( Action_MoveBackward, { ActionMap::BindingType::Key, kKeyS } );
        mActionMap.Bind ( Action_StrafeLeft,   { ActionMap::BindingType::Key, kKeyA } );
        mActionMap.Bind ( Action_StrafeRight,  { ActionMap::BindingType::Key, kKeyD } );
        mActionMap.Bind ( Action_MoveUp,       { ActionMap::BindingType::Key, kKeyE } );
        mActionMap.Bind ( Action_MoveDown,     { ActionMap::BindingType::Key, kKeyQ } );
        mActionMap.Bind ( Action_LookLeft,     { ActionMap::BindingType::Key, kKeyLeft } );
        mActionMap.Bind ( Action_LookRight,    { ActionMap::BindingType::Key, kKeyRight } );
        mActionMap.Bind ( Action_LookUp,       { ActionMap::BindingType::Key, kKeyUp } );
        mActionMap.Bind ( Action_LookDown,     { ActionMap::BindingType::Key, kKeyDown } );
        mActionMap.Bind ( Action_Boost,        { ActionMap::BindingType::Key, kKeyLShift } );
    }

    FreeCamera::~FreeCamera() = default;

    const StringId& FreeCamera::GetId() const
    {
        return FreeCameraStringId;
    }

    static constexpr std::array<const StringId, 6> FreeCameraPropertyIds
    {
        {
            {"Move Speed"},
            {"Boost Multiplier"},
            {"Look Speed"},
            {"Field of View"},
            {"Near Plane"},
            {"Far Plane"}
        }
    };

    size_t FreeCamera::GetPropertyCount () const
    {
        return FreeCameraPropertyIds.size();
    }

    const StringId* FreeCamera::GetPropertyInfoArray () const
    {
        return FreeCameraPropertyIds.data();
    }

    float FreeCamera::GetMoveSpeed() const
    {
        return mMoveSpeed;
    }
    float FreeCamera::GetBoostMultiplier() const
    {
        return mBoostMultiplier;
    }
    float FreeCamera::GetLookSpeed() const
    {
        return mLookSpeed;
    }
    float FreeCamera::GetFieldOfView() const
    {
        return mFieldOfView;
    }
    float FreeCamera::GetNearPlane() const
    {
        return mNearPlane;
    }
    float FreeCamera::GetFarPlane() const
    {
        return mFarPlane;
    }

    void FreeCamera::SetMoveSpeed ( float aMoveSpeed )
    {
        mMoveSpeed = aMoveSpeed;
    }
    void FreeCamera::SetBoostMultiplier ( float aBoostMultiplier )
    {
        mBoostMultiplier = aBoostMultiplier;
    }
    void FreeCamera::SetLookSpeed ( float aLookSpeed )
    {
        mLookSpeed = aLookSpeed;
    }
    void FreeCamera::SetFieldOfView ( float aFieldOfView )
    {
        mFieldOfView = aFieldOfView;
    }
    void FreeCamera::SetNearPlane ( float aNearPlane )
    {
        mNearPlane = aNearPlane;
    }
    void FreeCamera::SetFarPlane ( float aFarPlane )
    {
        mFarPlane = aFarPlane;
    }

    ActionMap& FreeCamera::GetActionMap()
    {
        return mActionMap;
    }
    const ActionMap& FreeCamera::GetActionMap() const
    {
        return mActionMap;
    }

    Property FreeCamera::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case FreeCameraPropertyIds[0]:
            return GetMoveSpeed();
        case FreeCameraPropertyIds[1]:
            return GetBoostMultiplier();
        case FreeCameraPropertyIds[2]:
            return GetLookSpeed();
        case FreeCameraPropertyIds[3]:
            return GetFieldOfView();
        case FreeCameraPropertyIds[4]:
            return GetNearPlane();
        case FreeCameraPropertyIds[5]:
            return GetFarPlane();
        }
        return Property{};
    }

    void FreeCamera::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        if ( !std::holds_alternative<float> ( aProperty ) )
        {
            return;
        }
        float value = std::get<float> ( aProperty );
        switch ( aId )
        {
        case FreeCameraPropertyIds[0]:
            SetMoveSpeed ( value );
            break;
        case FreeCameraPropertyIds[1]:
            SetBoostMultiplier ( value );
            break;
        case FreeCameraPropertyIds[2]:
            SetLookSpeed ( value );
            break;
        case FreeCameraPropertyIds[3]:
            SetFieldOfView ( value );
            break;
        case FreeCameraPropertyIds[4]:
            SetNearPlane ( value );
            break;
        case FreeCameraPropertyIds[5]:
            SetFarPlane ( value );
            break;
        }
    }

    void FreeCamera::Update ( Node& aNode, double aDelta )
    {
        Scene* scene = aNode.GetScene();
        if ( !scene || scene->GetCamera() != &aNode )
        {
            return;
        }

        Transform t = aNode.GetLocalTransform();

        // Seed yaw/pitch from the node's authored orientation the first time
        // we run so the free camera starts looking where the scene camera
        // pointed. Axis convention: +X right, +Y forward, +Z up; camera
        // looks along +Y. Roll in the authored rotation is discarded.
        if ( !mInitialized )
        {
            Vector3 forward = t.GetRotation() * Vector3{0.0f, 1.0f, 0.0f};
            mYaw = std::atan2 ( -forward.GetX(), forward.GetY() ) * kRadToDeg;
            mPitch = std::asin ( std::clamp ( forward.GetZ(), -1.0f, 1.0f ) ) * kRadToDeg;
            mInitialized = true;
        }

        const float dt = static_cast<float> ( aDelta );
        InputSystem* input = scene->GetInputSystem();

        if ( input )
        {
            // Mouse look while the right mouse button is held. Relative-mouse
            // mode keeps the cursor confined and recentred so deltas keep
            // accumulating without the cursor leaving the window.
            bool looking = input->IsMouseButtonDown ( MouseButton_Right );
            input->SetRelativeMouseMode ( looking );
            if ( looking )
            {
                mYaw   -= static_cast<float> ( input->GetMouseDeltaX() ) * mLookSpeed;
                mPitch -= static_cast<float> ( input->GetMouseDeltaY() ) * mLookSpeed;
            }

            // Arrow-key look as a mouse-free alternative.
            if ( mActionMap.IsActionDown ( Action_LookLeft,  *input ) )
            {
                mYaw += mKeyTurnSpeed * dt;
            }
            if ( mActionMap.IsActionDown ( Action_LookRight, *input ) )
            {
                mYaw -= mKeyTurnSpeed * dt;
            }
            if ( mActionMap.IsActionDown ( Action_LookUp,    *input ) )
            {
                mPitch += mKeyTurnSpeed * dt;
            }
            if ( mActionMap.IsActionDown ( Action_LookDown,  *input ) )
            {
                mPitch -= mKeyTurnSpeed * dt;
            }
        }

        mPitch = std::clamp ( mPitch, -kMaxPitch, kMaxPitch );

        // Rebuild the orientation from yaw (about world up) then pitch (about
        // the yawed local right axis). Building it fresh each frame keeps the
        // camera roll-free regardless of accumulated input.
        Transform orientation;
        orientation.RotateInertialSpace ( mYaw, 0.0f, 0.0f, 1.0f );
        orientation.RotateObjectSpace ( mPitch, 1.0f, 0.0f, 0.0f );
        t.SetRotation ( orientation.GetRotation() );

        if ( input )
        {
            float forward = 0.0f;
            float strafe  = 0.0f;
            float updown  = 0.0f;
            if ( mActionMap.IsActionDown ( Action_MoveForward,  *input ) )
            {
                forward += 1.0f;
            }
            if ( mActionMap.IsActionDown ( Action_MoveBackward, *input ) )
            {
                forward -= 1.0f;
            }
            if ( mActionMap.IsActionDown ( Action_StrafeRight,  *input ) )
            {
                strafe += 1.0f;
            }
            if ( mActionMap.IsActionDown ( Action_StrafeLeft,   *input ) )
            {
                strafe -= 1.0f;
            }
            if ( mActionMap.IsActionDown ( Action_MoveUp,       *input ) )
            {
                updown += 1.0f;
            }
            if ( mActionMap.IsActionDown ( Action_MoveDown,     *input ) )
            {
                updown -= 1.0f;
            }

            float speed = mMoveSpeed;
            if ( mActionMap.IsActionDown ( Action_Boost, *input ) )
            {
                speed *= mBoostMultiplier;
            }

            // Forward/strafe follow the view direction (object space), so the
            // camera flies along where it looks. Vertical movement is in world
            // space so Q/E always raise/lower regardless of pitch.
            if ( forward != 0.0f || strafe != 0.0f )
            {
                t.MoveInObjectSpace ( strafe * speed * dt, forward * speed * dt, 0.0f );
            }
            if ( updown != 0.0f )
            {
                t.Move ( 0.0f, 0.0f, updown * speed * dt );
            }
        }

        aNode.SetLocalTransform ( t );

        scene->SetViewMatrix ( aNode.GetGlobalTransform().GetInvertedMatrix() );
        scene->SetFieldOfView ( mFieldOfView );
        scene->SetNear ( mNearPlane );
        scene->SetFar ( mFarPlane );
    }

    void FreeCamera::Render ( const Node& /*aNode*/, Renderer& /*aRenderer*/, void* /*aWindowId*/ )
    {
    }

    void FreeCamera::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ )
    {
    }
}
