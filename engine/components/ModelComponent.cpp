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

#include <array>
#include <cstring>
#include <cmath>
#include <cassert>
#include "aeongames/ProtoBufClasses.hpp"
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Skeleton.hpp"
#include "aeongames/Animation.hpp"
#include "aeongames/Matrix4x4.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/Quaternion.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Buffer.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Model.hpp"
#include "ModelComponent.h"

namespace
{
    // Hermite smoothstep ease - removes the linear ramp's velocity
    // discontinuities at the start and end of the blend, which is the
    // main reason a constant-rate crossfade still reads as "snappy".
    inline float SmoothStep ( float t )
    {
        if ( t <= 0.0f )
        {
            return 0.0f;
        }
        if ( t >= 1.0f )
        {
            return 1.0f;
        }
        return t * t * ( 3.0f - 2.0f * t );
    }

    // Linearly interpolate scale and translation, slerp the rotation
    // quaternion. Slerp gives constant angular velocity along the
    // shortest arc which is noticeably smoother than nlerp for the
    // larger per-bone rotation deltas that occur between full body
    // animations like Idle <-> Walking.
    AeonGames::Transform BlendTransform (
        const AeonGames::Transform& a,
        const AeonGames::Transform& b,
        float t )
    {
        const AeonGames::Vector3& sa = a.GetScale();
        const AeonGames::Vector3& sb = b.GetScale();
        const AeonGames::Vector3& ta = a.GetTranslation();
        const AeonGames::Vector3& tb = b.GetTranslation();
        AeonGames::Vector3 scale = sa + ( sb - sa ) * t;
        AeonGames::Vector3 translation = ta + ( tb - ta ) * t;
        AeonGames::Quaternion rotation =
            AeonGames::SlerpQuats ( a.GetRotation(), b.GetRotation(), t );
        return AeonGames::Transform{ scale, rotation, translation };
    }
}

namespace AeonGames
{
    static const StringId ModelStringId{"Model Component"};
    const StringId& ModelComponent::GetClassId()
    {
        return ModelStringId;
    }

    ModelComponent::ModelComponent() = default;
    ModelComponent::~ModelComponent() = default;

    const StringId& ModelComponent::GetId() const
    {
        return ModelStringId;
    }

    static constexpr std::array<const StringId, 3> ModelComponentPropertyIds
    {
        {
            {"Model"},
            {"Active Animation"},
            {"Starting Frame"},
        }
    };

    size_t ModelComponent::GetPropertyCount () const
    {
        return ModelComponentPropertyIds.size();
    }

    const StringId* ModelComponent::GetPropertyInfoArray () const
    {
        return ModelComponentPropertyIds.data();
    }

    Property ModelComponent::GetProperty ( const StringId& aId ) const
    {
        switch ( aId )
        {
        case ModelComponentPropertyIds[0]:
            return GetModel().GetPathString();
        case ModelComponentPropertyIds[1]:
            return GetActiveAnimation();
        case ModelComponentPropertyIds[2]:
            return GetStartingFrame();
        }
        return Property{};
    }

    void ModelComponent::SetProperty ( uint32_t aId, const Property& aProperty )
    {
        switch ( aId )
        {
        case ModelComponentPropertyIds[0]:
            if ( std::holds_alternative<std::string> ( aProperty ) )
            {
                SetModel ( {"Model"_crc32, std::get<std::string> ( aProperty ) } );
            }
            else if ( std::holds_alternative<std::filesystem::path> ( aProperty ) )
            {
                SetModel ( {"Model"_crc32, std::get<std::filesystem::path> ( aProperty ).string() } );
            }
            else if ( std::holds_alternative<uint32_t> ( aProperty ) )
            {
                SetModel ( {"Model"_crc32, std::get<uint32_t> ( aProperty ) } );
            }
            break;
        case ModelComponentPropertyIds[1]:
            if ( std::holds_alternative<std::string> ( aProperty ) )
            {
                SetActiveAnimation ( std::get<std::string> ( aProperty ) );
            }
            break;
        case ModelComponentPropertyIds[2]:
            if ( std::holds_alternative<double> ( aProperty ) )
            {
                SetStartingFrame ( std::get<double> ( aProperty ) );
            }
            break;
        }
    }

    const std::vector<std::string>& ModelComponent::GetPropertyEnumValues ( const StringId& aId ) const
    {
        static const std::vector<std::string> empty{};
        if ( aId == ModelComponentPropertyIds[1] )
        {
            if ( auto model = mModel.Cast<Model>() )
            {
                return model->GetAnimationNames();
            }
        }
        return empty;
    }

