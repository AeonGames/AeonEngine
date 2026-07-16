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
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string_view>

namespace AeonGames
{
    Renderer::~Renderer() = default;

    void Renderer::RenderScene ( void* aWindowId, const Scene& aScene, const GuiOverlay* aGuiOverlay )
    {
        if ( !IsValidWindow ( aWindowId ) )
        {
            return;
        }
        // Skip the entire frame while the device is lost: recording any pass
        // against dead GPU handles would fault. The backend rebuilds the device
        // at the next BeginFrame, after which this gate reopens.
        if ( IsDeviceLost() )
        {
            return;
        }
        // The per-frame protocol, expressed once as an ordered sequence of step
        // primitives the backends implement. Keeping it here (rather than
        // duplicated per backend) means OpenGL and Vulkan render the scene
        // through identical logic.
        const Pipeline* lighting = aScene.GetLightingPipeline();
        if ( !mBenchmarkInitialized )
        {
            InitBenchmark();
        }
        // The opt-in per-pass benchmark times only lighting frames: the pass
        // boundaries the marks bracket exist only when the lighting path runs,
        // so a frame records either the full mark set or none.
        mBenchmarkFrameRecorded = mBenchmarkActive && ( lighting != nullptr );
        BeginRender ( aWindowId, lighting );
        MaybeRecordTimestamp ( aWindowId, 0 );
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
            // Point shadow passes: each point caster is omnidirectional, so all
            // six cube faces are rendered in a single draw -- Vulkan multiview
            // (one view per face) or an OpenGL geometry shader -- into the
            // caster's six cube-map-array layers.
            //
            // Each caster's cube map is cached: it is only re-rendered when the
            // caster's light (position/radius) or some shadow-casting geometry
            // actually changed since it was last drawn. The depth image persists
            // between frames, so an unchanged caster reuses its previous map and
            // skips the pass -- on a static scene the point shadow maps are
            // rendered once and then sampled for free every frame, even while the
            // camera moves.
            GpuPointShadowParams point_shadow_params{};
            const uint32_t point_caster_count = aScene.GetPointShadowCasters ( point_shadow_params );
            SetPointShadowParams ( aWindowId, point_shadow_params );
            const uint64_t shadow_geometry_signature = aScene.GetShadowGeometrySignature();
            auto& point_cache = mPointShadowCache[aWindowId];
            for ( uint32_t caster = 0; caster < point_caster_count; ++caster )
            {
                PointShadowCacheEntry& entry = point_cache[caster];
                if ( entry.rendered &&
                     entry.geometry_signature == shadow_geometry_signature &&
                     entry.light_position_radius == point_shadow_params.caster_position_radius[caster] )
                {
                    // Nothing this caster sees changed; reuse its cached cube map.
                    continue;
                }
                // The whole sphere is rendered in one pass, so cull once to a box
                // that bounds the caster's shadow sphere (a point light has no
                // single frustum; the axis-aligned box is a conservative superset).
                const Vector4& caster_position_radius = point_shadow_params.caster_position_radius[caster];
                const float radius = caster_position_radius.GetW();
                Matrix4x4 caster_bounds;
                caster_bounds.Ortho (
                    caster_position_radius.GetX() - radius, caster_position_radius.GetX() + radius,
                    caster_position_radius.GetZ() - radius, caster_position_radius.GetZ() + radius,
                    caster_position_radius.GetY() - radius, caster_position_radius.GetY() + radius );
                aScene.BuildRenderQueue ( Frustum ( caster_bounds ) );
                BeginPointShadowPass ( aWindowId, caster );
                SubmitRenderQueue ( aWindowId, aScene, RenderPass::ShadowPass );
                EndPointShadowPass ( aWindowId );
                entry.light_position_radius = point_shadow_params.caster_position_radius[caster];
                entry.geometry_signature = shadow_geometry_signature;
                entry.rendered = true;
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
            MaybeRecordTimestamp ( aWindowId, 1 );
            SubmitRenderQueue ( aWindowId, aScene, RenderPass::DepthPrePass );
            MaybeRecordTimestamp ( aWindowId, 2 );
            EndDepthPrePass ( aWindowId, lighting );
            MaybeRecordTimestamp ( aWindowId, 3 );
        }
        SubmitRenderQueue ( aWindowId, aScene, RenderPass::Shading );
        MaybeRecordTimestamp ( aWindowId, 4 );
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
        MaybeRecordTimestamp ( aWindowId, 5 );
        EndRender ( aWindowId );
        EndBenchmarkFrame ( aWindowId );
    }

    void Renderer::InitBenchmark()
    {
        mBenchmarkInitialized = true;
        const char* frames_env = std::getenv ( "AEON_BENCH_FRAMES" );
        if ( frames_env == nullptr )
        {
            return;
        }
        const long frames = std::strtol ( frames_env, nullptr, 10 );
        if ( frames <= 0 )
        {
            return;
        }
        mBenchmarkTarget = static_cast<uint32_t> ( frames );
        const char* warmup_env = std::getenv ( "AEON_BENCH_WARMUP" );
        mBenchmarkWarmup = ( warmup_env != nullptr )
                           ? static_cast<uint32_t> ( std::max ( 0L, std::strtol ( warmup_env, nullptr, 10 ) ) )
                           : 60u;
        mBenchmarkActive = true;
        for ( auto& segment : mBenchmarkSegments )
        {
            segment.reserve ( mBenchmarkTarget );
        }
        mBenchmarkTotals.reserve ( mBenchmarkTarget );
        std::cout << "GPU benchmark armed: " << mBenchmarkTarget
                  << " frames after " << mBenchmarkWarmup << " warm-up frames" << std::endl;
    }

