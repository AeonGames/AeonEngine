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
#ifndef AEONGAMES_FREECAMERA_HPP
#define AEONGAMES_FREECAMERA_HPP
#include "aeongames/Component.hpp"
#include "aeongames/ActionMap.hpp"

namespace AeonGames
{
    class Node;

    /** @brief Free-flying spectator camera driven by keyboard and mouse.
     *
     * Intended for navigating a scene during development. When the owning
     * node is the active scene camera the component drives the node's own
     * transform and pushes the resulting view matrix and projection
     * parameters into the scene (so it doubles as the camera, like the
     * Camera component).
     *
     * Controls (default bindings target Win32 virtual key codes):
     *  - W / S          : move forward / backward along the view direction.
     *  - A / D          : strafe left / right.
     *  - E / Q          : move up / down in world space.
     *  - Arrow keys     : look (yaw with Left/Right, pitch with Up/Down).
     *  - Right mouse    : hold to mouse-look (yaw/pitch from mouse motion).
     *  - Left Shift     : speed boost while held.
     *
     * Yaw is applied about the world up axis (+Z) and pitch about the
     * camera's local right axis (+X); roll is never introduced. The axis
     * convention matches the other camera components: +X right, +Y forward,
     * +Z up, with the camera looking along +Y.
     */
    class FreeCamera final : public Component
    {
    public:
        FreeCamera();
        /** @name Overrides */
        ///@{
        ~FreeCamera() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        void Update ( Node& aNode, double aDelta ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}

        float GetMoveSpeed() const;
        float GetBoostMultiplier() const;
        float GetLookSpeed() const;
        float GetFieldOfView() const;
        float GetNearPlane() const;
        float GetFarPlane() const;

        void SetMoveSpeed ( float aMoveSpeed );
        void SetBoostMultiplier ( float aBoostMultiplier );
        void SetLookSpeed ( float aLookSpeed );
        void SetFieldOfView ( float aFieldOfView );
        void SetNearPlane ( float aNearPlane );
        void SetFarPlane ( float aFarPlane );

        /** Access the underlying action map so callers can rebind. */
        ActionMap& GetActionMap();
        const ActionMap& GetActionMap() const;

        static const StringId& GetClassId();

        /** @brief Action identifiers consumed by this component. */
        enum Action : uint32_t
        {
            Action_MoveForward  = 0,
            Action_MoveBackward = 1,
            Action_StrafeLeft   = 2,
            Action_StrafeRight  = 3,
            Action_MoveUp       = 4,
            Action_MoveDown     = 5,
            Action_LookLeft     = 6,
            Action_LookRight    = 7,
            Action_LookUp       = 8,
            Action_LookDown     = 9,
            Action_Boost        = 10
        };

    private:
        float mMoveSpeed{6.0f};        ///< meters / second
        float mBoostMultiplier{4.0f};  ///< speed factor while boosting
        float mLookSpeed{0.15f};       ///< degrees per mouse-pixel
        float mKeyTurnSpeed{90.0f};    ///< degrees / second for arrow-key look
        float mFieldOfView{60.0f};
        float mNearPlane{0.1f};
        float mFarPlane{10000.0f};
        float mYaw{0.0f};              ///< degrees about world +Z
        float mPitch{0.0f};            ///< degrees about local +X
        bool  mInitialized{false};     ///< yaw/pitch seeded from node rotation
        ActionMap mActionMap{};
    };
}
#endif