    void ModelComponent::SetModel ( const ResourceId& aModel )
    {
        mModel = aModel;
        mModel.Store();
        mLastResolvedModel = nullptr;
        mActiveAnimationIndex = Model::INVALID_ANIMATION_INDEX;
    }

    const ResourceId& ModelComponent::GetModel() const noexcept
    {
        return mModel;
    }

    void ModelComponent::SetActiveAnimation ( std::string_view aActiveAnimation )
    {
        // No-op when the active animation is unchanged so callers can
        // safely poke this every frame without restarting playback or
        // re-triggering a crossfade. Also no-op if the same target is
        // already pending so rapid repeat requests don't keep deferring
        // the switch.
        if ( aActiveAnimation == mActiveAnimation && !mPendingAnimationSwitch )
        {
            return;
        }
        if ( mPendingAnimationSwitch && aActiveAnimation == mPendingAnimation )
        {
            return;
        }
        // No previous clip to blend from yet, or blending is disabled:
        // switch immediately and skip the snapshot machinery entirely.
        if ( mBlendDuration <= 0.0f ||
             mActiveAnimationIndex == Model::INVALID_ANIMATION_INDEX )
        {
            mActiveAnimation.assign ( aActiveAnimation );
            mCurrentSample = mStartingFrame;
            mLastResolvedModel = nullptr;
            mActiveAnimationIndex = Model::INVALID_ANIMATION_INDEX;
            mPendingAnimationSwitch = false;
            mHasBlendSnapshot = false;
            mBlendElapsed = 0.0f;
            return;
        }
        // Defer the actual swap until Update() so we can first capture
        // the pose currently being rendered (which may itself be the
        // result of an in-flight crossfade) and blend out from it.
        mPendingAnimation.assign ( aActiveAnimation );
        mPendingAnimationSwitch = true;
    }

    const std::string& ModelComponent::GetActiveAnimation() const noexcept
    {
        return mActiveAnimation;
    }

    void ModelComponent::SetStartingFrame ( double aAnimationDelta ) noexcept
    {
        mStartingFrame = mCurrentSample = aAnimationDelta;
    }

    const double& ModelComponent::GetStartingFrame() const noexcept
    {
        return mStartingFrame;
    }

    void ModelComponent::SetBlendDuration ( float aSeconds ) noexcept
    {
        mBlendDuration = aSeconds < 0.0f ? 0.0f : aSeconds;
    }

    float ModelComponent::GetBlendDuration() const noexcept
    {
        return mBlendDuration;
    }