    void Renderer::MaybeRecordTimestamp ( void* aWindowId, uint32_t aSlot )
    {
        if ( !mBenchmarkFrameRecorded )
        {
            return;
        }
        RecordGpuTimestamp ( aWindowId, aSlot );
    }

    void Renderer::EndBenchmarkFrame ( void* aWindowId )
    {
        if ( !mBenchmarkFrameRecorded )
        {
            return;
        }
        mBenchmarkFrameRecorded = false;
        std::array<uint64_t, kGpuTimestampMarks> marks{};
        if ( !ReadGpuTimestamps ( aWindowId, marks ) )
        {
            return;
        }
        ++mBenchmarkFrameCounter;
        if ( mBenchmarkFrameCounter <= mBenchmarkWarmup )
        {
            return;
        }
        for ( uint32_t i = 0; i + 1 < kGpuTimestampMarks; ++i )
        {
            const double ms = ( marks[i + 1] >= marks[i] )
                              ? static_cast<double> ( marks[i + 1] - marks[i] ) * 1e-6 : 0.0;
            mBenchmarkSegments[i].push_back ( ms );
        }
        mBenchmarkTotals.push_back ( ( marks[kGpuTimestampMarks - 1] >= marks[0] )
                                     ? static_cast<double> ( marks[kGpuTimestampMarks - 1] - marks[0] ) * 1e-6 : 0.0 );
        if ( ++mBenchmarkCollected < mBenchmarkTarget )
        {
            return;
        }
        // Target reached: summarise each segment and the total, then exit.
        static constexpr std::array < const char*, kGpuTimestampMarks - 1 > kSegmentNames =
        {
            "shadows+setup", "depth prepass", "hiz+lightcull", "shading", "debug+overlay"
        };
        auto stat = [] ( std::vector<double> v, double& mn, double& med, double& mean, double& p95, double& mx )
        {
            std::sort ( v.begin(), v.end() );
            mn = v.front();
            mx = v.back();
            med = v[v.size() / 2];
            p95 = v[std::min ( v.size() - 1, static_cast<size_t> ( static_cast<double> ( v.size() ) * 0.95 ) )];
            double sum = 0.0;
            for ( double x : v )
            {
                sum += x;
            }
            mean = sum / static_cast<double> ( v.size() );
        };
        const char* occ_env = std::getenv ( "AEON_HIZ_OCCLUSION" );
        const bool occlusion_on = ( occ_env == nullptr ) || ( std::string_view ( occ_env ) != "0" );
        std::cout << "\n=== GPU per-pass benchmark (" << mBenchmarkCollected
                  << " frames, occlusion=" << ( occlusion_on ? "ON" : "OFF" ) << ") ===\n";
        std::cout << std::left << std::setw ( 16 ) << "segment"
                  << std::right << std::setw ( 9 ) << "min" << std::setw ( 9 ) << "median"
                  << std::setw ( 9 ) << "mean" << std::setw ( 9 ) << "p95"
                  << std::setw ( 9 ) << "max" << "   (ms)\n";
        std::cout << std::fixed << std::setprecision ( 3 );
        double mn = 0.0, med = 0.0, mean = 0.0, p95 = 0.0, mx = 0.0;
        for ( uint32_t i = 0; i + 1 < kGpuTimestampMarks; ++i )
        {
            if ( mBenchmarkSegments[i].empty() )
            {
                continue;
            }
            stat ( mBenchmarkSegments[i], mn, med, mean, p95, mx );
            std::cout << std::left << std::setw ( 16 ) << kSegmentNames[i]
                      << std::right << std::setw ( 9 ) << mn << std::setw ( 9 ) << med
                      << std::setw ( 9 ) << mean << std::setw ( 9 ) << p95 << std::setw ( 9 ) << mx << "\n";
        }
        std::cout << std::string ( 61, '-' ) << "\n";
        stat ( mBenchmarkTotals, mn, med, mean, p95, mx );
        std::cout << std::left << std::setw ( 16 ) << "TOTAL"
                  << std::right << std::setw ( 9 ) << mn << std::setw ( 9 ) << med
                  << std::setw ( 9 ) << mean << std::setw ( 9 ) << p95 << std::setw ( 9 ) << mx
                  << "\n" << std::endl;
        std::exit ( 0 );
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
        mDebugPipeline->LoadFromFile ( "shaders/solid_color" );
        mDebugWireMesh->LoadFromFile ( "meshes/aabb_wire" );
        mDebugAABBMaterial->LoadFromFile ( "materials/solidcolor" );
        mDebugOctreeMaterial->LoadFromFile ( "materials/solidcolor" );
        mDebugFrustumMaterial->LoadFromFile ( "materials/solidcolor" );
        mDebugGridPipeline->LoadFromFile ( "shaders/debug_grid" );
        mDebugGridMesh->LoadFromFile ( "meshes/fullscreen_triangle" );
        mDebugGridMaterial->LoadFromFile ( "materials/debug_grid" );
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
