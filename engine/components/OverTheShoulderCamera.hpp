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
#ifndef AEONGAMES_OVERTHESHOULDERCAMERA_HPP
#define AEONGAMES_OVERTHESHOULDERCAMERA_HPP
#include "aeongames/Component.hpp"

namespace AeonGames
{
    class Node;
    /** @brief Over-the-shoulder camera component.
     *
     * Positions the owning node behind and to the side of its parent node's
     * origin, producing a classic third-person "over the shoulder" framing.
     * The parent is expected to be the character node (typically the one
     * carrying a ModelComponent). The camera's own translation is set each
     * Update() as: parent_global_translation + (right, up, back) in world
     * space. Rotation is left untouched for this initial version.
     */
    class OverTheShoulderCamera final : public Component
    {
    public:
        OverTheShoulderCamera();
        /** @name Overrides */
        ///@{
        ~OverTheShoulderCamera() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, Renderer& aRenderer, void* aWindowId ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}

        /** Horizontal offset from the character origin (positive = right shoulder). */
        float GetShoulderOffset() const;
        /** Vertical offset from the character origin (positive = up). */
        float GetHeightOffset() const;
        /** Distance behind the character along world -Y (positive = farther back). */
        float GetDistanceBehind() const;
        float GetFieldOfView() const;
        float GetNearPlane() const;
        float GetFarPlane() const;

        void SetShoulderOffset ( float aShoulderOffset );
        void SetHeightOffset ( float aHeightOffset );
        void SetDistanceBehind ( float aDistanceBehind );
        void SetFieldOfView ( float aFieldOfView );
        void SetNearPlane ( float aNearPlane );
        void SetFarPlane ( float aFarPlane );

        static const StringId& GetClassId();

    private:
        float mShoulderOffset{0.5f};
        float mHeightOffset{1.7f};
        float mDistanceBehind{3.0f};
        float mFieldOfView{60.0f};
        float mNearPlane{1.0f};
        float mFarPlane{16000.0f};
    };
}
#endif
