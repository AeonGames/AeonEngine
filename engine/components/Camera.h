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
#ifndef AEONGAMES_CAMERA_H
#define AEONGAMES_CAMERA_H
#include "aeongames/Component.hpp"

namespace AeonGames
{
    class Node;
    class Window;
    /** @brief Camera component providing perspective projection parameters for a scene node. */
    class Camera final : public Component
    {
    public:
        /** @brief Default constructor. */
        Camera();
        /** @name Overrides */
        ///@{
        ~Camera() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, Renderer& aRenderer, void* aWindowId ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}
        /** @brief Returns the field of view in degrees. */
        float GetFieldOfView() const;
        /** @brief Returns the near clipping plane distance. */
        float GetNearPlane() const;
        /** @brief Returns the far clipping plane distance. */
        float GetFarPlane() const;
        /** @brief Sets the field of view.
            @param aFieldOfView Field of view in degrees. */
        void SetFieldOfView ( float aFieldOfView );
        /** @brief Sets the near clipping plane distance.
            @param aNearPlane Near plane distance. */
        void SetNearPlane ( float aNearPlane );
        /** @brief Sets the far clipping plane distance.
            @param aFarPlane Far plane distance. */
        void SetFarPlane ( float aFarPlane );
        /** @brief Returns the class identifier for the Camera component. */
        static const StringId& GetClassId();
    private:
        float mFieldOfView{60.0f};
        float mNearPlane{1.0f};
        float mFarPlane{16000.0f};
    };
}
#endif
