/*
Copyright (C) 2018,2019,2021,2022,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include <string>
#include <string_view>
#include <vector>
#include "aeongames/Component.hpp"
#include "aeongames/ResourceId.hpp"
#include "aeongames/BufferAccessor.hpp"
#include "aeongames/Transform.hpp"

namespace AeonGames
{
    class Node;
    class Window;
    class Buffer;
    class Model;
    /** @brief Component that attaches a 3D model with skeletal animation support to a scene node. */
    class ModelComponent final : public Component
    {
    public:
        /** @brief Default constructor. */
        ModelComponent();
        /** @name Overrides */
        ///@{
        ~ModelComponent() final;
        const StringId& GetId() const final;
        size_t GetPropertyCount () const final;
        const StringId* GetPropertyInfoArray () const final;
        Property GetProperty ( const StringId& aId ) const final;
        void SetProperty ( uint32_t, const Property& aProperty ) final;
        const std::vector<std::string>& GetPropertyEnumValues ( const StringId& aId ) const final;
        void Update ( Node& aNode, double aDelta ) final;
        void Render ( const Node& aNode, Renderer& aRenderer, void* aWindowId ) final;
        void Skin ( const Node& aNode, Renderer& aRenderer, void* aWindowId ) final;
        uint32_t GetInstanceBatchId() const final;
        void ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData ) final;
        ///@}

        /** @name Properties */
        ///@{
        /** @brief Sets the model resource.
            @param aModel Resource identifier of the model. */
        void SetModel ( const ResourceId& aModel );
        /** @brief Returns the current model resource identifier. */
        const ResourceId& GetModel() const noexcept;
        /** @brief Sets the active animation by name.
            @param aActiveAnimation Name of the animation as declared by the model. */
        void SetActiveAnimation ( std::string_view aActiveAnimation );
        /** @brief Returns the name of the active animation. */
        const std::string& GetActiveAnimation() const noexcept;
        /** @brief Sets the starting frame for the active animation.
            @param aAnimationDelta Starting frame value. */
        void SetStartingFrame ( double aAnimationDelta ) noexcept;
        /** @brief Returns the starting frame for the active animation. */
        const double& GetStartingFrame() const noexcept;
        /** @brief Sets the crossfade duration used when switching animations.
            @param aSeconds Blend duration in seconds. A value of 0 disables
            crossfading and switches animations instantly. */
        void SetBlendDuration ( float aSeconds ) noexcept;
        /** @brief Returns the crossfade duration in seconds. */
        float GetBlendDuration() const noexcept;
        ///@}
        /** @brief Returns the class identifier for the ModelComponent. */
        static const StringId& GetClassId();
    private:
        // Properties
        ResourceId mModel{};
        std::string mActiveAnimation{};
        double mStartingFrame{};
        // Private Data
        double mCurrentSample{};
        // Cached resolution of mActiveAnimation against the currently loaded
        // model. Invalidated when the model or the active animation name
        // changes, or when the underlying Model pointer changes (e.g. reload).
        const Model* mLastResolvedModel{nullptr};
        size_t mActiveAnimationIndex{static_cast<size_t> ( -1 ) };
        // Crossfade state. When a new animation is requested mid-render we
        // capture the currently-displayed per-bone pose into mBlendSnapshot
        // and then interpolate from that frozen pose to the freshly started
        // animation. This avoids the popping that occurs when switching
        // animations again while a previous crossfade is still in flight.
        std::string mPendingAnimation{};
        bool mPendingAnimationSwitch{false};
        std::vector<Transform> mBlendSnapshot{};
        bool mHasBlendSnapshot{false};
        float mBlendDuration{0.25f};
        float mBlendElapsed{0.0f};
        // 128 is the maximum number of bones per model
        std::array<uint8_t, 16 * 128 * sizeof ( float ) > mSkeleton{};
        // Per-assembly skinned output vertex buffers produced by the compute
        // skinning pre-pass (Skin). Indexed in lockstep with the model's
        // assemblies; entries for non-skinned assemblies remain default (empty).
        // Frame-transient: refilled every Skin() and consumed by Render().
        std::vector<BufferAccessor> mSkinnedVertices{};
    };
}
#endif
