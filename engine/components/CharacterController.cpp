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
#include <string_view>
#include "CharacterController.hpp"
#include "ModelComponent.h"
#include "CollisionComponent.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Quaternion.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/InputSystem.hpp"

namespace AeonGames
{
    static const StringId CharacterControllerStringId{"Character Controller"};
    const StringId& CharacterController::GetClassId()
    {
        return CharacterControllerStringId;
    }

    // Default key bindings. KeyCode names physical keys, so these follow the
    // WASD block wherever the active layout puts its labels.
    CharacterController::CharacterController() : Component{}
    {
        mActionMap.Bind ( Action_MoveForward,  KeyCode::W );
        mActionMap.Bind ( Action_MoveForward,  KeyCode::Up );
        mActionMap.Bind ( Action_MoveBackward, KeyCode::S );
        mActionMap.Bind ( Action_MoveBackward, KeyCode::Down );
        mActionMap.Bind ( Action_StrafeLeft,   KeyCode::A );
        mActionMap.Bind ( Action_StrafeRight,  KeyCode::D );
        mActionMap.Bind ( Action_TurnLeft,     KeyCode::Left );
        mActionMap.Bind ( Action_TurnRight,    KeyCode::Right );
    }

    CharacterController::~CharacterController() = default;

    const StringId& CharacterController::GetId() const
    {
        return CharacterControllerStringId;
    }

    static constexpr std::array<const StringId, 2> CharacterControllerPropertyIds
    {
        {
            {"Move Speed"},
            {"Turn Speed"}
        }
    };

    size_t CharacterController::GetPropertyCount () const
    {
        return CharacterControllerPropertyIds.size();
    }

    const StringId* CharacterController::GetPropertyInfoArray () const
    {
        return CharacterControllerPropertyIds.data();
    }

    float CharacterController::GetMoveSpeed() const
    {
        return mMoveSpeed;
    }
    float CharacterController::GetTurnSpeed() const
    {
        return mTurnSpeed;
    }
    void CharacterController::SetMoveSpeed ( float aMoveSpeed )
    {
        mMoveSpeed = aMoveSpeed;
    }
    void CharacterController::SetTurnSpeed ( float aTurnSpeed )
    {
        mTurnSpeed = aTurnSpeed;
    }

    ActionMap& CharacterController::GetActionMap()
    {
        return mActionMap;
    }
    const ActionMap& CharacterController::GetActionMap() const
    {
        return mActionMap;
    }

    Property CharacterController::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case CharacterControllerPropertyIds[0]:
            return GetMoveSpeed();
        case CharacterControllerPropertyIds[1]:
            return GetTurnSpeed();
        }
        return Property{};
    }

    void CharacterController::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        if ( !std::holds_alternative<float> ( aProperty ) )
        {
            return;
        }
        float value = std::get<float> ( aProperty );
        switch ( aId )
        {
        case CharacterControllerPropertyIds[0]:
            SetMoveSpeed ( value );
            break;
        case CharacterControllerPropertyIds[1]:
            SetTurnSpeed ( value );
            break;
        }
    }

    void CharacterController::Update ( Node& aNode, double aDelta )
    {
        Scene* scene = aNode.GetScene();
        if ( !scene )
        {
            return;
        }
        InputSystem* input = scene->GetInputSystem();
        if ( !input )
        {
            return;
        }

        float forward = 0.0f;
        if ( mActionMap.IsActionDown ( Action_MoveForward,  *input ) )
        {
            forward += 1.0f;
        }
        if ( mActionMap.IsActionDown ( Action_MoveBackward, *input ) )
        {
            forward -= 1.0f;
        }

        float strafe = 0.0f;
        if ( mActionMap.IsActionDown ( Action_StrafeRight, *input ) )
        {
            strafe += 1.0f;
        }
        if ( mActionMap.IsActionDown ( Action_StrafeLeft,  *input ) )
        {
            strafe -= 1.0f;
        }

        float turn = 0.0f;
        if ( mActionMap.IsActionDown ( Action_TurnLeft,  *input ) )
        {
            turn += 1.0f;
        }
        if ( mActionMap.IsActionDown ( Action_TurnRight, *input ) )
        {
            turn -= 1.0f;
        }

        // Map current input intent to a named animation clip. The names
        // must match the animations declared in the model's .mdl file
        // (see assets/aerin/aerin.blend for the canonical list). Forward +
        // strafe combinations use the dedicated "strafe walk" clips when
        // available; pure strafing or pure turning fall back to the
        // in-place strafe and turn clips. With no input the character
        // returns to the idle pose.
        std::string_view desired_animation = "Idle";
        if ( forward != 0.0f && strafe < 0.0f )
        {
            desired_animation = "Left Strafe Walk";
        }
        else if ( forward != 0.0f && strafe > 0.0f )
        {
            desired_animation = "Right Strafe Walk";
        }
        else if ( forward != 0.0f )
        {
            desired_animation = "Walking";
        }
        else if ( strafe < 0.0f )
        {
            desired_animation = "Left Strafe";
        }
        else if ( strafe > 0.0f )
        {
            desired_animation = "Strafe Right";
        }
        else if ( turn > 0.0f )
        {
            desired_animation = "Left Turn";
        }
        else if ( turn < 0.0f )
        {
            desired_animation = "Right Turn";
        }

        if ( Component * component = aNode.GetComponent ( ModelComponent::GetClassId() ) )
        {
            auto* model_component = static_cast<ModelComponent*> ( component );
            if ( model_component->GetActiveAnimation() != desired_animation )
            {
                model_component->SetActiveAnimation ( desired_animation );
            }
        }

        const float dt = static_cast<float> ( aDelta );

        // Axis convention (matches OverTheShoulderCamera): +X right, +Y forward, +Z up.
        if ( forward != 0.0f || strafe != 0.0f || turn != 0.0f )
        {
            Transform t = aNode.GetLocalTransform();
            if ( forward != 0.0f || strafe != 0.0f )
            {
                float strafe_move = strafe * mMoveSpeed * dt;
                float forward_move = forward * mMoveSpeed * dt;
                // Sweep the character's bounds through the scene's collision
                // geometry and clamp the motion to the first contact so the
                // character stops at walls. With no collision nodes present the
                // sweep returns 1 and movement is unaffected. The displacement
                // is linear in the object-space amounts, so scaling them by the
                // returned fraction also scales the world displacement.
                // @note A pure stop can stick if the character starts embedded;
                // sliding / depenetration is a later refinement.
                const Transform& global = aNode.GetGlobalTransform();
                Vector3 world_displacement = global.GetRotation() * Vector3{ strafe_move, forward_move, 0.0f };
                float fraction = CollisionComponent::Sweep ( *scene, global * aNode.GetAABB(), world_displacement );
                t.MoveInObjectSpace ( strafe_move * fraction,
                                      forward_move * fraction,
                                      0.0f );
            }
            if ( turn != 0.0f )
            {
                t.RotateObjectSpace ( turn * mTurnSpeed * dt, 0.0f, 0.0f, 1.0f );
            }
            aNode.SetLocalTransform ( t );
        }
    }

    void CharacterController::ProcessMessage ( Node& /*aNode*/, uint32_t /*aMessageType*/, const void* /*aMessageData*/ )
    {
    }
}
