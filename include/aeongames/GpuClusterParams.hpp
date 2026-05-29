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
#ifndef AEONGAMES_GPUCLUSTERPARAMS_HPP
#define AEONGAMES_GPUCLUSTERPARAMS_HPP

#include <cstdint>
#include "aeongames/Vector4.hpp"
#include "aeongames/Matrix4x4.hpp"

namespace AeonGames
{
    /** @name Clustered Forward+ grid dimensions.
     *  The view frustum is sliced into CLUSTER_GRID_X x CLUSTER_GRID_Y tiles
     *  across the screen and CLUSTER_GRID_Z logarithmic depth slices. 16x9 keeps
     *  tiles roughly square at a 16:9 aspect; 24 depth slices give good depth
     *  discrimination without exploding memory. The values live here and
     *  propagate to shaders via the ClusterParams uniform block. */
    ///@{
    constexpr uint32_t CLUSTER_GRID_X = 16;
    constexpr uint32_t CLUSTER_GRID_Y = 9;
    constexpr uint32_t CLUSTER_GRID_Z = 24;
    constexpr uint32_t CLUSTER_COUNT = CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z;
    ///@}

    /** @brief Per-cluster view-space axis-aligned bounding box (ClusterAABBs SSBO element).
     *
     *  Two std430-friendly vec4s. The @c .xyz of each holds the min/max view-space
     *  corner; @c .w is padding kept zero so the CPU and GPU layouts agree. */
    struct GpuClusterAABB
    {
        Vector4 min_point { 0.0f, 0.0f, 0.0f, 0.0f };
        Vector4 max_point { 0.0f, 0.0f, 0.0f, 0.0f };
    };
    static_assert ( sizeof ( Vector4 ) == 16,
                    "Vector4 must be a tight 4xfloat for GPU layout compatibility." );
    static_assert ( sizeof ( GpuClusterAABB ) == 32,
                    "GpuClusterAABB must stay 16-byte aligned at 32 bytes for std430." );

    /** @brief CPU-side mirror of the @c ClusterParams uniform block.
     *
     *  Consumed by @c cluster_build.comp (to construct the per-cluster AABBs)
     *  and, later, by the clustered fragment shader (to map a fragment to its
     *  cluster). Laid out to match the std140 block:
     *  @code
     *  layout(std140) uniform ClusterParams {
     *      mat4 inverse_projection; // clip -> view (engine: +X right, +Y forward, +Z up)
     *      uvec4 grid;              // (x, y, z, total) cluster counts
     *      vec4 screen;             // (width, height, 0, 0) viewport size in pixels
     *  };
     *  @endcode
     *
     *  The shader derives the view-space near/far depths directly from
     *  @c inverse_projection, so no explicit near/far is stored here. */
    struct GpuClusterParams
    {
        Matrix4x4 inverse_projection {};
        uint32_t  grid[4]   { CLUSTER_GRID_X, CLUSTER_GRID_Y, CLUSTER_GRID_Z, CLUSTER_COUNT };
        float     screen[4] { 0.0f, 0.0f, 0.0f, 0.0f };
    };
    static_assert ( sizeof ( GpuClusterParams ) == 64 + 16 + 16,
                    "GpuClusterParams layout must match the shader-side std140 ClusterParams block." );
}
#endif
