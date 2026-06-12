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
        // Collect every visible draw once; the queue feeds both the depth
        // pre-pass and the shading pass, merging sorted runs into instanced
        // draws on submit.
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
