/*
Copyright (C) 2016,2018,2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "Factory.h"
#include "aeongames/StringId.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Material.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/CRC.hpp"

namespace AeonGames
{
    Renderer::~Renderer() = default;

    void Renderer::RenderScene ( void* aWindowId, const Scene& aScene, const GuiOverlay* aGuiOverlay )
    {
        if ( !IsValidWindow ( aWindowId ) )
        {
            return;
        }
        // The per-frame protocol, expressed once as an ordered sequence of step
        // primitives the backends implement. Keeping it here (rather than
        // duplicated per backend) means OpenGL and Vulkan render the scene
        // through identical logic.
        const Pipeline* lighting = aScene.GetLightingPipeline();
        BeginRender ( aWindowId, lighting );
        if ( lighting )
        {
            // Spot shadow passes: render each spot shadow caster's depth into its
            // own layer of the spot shadow map array before the directional pass.
            // Spots run BEFORE the directional pass on purpose: every shadow
            // depth pass reuses the window's ShadowParams matrix as scratch for
            // the depth vertex shader, so the directional pass must write it LAST
            // to leave the directional matrix in place for the shading pass.
            GpuSpotShadowParams spot_shadow_params{};
            const uint32_t spot_caster_count = aScene.GetSpotShadowCasters ( spot_shadow_params );
            SetSpotShadowParams ( aWindowId, spot_shadow_params );
            for ( uint32_t slot = 0; slot < spot_caster_count; ++slot )
            {
                const Matrix4x4 spot_light_view_projection =
                    spot_shadow_params.spot_light_view_projection[slot];
                aScene.BuildRenderQueue ( Frustum ( spot_light_view_projection ) );
                BeginSpotShadowPass ( aWindowId, slot, spot_light_view_projection );
                SubmitRenderQueue ( aWindowId, aScene, RenderPass::ShadowPass );
                EndSpotShadowPass ( aWindowId );
            }
            // Point shadow passes: each point caster is omnidirectional, so its
            // depth is captured as six 90-degree faces (one per cube axis) into
            // six consecutive layers of the point shadow map array.
            GpuPointShadowParams point_shadow_params{};
            const uint32_t point_caster_count = aScene.GetPointShadowCasters ( point_shadow_params );
            SetPointShadowParams ( aWindowId, point_shadow_params );
            for ( uint32_t caster = 0; caster < point_caster_count; ++caster )
            {
                for ( uint32_t face = 0; face < POINT_SHADOW_FACES; ++face )
                {
                    const Matrix4x4 point_light_view_projection =
                        point_shadow_params.point_light_view_projection[caster * POINT_SHADOW_FACES + face];
                    aScene.BuildRenderQueue ( Frustum ( point_light_view_projection ) );
                    BeginPointShadowPass ( aWindowId, caster, face, point_light_view_projection );
                    SubmitRenderQueue ( aWindowId, aScene, RenderPass::ShadowPass );
                    EndPointShadowPass ( aWindowId );
                }
            }
            // Directional shadow pass: render scene depth from the sun's point of
            // view into the shadow map before shading so the fragment stage can
            // sample it. The shadow map must contain every caster the light can
            // see, NOT just what the camera sees, so the queue is culled to the
            // light's orthographic frustum here. Reusing the camera frustum would
            // make casters outside the view pop in and out as the camera moves.
            Matrix4x4 light_view_projection;
            if ( aScene.GetDirectionalShadowMatrix ( light_view_projection, GetProjectionMatrix ( aWindowId ) ) )
            {
                aScene.BuildRenderQueue ( Frustum ( light_view_projection ) );
                BeginShadowPass ( aWindowId, light_view_projection );
                SubmitRenderQueue ( aWindowId, aScene, RenderPass::ShadowPass );
                EndShadowPass ( aWindowId );
            }
        }
        // Collect every camera-visible draw; the queue feeds both the depth
        // pre-pass and the shading pass, merging sorted runs into instanced
        // draws on submit. Built after the shadow pass, which uses its own
        // light-space queue.
        aScene.BuildRenderQueue ( GetFrustum ( aWindowId ) );
        if ( lighting )
        {
            // Depth pre-pass: flag clusters containing visible geometry with the
            // renderer's marking pipeline before light culling.
            SubmitRenderQueue ( aWindowId, aScene, RenderPass::DepthPrePass );
            EndDepthPrePass ( aWindowId, lighting );
        }
        SubmitRenderQueue ( aWindowId, aScene, RenderPass::Shading );
        // Debug geometry shares the scene depth buffer and must precede the
        // overlay so the GUI stays on top.
        if ( mDebugRendering )
        {
            SubmitDebugGeometry ( aWindowId, aScene );
        }
        if ( aGuiOverlay )
        {
            RenderOverlay ( aWindowId, *aGuiOverlay );
        }
        EndRender ( aWindowId );
    }

    void Renderer::SetDebugRendering ( bool aEnabled )
    {
        mDebugRendering = aEnabled;
    }

    bool Renderer::GetDebugRendering() const
    {
        return mDebugRendering;
    }

    void Renderer::SetLightTypeEnabled ( LightType aType, bool aEnabled )
    {
        const uint32_t bit = 1u << static_cast<uint32_t> ( aType );
        if ( aEnabled )
        {
            mLightTypeMask |= bit;
        }
        else
        {
            mLightTypeMask &= ~bit;
        }
    }

    bool Renderer::GetLightTypeEnabled ( LightType aType ) const
    {
        return ( mLightTypeMask & ( 1u << static_cast<uint32_t> ( aType ) ) ) != 0u;
    }

    void Renderer::ToggleLightType ( LightType aType )
    {
        mLightTypeMask ^= ( 1u << static_cast<uint32_t> ( aType ) );
    }

    std::span<const GpuLight> Renderer::FilterLightsByType ( std::span<const GpuLight> aLights ) const
    {
        // Fast path: every type enabled, no filtering needed.
        constexpr uint32_t all_types =
            ( 1u << static_cast<uint32_t> ( LightType::Point ) ) |
            ( 1u << static_cast<uint32_t> ( LightType::Spot ) ) |
            ( 1u << static_cast<uint32_t> ( LightType::Directional ) );
        if ( ( mLightTypeMask & all_types ) == all_types )
        {
            return aLights;
        }
        mFilteredLights.clear();
        for ( const GpuLight& light : aLights )
        {
            if ( mLightTypeMask & ( 1u << light.type ) )
            {
                mFilteredLights.push_back ( light );
            }
        }
        return mFilteredLights;
    }

    void Renderer::SetDebugRenderSettings ( const DebugRenderSettings& aSettings )
    {
        mDebugSettings = aSettings;
        mDebugSettingsDirty = true;
    }

    const DebugRenderSettings& Renderer::GetDebugRenderSettings() const
    {
        return mDebugSettings;
    }

    void Renderer::EnsureDebugAssets()
    {
        if ( mDebugAssetsLoaded )
        {
            return;
        }
        mDebugPipeline = std::make_unique<Pipeline>();
        mDebugWireMesh = std::make_unique<Mesh>();
        mDebugAABBMaterial = std::make_unique<Material>();
        mDebugOctreeMaterial = std::make_unique<Material>();
        mDebugFrustumMaterial = std::make_unique<Material>();
        mDebugGridPipeline = std::make_unique<Pipeline>();
        mDebugGridMesh = std::make_unique<Mesh>();
        mDebugGridMaterial = std::make_unique<Material>();
        mDebugPipeline->LoadFromId ( "shaders/solid_color.txt"_crc32 );
        mDebugWireMesh->LoadFromId ( "meshes/aabb_wire.msh"_crc32 );
        mDebugAABBMaterial->LoadFromId ( "materials/solidcolor.txt"_crc32 );
        mDebugOctreeMaterial->LoadFromId ( "materials/solidcolor.txt"_crc32 );
        mDebugFrustumMaterial->LoadFromId ( "materials/solidcolor.txt"_crc32 );
        mDebugGridPipeline->LoadFromId ( "shaders/debug_grid.txt"_crc32 );
        mDebugGridMesh->LoadFromId ( "meshes/fullscreen_triangle.msh"_crc32 );
        mDebugGridMaterial->LoadFromId ( "materials/debug_grid.txt"_crc32 );
        mDebugAssetsLoaded = true;
    }

    void Renderer::SubmitDebugGeometry ( void* aWindowId, const Scene& aScene )
    {
        EnsureDebugAssets();
        if ( mDebugSettingsDirty )
        {
            // Push the tunable grid parameters into the grid material once per
            // change rather than every frame.
            mDebugGridMaterial->Set ( { "GridColor", mDebugSettings.mGridColor } );
            mDebugGridMaterial->Set ( { "GridMajorColor", mDebugSettings.mGridMajorColor } );
            mDebugGridMaterial->Set ( { "AxisXColor", mDebugSettings.mAxisXColor } );
            mDebugGridMaterial->Set ( { "AxisYColor", mDebugSettings.mAxisYColor } );
            mDebugGridMaterial->Set ( { "CellSize", mDebugSettings.mGridCellSize } );
            mDebugGridMaterial->Set ( { "MajorInterval", mDebugSettings.mGridMajorInterval } );
            mDebugGridMaterial->Set ( { "FadeDistance", mDebugSettings.mGridFadeDistance } );
            // Each wireframe category gets its own solid color.
            mDebugAABBMaterial->Set ( { "SolidColor", mDebugSettings.mNodeAABBColor } );
            mDebugOctreeMaterial->Set ( { "SolidColor", mDebugSettings.mOctreeColor } );
            mDebugFrustumMaterial->Set ( { "SolidColor", mDebugSettings.mCameraFrustumColor } );
            mDebugSettingsDirty = false;
        }
        // Infinite ground grid first: a single full-screen triangle the grid
        // shader unprojects to the z = 0 plane. It writes true scene depth, so
        // the wireframes drawn afterward depth-test against it correctly.
        if ( mDebugSettings.mDrawGrid )
        {
            Render ( aWindowId, Matrix4x4{}, *mDebugGridMesh, *mDebugGridPipeline,
                     mDebugGridMaterial.get(), Topology::TRIANGLE_LIST );
        }
        const Frustum& frustum = GetFrustum ( aWindowId );
        // Per-node world-space AABB wireframes for everything visible this frame.
        // The unit (radii 1) wire cube is scaled and translated to each box by
        // its transform; LINE_LIST topology draws the 12 edges.
        if ( mDebugSettings.mDrawNodeAABBs )
        {
            aScene.CullVisible ( frustum, [this, aWindowId] ( const Node & aNode )
            {
                const AABB world = aNode.GetGlobalTransform() * aNode.GetAABB();
                Render ( aWindowId, world.GetTransform(), *mDebugWireMesh, *mDebugPipeline,
                         mDebugAABBMaterial.get(), Topology::LINE_LIST );
            } );
        }
        // Scene octree grid: one wire cube per allocated cell whose bounds are on
        // screen, visualizing the spatial subdivision.
        if ( mDebugSettings.mDrawOctree )
        {
            aScene.ForEachOctreeCell ( frustum, [this, aWindowId] ( const AABB & aBounds, uint32_t /*aDepth*/ )
            {
                Render ( aWindowId, aBounds.GetTransform(), *mDebugWireMesh, *mDebugPipeline,
                         mDebugOctreeMaterial.get(), Topology::LINE_LIST );
            } );
        }
        // Camera frustums: the NDC cube ([-1, 1] wire cube) maps to a camera's
        // world-space frustum via nodeGlobalTransform * inverse(projection).
        // Uses the window's active projection (correct aspect already baked in);
        // drawn for every node carrying a Camera component, regardless of
        // visibility, so off-screen cameras still show.
        if ( mDebugSettings.mDrawCameraFrustums )
        {
            Matrix4x4 inverse_projection = GetProjectionMatrix ( aWindowId );
            inverse_projection.Invert();
            aScene.LoopTraverseDFSPreOrder ( [this, aWindowId, &inverse_projection] ( const Node & aNode )
            {
                if ( aNode.GetComponent ( "Camera"_crc32 ) == nullptr )
                {
                    return;
                }
                const Matrix4x4 model = Matrix4x4{ aNode.GetGlobalTransform() } * inverse_projection;
                Render ( aWindowId, model, *mDebugWireMesh, *mDebugPipeline,
                         mDebugFrustumMaterial.get(), Topology::LINE_LIST );
            } );
        }
    }

    /// @brief Factory implementation for Renderer with a window argument.
    FactoryImplementation1Arg ( Renderer, void* );
}
