/*
Copyright (C) 2018,2019,2021,2022 Rodrigo Jose Hernandez Cordoba

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
#ifndef AEONGAMES_MODELCOMPONENT_H
#define AEONGAMES_MODELCOMPONENT_H
#include <array>
#include "aeongames/Component.h"
#include "aeongames/ResourceId.h"
#include "aeongames/BufferAccessor.h"

namespace AeonGames
{
    class Node;
    class Window;
    class Buffer;
    class ModelComponent final : public Component
    {
    public:
        ModelComponent();
        /** @name Overrides */
        ///@{
        ~ModelComponent() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, Renderer& aRenderer, void* aWindowId ) final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}

        /** @name Properties */
        ///@{
        void SetModel ( const ResourceId& aModel );
        const ResourceId& GetModel() const noexcept;
        void SetActiveAnimation ( size_t aActiveAnimation ) noexcept;
        const size_t& GetActiveAnimation() const noexcept;
        void SetStartingFrame ( double aAnimationDelta ) noexcept;
        const double& GetStartingFrame() const noexcept;
        ///@}
        static const StringId& GetClassId();
    private:
        // Properties
        ResourceId mModel{};
        size_t mActiveAnimation{};
        double mStartingFrame{};
        // Private Data
        double mCurrentSample{};
        // 128 is the maximum number of bones per model
        std::array<uint8_t, 16 * 128 * sizeof ( float ) > mSkeleton{};
    };
}
#endif
