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
#ifndef AEONGAMES_RENDERITEM_H
#define AEONGAMES_RENDERITEM_H
/*! \file
    \brief Header for the RenderItem draw-submission record.
*/
#include "aeongames/Matrix4x4.hpp"

namespace AeonGames
{
    class Mesh;
    class Pipeline;
    class Material;
    class BufferAccessor;
    /*! \brief Identifies which pass a render-queue submission feeds, so the
        renderer selects the right pipeline from an explicit argument instead of
        inferring it from internal frame state. */
    enum class RenderPass : uint32_t
    {
        /// Shadow depth pass: every geometry pipeline is substituted with the
        /// renderer-owned shadow-depth pipeline, which renders geometry from the
        /// shadow-casting light's point of view into a depth-only shadow map.
        ShadowPass,
        /// Clustered light-cull marking pass: every geometry pipeline is
        /// substituted with the renderer-owned cluster-mark pipeline, which only
        /// records the cluster each fragment occupies.
        DepthPrePass,
        /// Main color pass: each item draws with its own pipeline and material.
        Shading
    };
    /*! \brief A single draw declared by a component during the scene's collect
        phase.

        Components append RenderItems to the scene's render queue from
        Component::Collect instead of issuing draws themselves. A later submit
        phase sorts the queue and merges items that share geometry, pipeline and
        material into instanced draws, so components never talk to the Renderer
        directly and instancing is decided in one place. */
    struct RenderItem
    {
        /// Geometry to draw. Never null for a valid item.
        const Mesh* mMesh{};
        /// Graphics pipeline used to draw the geometry. Never null.
        const Pipeline* mPipeline{};
        /// Optional material bound to the pipeline; may be null.
        const Material* mMaterial{};
        /// Non-null when the mesh's vertices are posed by the compute skinning
        /// pre-pass; skinned items always draw individually (never instanced).
        const BufferAccessor* mSkinnedVertices{};
        /// World-space model matrix for this draw.
        Matrix4x4 mTransform{};
    };
}
#endif
