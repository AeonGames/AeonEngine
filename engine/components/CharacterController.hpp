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
#ifndef AEONGAMES_CHARACTERCONTROLLER_HPP
#define AEONGAMES_CHARACTERCONTROLLER_HPP
#include "aeongames/Component.hpp"
#include "aeongames/ActionMap.hpp"

namespace AeonGames
{
    class Node;

    /** @brief Drives an owning node from polled input via an ActionMap.
     *
     * Translates the owning node forward / backward along its local +Y axis
     * and strafes along its local +X axis at "Move Speed" meters per second,
     * and yaws about its local +Z axis at "Turn Speed" degrees per second.
     *
     * The component queries the InputSystem attached to the scene via
     * Scene::GetInputSystem(); if no input system is present the component
     * is a no-op. The default key bindings target Win32 virtual key codes
     * (W/A/S/D and arrow keys). Other platforms will need a key-code
     * translation layer in the front-end before bindings make sense there.
     */
    class CharacterController final : public Component
    {
    public:
        CharacterController();
        /** @name Overrides */
        ///@{
        ~CharacterController() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        void Update ( Node& aNode, double aDelta ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}

        float GetMoveSpeed() const;
        float GetTurnSpeed() const;
        void SetMoveSpeed ( float aMoveSpeed );
        void SetTurnSpeed ( float aTurnSpeed );

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
            Action_TurnLeft     = 4,
            Action_TurnRight    = 5
        };

    private:
        float mMoveSpeed{3.0f};   ///< meters / second
        float mTurnSpeed{120.0f}; ///< degrees / second
        ActionMap mActionMap{};
    };
}
#endif
