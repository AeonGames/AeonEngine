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

    /// @brief Factory implementation for Renderer with a window argument.
    FactoryImplementation1Arg ( Renderer, void* );
}