    void ModelComponent::Update ( Node& aNode, double aDelta )
    {
        if ( auto model = mModel.Cast<Model>() )
        {
            // Refresh the cached animation index when either the model or
            // active animation name changes (SetModel/SetActiveAnimation
            // clear mLastResolvedModel, so this also triggers on reload).
            if ( model != mLastResolvedModel )
            {
                mActiveAnimationIndex = model->GetAnimationIndexByName ( mActiveAnimation );
                mLastResolvedModel = model;
            }
            AABB aabb;
            for ( auto& i : model->GetAssemblies() )
            {
                if ( Mesh * mesh = std::get<0> ( i ).Cast<Mesh>() )
                {
                    aabb += mesh->GetAABB();
                }
            }
            aNode.SetAABB ( aabb );
            const Skeleton* skeleton{ model->GetSkeleton() };
            if ( skeleton )
            {
                float* skeleton_buffer = reinterpret_cast<float*> ( mSkeleton.data() );
                assert ( ( skeleton->GetJoints().size() * sizeof ( float ) * 16 ) < mSkeleton.size() );
                if ( mActiveAnimationIndex != Model::INVALID_ANIMATION_INDEX )
                {
                    const size_t joint_count = skeleton->GetJoints().size();

                    // Step 1: figure out the pose for this frame using
                    // the *currently active* animation (plus any in-flight
                    // snapshot blend). This is what would normally be
                    // written to the skeleton buffer.
                    auto animation = model->GetAnimationResources() [mActiveAnimationIndex].Cast<Animation>();
                    mCurrentSample = animation->AddTimeToSample ( mCurrentSample, aDelta );

                    float blend_weight = 1.0f;
                    bool snapshot_active = mHasBlendSnapshot && mBlendDuration > 0.0f &&
                                           mBlendSnapshot.size() >= joint_count;
                    if ( snapshot_active )
                    {
                        mBlendElapsed += static_cast<float> ( aDelta );
                        blend_weight = SmoothStep ( mBlendElapsed / mBlendDuration );
                        if ( mBlendElapsed >= mBlendDuration )
                        {
                            snapshot_active = false;
                            mHasBlendSnapshot = false;
                            mBlendElapsed = 0.0f;
                            blend_weight = 1.0f;
                        }
                    }

                    // Compute the per-bone pose for this frame. We keep it
                    // in a small local buffer so the same poses can be
                    // captured into mBlendSnapshot if a pending switch was
                    // queued via SetActiveAnimation().
                    std::vector<Transform> frame_pose;
                    frame_pose.resize ( joint_count );
                    for ( size_t i = 0; i < joint_count; ++i )
                    {
                        Transform current_xform = animation->GetTransform ( i, mCurrentSample );
                        if ( snapshot_active )
                        {
                            frame_pose[i] = BlendTransform ( mBlendSnapshot[i], current_xform, blend_weight );
                        }
                        else
                        {
                            frame_pose[i] = current_xform;
                        }
                    }

                    // Step 2: if a switch was requested since the last
                    // Update, freeze this frame's pose as the new snapshot
                    // and promote the pending animation to active. The
                    // next frame will start interpolating from the pose
                    // we're about to render this frame -> no pop.
                    if ( mPendingAnimationSwitch )
                    {
                        mBlendSnapshot = std::move ( frame_pose );
                        mHasBlendSnapshot = true;
                        mBlendElapsed = 0.0f;

                        mActiveAnimation = std::move ( mPendingAnimation );
                        mPendingAnimation.clear();
                        mPendingAnimationSwitch = false;
                        mActiveAnimationIndex = model->GetAnimationIndexByName ( mActiveAnimation );
                        mLastResolvedModel = model;
                        mCurrentSample = mStartingFrame;

                        // Write the snapshot pose itself for this frame.
                        for ( size_t i = 0; i < joint_count; ++i )
                        {
                            Matrix4x4 matrix{ mBlendSnapshot[i] *
                                              skeleton->GetJoints() [i].GetInvertedTransform() };
                            memcpy ( skeleton_buffer + ( i * 16 ), matrix.GetMatrix4x4(), sizeof ( float ) * 16 );
                        }
                    }
                    else
                    {
                        for ( size_t i = 0; i < joint_count; ++i )
                        {
                            Matrix4x4 matrix{ frame_pose[i] *
                                              skeleton->GetJoints() [i].GetInvertedTransform() };
                            memcpy ( skeleton_buffer + ( i * 16 ), matrix.GetMatrix4x4(), sizeof ( float ) * 16 );
                        }
                    }
                }
                else
                {
                    // No animation - use identity matrices (bind pose * inverse bind pose = identity)
                    Matrix4x4 identity{}; // Identity matrix
                    for ( size_t i = 0; i < skeleton->GetJoints().size(); ++i )
                    {
                        memcpy ( skeleton_buffer + ( i * 16 ), identity.GetMatrix4x4(), sizeof ( float ) * 16 );
                    }
                }
            }
        }
    }

    void ModelComponent::Render ( const Node& aNode, Renderer& aRenderer, void* aWindowId )
    {
        if ( auto model = mModel.Cast<Model>() )
        {
            BufferAccessor* skeleton_accessor_ptr{nullptr};
            BufferAccessor skeleton_accessor{};
            const Skeleton* skeleton{model->GetSkeleton() };
            if ( skeleton )
            {
                size_t used_bones_size = skeleton->GetJoints().size() * sizeof ( float ) * 16;
                assert ( used_bones_size <= mSkeleton.size() );
                skeleton_accessor = aRenderer.AllocateSingleFrameUniformMemory ( aWindowId, used_bones_size );
                skeleton_accessor.WriteMemory ( 0, used_bones_size, mSkeleton.data() );
                skeleton_accessor_ptr = &skeleton_accessor;
            }
            const auto& assemblies = model->GetAssemblies();
            // Draw pipeline substituted for skinned assemblies: the vertices are
            // already posed in the compute pre-pass, so they are drawn with a
            // non-skinning shader that consumes the compact 56-byte skinned
            // layout instead of re-applying the skeleton in the vertex stage.
            static const ResourceId no_skeleton_pipeline_id{ "Pipeline", "shaders/diffuse_map_phong_no_skeleton.txt" };
            for ( size_t index = 0; index < assemblies.size(); ++index )
            {
                const auto& i = assemblies[index];
                const BufferAccessor* skinned_vertices_ptr{nullptr};
                if ( index < mSkinnedVertices.size() &&
                     mSkinnedVertices[index].GetMemoryPoolBuffer() != nullptr )
                {
                    skinned_vertices_ptr = &mSkinnedVertices[index];
                }
                const Pipeline* pipeline = std::get<1> ( i ).Cast<Pipeline>();
                const Pipeline* no_skeleton_pipeline = nullptr;
                if ( skinned_vertices_ptr != nullptr )
                {
                    no_skeleton_pipeline = no_skeleton_pipeline_id.Get<Pipeline>();
                }
                aRenderer.Render (
                    aWindowId,
                    aNode.GetGlobalTransform(),
                    *std::get<0> ( i ).Cast<Mesh>(),
                    ( no_skeleton_pipeline != nullptr ) ? *no_skeleton_pipeline : *pipeline,
                    std::get<2> ( i ).Cast<Material>(),
                    // The skeleton is only needed by the in-shader skinning path;
                    // a pre-skinned draw passes none.
                    ( skinned_vertices_ptr != nullptr ) ? nullptr : skeleton_accessor_ptr,
                    Topology::TRIANGLE_LIST,
                    0,
                    0xffffffff,
                    1,
                    0,
                    skinned_vertices_ptr );
            }
        }
    }

