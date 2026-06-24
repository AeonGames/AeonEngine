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
#ifndef AEONGAMES_GPUGLOBALS_HPP
#define AEONGAMES_GPUGLOBALS_HPP

#include "aeongames/Vector4.hpp"

namespace AeonGames
{
    /** @brief CPU-side mirror of the @c Globals uniform block.
     *
     *  Per-frame, scene-wide shading globals that have no relationship to any
     *  single object, light or cluster, so they live in their own small block
     *  rather than riding along an unrelated one. Today this is just the
     *  ambient fill term: the engine has no global illumination, so surfaces
     *  facing away from every light would otherwise render pure black; the
     *  ambient term lifts them just enough to reveal the diffuse maps,
     *  approximating the indirect bounce a reference render gets from its world
     *  lighting. Kept extensible so future frame-wide values (fog, exposure,
     *  time) can join it without touching other blocks. Laid out to match the
     *  std140 block:
     *  @code
     *  layout(std140) uniform Globals {
     *      vec4 ambient; // (rgb) ambient color, (a) intensity multiplier
     *  };
     *  @endcode
     *
     *  The fragment shader uses @c ambient.rgb * ambient.a as the ambient
     *  radiance. The default @c (1,1,1,0.25) reproduces the previous hard-coded
     *  @c vec3(0.25) flat ambient, so nothing changes until a scene sets it. */
    struct GpuGlobals
    {
        Vector4 ambient { 1.0f, 1.0f, 1.0f, 0.25f };
    };
    static_assert ( sizeof ( Vector4 ) == 16,
                    "Vector4 must be a tight 4xfloat for GPU layout compatibility." );
    static_assert ( sizeof ( GpuGlobals ) == 16,
                    "GpuGlobals layout must match the shader-side std140 Globals block." );
}
#endif