    void ModelComponent::Skin ( const Node& aNode, Renderer& aRenderer, void* aWindowId )
    {
        ( void ) aNode;
        mSkinnedVertices.clear();
        auto model = mModel.Cast<Model>();
        if ( !model )
        {
            return;
        }
        const Skeleton* skeleton{ model->GetSkeleton() };
        if ( !skeleton )
        {
            return;
        }
        // The skinning compute pipeline is renderer-agnostic and shared across
        // every skinned model; fetch (and cache) it from the resource store.
        static const ResourceId skinning_pipeline_id{ "Pipeline", "shaders/skinning.txt" };
        const Pipeline* skinning_pipeline = skinning_pipeline_id.Get<Pipeline>();
        if ( !skinning_pipeline )
        {
            return;
        }
        // Upload the per-joint pose*inverse-bind matrices computed in Update()
        // as a storage buffer the compute kernel reads.
        size_t used_bones_size = skeleton->GetJoints().size() * sizeof ( float ) * 16;
        assert ( used_bones_size <= mSkeleton.size() );
        BufferAccessor skinning_matrices = aRenderer.AllocateSingleFrameStorageMemory ( aWindowId, used_bones_size );
        skinning_matrices.WriteMemory ( 0, used_bones_size, mSkeleton.data() );

        const auto& assemblies = model->GetAssemblies();
        mSkinnedVertices.resize ( assemblies.size() );
        bool dispatched = false;
        for ( size_t i = 0; i < assemblies.size(); ++i )
        {
            Mesh* mesh = std::get<0> ( assemblies[i] ).Cast<Mesh>();
            if ( mesh == nullptr || mesh->GetVertexCount() == 0 )
            {
                continue;
            }
            // The skinning kernel assumes the canonical 64-byte skinned vertex
            // layout with packed weight indices/values in the last two words;
            // skip assemblies whose meshes are not authored that way.
            bool has_weights = false;
            for ( const auto& attribute : mesh->GetAttributes() )
            {
                if ( std::get<0> ( attribute ) == Mesh::WEIGHT_INDEX )
                {
                    has_weights = true;
                    break;
                }
            }
            if ( !has_weights || mesh->GetStride() != 64 )
            {
                continue;
            }
            // The skinned output drops the per-vertex weight data, so it uses
            // the compact 56-byte stride consumed by the no-skeleton draw
            // pipeline (position, normal, tangent, bitangent, uv).
            constexpr size_t kSkinnedVertexStride = 56;
            size_t skinned_size = static_cast<size_t> ( mesh->GetVertexCount() ) * kSkinnedVertexStride;
            mSkinnedVertices[i] = aRenderer.AllocateSingleFrameStorageMemory ( aWindowId, skinned_size );
            aRenderer.Skin ( aWindowId, *skinning_pipeline, *mesh, skinning_matrices, mSkinnedVertices[i] );
            dispatched = true;
        }
        // Make the compute writes visible to the subsequent draw traversals.
        if ( dispatched )
        {
            aRenderer.Barrier ( aWindowId );
        }
    }

    void ModelComponent::ProcessMessage ( Node& aNode, uint32_t aMessageType, const void* aMessageData )
    {
    }
}
